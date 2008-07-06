/// @copyright (c) 2007 CSIRO
/// Australia Telescope National Facility (ATNF)
/// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
/// PO Box 76, Epping NSW 1710, Australia
/// atnf-enquiries@csiro.au
///
/// This file is part of the ASKAP software distribution.
///
/// The ASKAP software distribution is free software: you can redistribute it
/// and/or modify it under the terms of the GNU General Public License as
/// published by the Free Software Foundation; either version 2 of the License,
/// or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
///

#include <gridding/AProjectWStackVisGridder.h>

#include <casa/BasicSL/Complex.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/ArrayMath.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/UVWMachine.h>
#include <casa/Quanta/MVDirection.h>
#include <casa/Quanta/MVAngle.h>
#include <casa/Quanta/MVTime.h>

#include <askap_synthesis.h>
#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, ".gridding");

#include <askap/AskapError.h>
#include <askap/AskapUtil.h>

#include <fft/FFTWrapper.h>
#include <gridding/UVPattern.h>
#include <gridding/IBasicIllumination.h>

using namespace askap;

namespace askap {
namespace synthesis {

AProjectWStackVisGridder::AProjectWStackVisGridder(
        const boost::shared_ptr<IBasicIllumination const> &illum,
        const double wmax, const int nwplanes,
		const int overSample, const int maxSupport, const int maxFeeds,
		const int maxFields, const double pointingTol,
		const bool frequencyDependent, const std::string& name) :
	WStackVisGridder(wmax, nwplanes), itsReferenceFrequency(0.0),
			itsIllumination(illum),
			itsMaxFeeds(maxFeeds), itsMaxFields(maxFields),
			itsPointingTolerance(pointingTol), itsFreqDep(frequencyDependent)

{	
	ASKAPCHECK(maxFeeds>0, "Maximum number of feeds must be one or more");
	ASKAPCHECK(maxFields>0, "Maximum number of fields must be one or more");
	ASKAPCHECK(overSample>0, "Oversampling must be greater than 0");
	ASKAPCHECK(maxSupport>0, "Maximum support must be greater than 0")
	ASKAPCHECK(pointingTol>0.0, "Pointing tolerance must be greater than 0.0");
	ASKAPDEBUGASSERT(itsIllumination);
	itsSupport=0;
	itsOverSample=overSample;
	itsMaxSupport=maxSupport;
	itsName=name;

	itsSlopes.resize(2, itsMaxFeeds, itsMaxFields);
	itsSlopes.set(0.0);
	itsDone.resize(itsMaxFeeds, itsMaxFields);
	itsDone.set(false);
	itsPointings.resize(itsMaxFeeds, itsMaxFields);
	itsPointings.set(casa::MVDirection());
	itsLastField=-1;
	itsCurrentField=0;
}

/// @brief copy constructor
/// @details It is required to decouple internal arrays between input object
/// and this copy.
/// @param[in] other input object
/// @note illumination pattern is copied as a shared pointer, hence referencing
/// the same model
AProjectWStackVisGridder::AProjectWStackVisGridder(const AProjectWStackVisGridder &other) :
       WStackVisGridder(other), itsReferenceFrequency(other.itsReferenceFrequency),
       itsIllumination(other.itsIllumination), itsMaxFeeds(other.itsMaxFeeds),
       itsMaxFields(other.itsMaxFields), itsPointingTolerance(other.itsPointingTolerance),
       itsLastField(other.itsLastField), itsCurrentField(other.itsCurrentField),
       itsFreqDep(other.itsFreqDep), itsMaxSupport(other.itsMaxSupport),
       itsCMap(other.itsCMap.copy()), itsSlopes(other.itsSlopes.copy()),
       itsDone(other.itsDone.copy()), itsPointings(other.itsPointings.copy()) {}


AProjectWStackVisGridder::~AProjectWStackVisGridder() {
}

/// Clone a copy of this Gridder
IVisGridder::ShPtr AProjectWStackVisGridder::clone() {
	return IVisGridder::ShPtr(new AProjectWStackVisGridder(*this));
}

/// Initialize the indices into the cube.
void AProjectWStackVisGridder::initIndices(const IConstDataAccessor& acc) {

	// Validate cache using first row only
	bool newField=true;

	int firstFeed=acc.feed1()(0);
	ASKAPCHECK(firstFeed<itsMaxFeeds, "Too many feeds: increase maxfeeds");
	casa::MVDirection firstPointing=acc.pointingDir1()(0);

	for (int field=itsLastField; field>-1; --field) {
		if (firstPointing.separation(itsPointings(firstFeed, field))
				<itsPointingTolerance) {
			itsCurrentField=field;
			newField=false;
			break;
		}
	}
	if (newField) {
		itsLastField++;
		itsCurrentField=itsLastField;
		ASKAPCHECK(itsCurrentField<itsMaxFields,
				"Too many fields: increase maxfields " << itsMaxFields);
		itsPointings(firstFeed, itsCurrentField)=firstPointing;
		ASKAPLOG_INFO_STR(logger, "Found new field " << itsCurrentField<<" at "<<
		                  printDirection(firstPointing));
	}

	/// We have to calculate the lookup function converting from
	/// row and channel to plane of the w-dependent convolution
	/// function
	const int nSamples = acc.nRow();
	const int nChan = acc.nChannel();

	const int nPol = acc.nPol();
	itsCMap.resize(nSamples, nPol, nChan);
	itsCMap.set(0);
	/// @todo Select max feeds more carefully

	itsGMap.resize(nSamples, nPol, nChan);
	itsGMap.set(0);

	int cenw=(itsNWPlanes-1)/2;

	for (int i=0; i<nSamples; ++i) {
		int feed=acc.feed1()(i);
		ASKAPCHECK(feed<itsMaxFeeds,
				"Exceeded specified maximum number of feeds");
		ASKAPCHECK(feed>-1, "Illegal negative feed number");

		const double w=(acc.uvw()(i)(2))/(casa::C::c);

		for (int chan=0; chan<nChan; ++chan) {
		  const double freq=acc.frequency()[chan];
			for (int pol=0; pol<nPol; pol++) {
				/// Order is (chan, feed)
				if(itsFreqDep) {
					itsCMap(i, pol, chan)=chan+nChan*(feed+itsMaxFeeds*itsCurrentField);
					ASKAPCHECK(itsCMap(i, pol, chan)<itsMaxFields*itsMaxFeeds
							*nChan, "CMap index too large");
					ASKAPCHECK(itsCMap(i, pol, chan)>-1,
							"CMap index less than zero");
				}
				else {
					itsCMap(i, pol, chan)=(feed+itsMaxFeeds*itsCurrentField);
					ASKAPCHECK(itsCMap(i, pol, chan)<itsMaxFields*itsMaxFeeds*nChan,
							"CMap index too large");
					ASKAPCHECK(itsCMap(i, pol, chan)>-1, "CMap index less than zero");
				}

				/// Calculate the index into the grids
				if (itsNWPlanes>1) {
					itsGMap(i, pol, chan)=cenw+nint(w*freq/itsWScale);
				} else {
					itsGMap(i, pol, chan)=0;
				}
				ASKAPCHECK(itsGMap(i, pol, chan)<itsNWPlanes,
						"W scaling error: recommend allowing larger range of w");
				ASKAPCHECK(itsGMap(i, pol, chan)>-1,
						"W scaling error: recommend allowing larger range of w");
			}
		}
	}
}

/// Initialize the convolution function into the cube. If necessary this
/// could be optimized by using symmetries.
/// @todo Make initConvolutionFunction more robust
void AProjectWStackVisGridder::initConvolutionFunction(const IConstDataAccessor& acc) {
 
    ASKAPDEBUGASSERT(itsIllumination);
    // just to avoid a repeated call to a virtual function from inside the loop
    const bool hasSymmetricIllumination = itsIllumination->isSymmetric();
    
	casa::MVDirection out = getImageCentre();
	const int nSamples = acc.nRow();

	/// We have to calculate the lookup function converting from
	/// row and channel to plane of the w-dependent convolution
	/// function
	const int nChan = itsFreqDep ? acc.nChannel() : 1;

	if(itsSupport==0) {
	  itsConvFunc.resize(itsOverSample*itsOverSample*itsMaxFeeds*itsMaxFields*nChan);
	  itsSumWeights.resize(itsMaxFeeds*itsMaxFields*nChan, itsShape(2), itsShape(3));
	  itsSumWeights.set(0.0);
	}

	/// Limit the size of the convolution function since
	/// we don't need it finely sampled in image space. This
	/// will reduce the time taken to calculate it.
	casa::uInt nx=std::min(itsMaxSupport, itsShape(0));
	casa::uInt ny=std::min(itsMaxSupport, itsShape(1));
    
    // this is just a buffer in the uv-space
    UVPattern pattern(nx,ny, itsUVCellSize(0),itsUVCellSize(1),itsOverSample);
    
	int nDone=0;
	for (int row=0; row<nSamples; ++row) {
		const int feed=acc.feed1()(row);

		if (!itsDone(feed, itsCurrentField)) {
			itsDone(feed, itsCurrentField)=true;
			nDone++;
			casa::MVDirection offset(acc.pointingDir1()(row).getAngle());
			itsSlopes(0, feed, itsCurrentField) =sin(offset.getLong()
					-out.getLong()) *cos(offset.getLat());
			itsSlopes(1, feed, itsCurrentField)=sin(offset.getLat())
					*cos(out.getLat()) - cos(offset.getLat())*sin(out.getLat())
					*cos(offset.getLong()-out.getLong());

            const double parallacticAngle = hasSymmetricIllumination ? 0. : acc.feed1PA()(row);
            
			for (int chan=0; chan<nChan; chan++) {
				/// Extract illumination pattern for this channel
				itsIllumination->getPattern(acc.frequency()[chan], pattern,
				         itsSlopes(0, feed, itsCurrentField),
				         itsSlopes(1, feed, itsCurrentField), 
				         parallacticAngle);
				         
				/// Now convolve the disk with itself
				fft2d(pattern.pattern(), false);

				for (casa::uInt ix=0; ix<nx; ++ix) {
					for (casa::uInt iy=0; iy<ny; ++iy) {
						pattern(ix, iy)=pattern(ix,iy)*conj(pattern(ix,iy));
					}
				}

				fft2d(pattern.pattern(), true);
				
				double sumdisk=0.0;
				for (casa::uInt ix=0; ix<nx; ++ix) {
					for (casa::uInt iy=0; iy<ny; ++iy) {
						sumdisk+=casa::abs(pattern(ix, iy));
					}
				}
				pattern.pattern() *= casa::Complex(float(itsOverSample)*float(itsOverSample)/casa::Complex(sumdisk));

				if (itsSupport==0) {
				    // we probably need a proper support search here
				    // it can be encapsulated in a method of the UVPattern class
					itsSupport = pattern.maxSupport();
					ASKAPCHECK(itsSupport>0,
							"Unable to determine support of convolution function");
					ASKAPCHECK(itsSupport*itsOverSample<int(nx)/2,
							"Overflowing convolution function - increase maxSupport or decrease overSample")
					itsCSize=2*itsSupport+1;
					// just for logging
					const double cell = std::abs(pattern.uCellSize())*(casa::C::c/acc.frequency()[chan]);
					ASKAPLOG_INFO_STR(logger, "Convolution function support = "
							<< itsSupport << " pixels, size = " << itsCSize
							<< " pixels");
					ASKAPLOG_INFO_STR(logger, "Maximum extent = "<< itsSupport
							*cell << " (m) sampled at "<< cell
							<< " (m)");
					itsCCenter=itsSupport;
					ASKAPLOG_INFO_STR(logger, "Number of planes in convolution function = "
							  << itsConvFunc.size());
				}
				int zIndex=chan+nChan*(feed+itsMaxFeeds*itsCurrentField);

				for (int fracu=0; fracu<itsOverSample; fracu++) {
					for (int fracv=0; fracv<itsOverSample; fracv++) {
						int plane=fracu+itsOverSample*(fracv+itsOverSample*zIndex);
						ASKAPDEBUGASSERT(plane>=0 && plane<int(itsConvFunc.size()));
						itsConvFunc[plane].resize(itsCSize, itsCSize);
						itsConvFunc[plane].set(0.0);
						// Now cut out the inner part of the convolution function and
						// insert it into the convolution function
						for (int iy=-itsSupport; iy<itsSupport; iy++) {
							for (int ix=-itsSupport; ix<itsSupport; ix++) {
								itsConvFunc[plane](ix+itsCCenter, iy+itsCCenter)
										= pattern(itsOverSample*ix+fracu+nx/2,
												itsOverSample*iy+fracv+ny/2);
							}
						}
					}
				}
			}
		}
	}

	ASKAPCHECK(itsSupport>0, "Support not calculated correctly");

}

// To finalize the transform of the weights, we use the following steps:
// 1. For each plane of the convolution function, transform to image plane
// and multiply by conjugate to get abs value squared.
// 2. Sum all planes weighted by the weight for that convolution function.
void AProjectWStackVisGridder::finaliseWeights(casa::Array<double>& out) {

	ASKAPLOG_INFO_STR(logger, "Calculating sum of weights image");

	int nx=itsShape(0);
	int ny=itsShape(1);
	int nPol=itsShape(2);
	int nChan=itsShape(3);

	int nZ=itsSumWeights.shape()(0);

	/// We must pad the convolution function to full size, reverse transform
	/// square, and sum multiplied by the corresponding weight
	int cnx=std::min(itsMaxSupport, nx);
	int cny=std::min(itsMaxSupport, ny);
	int ccenx=cnx/2;
	int cceny=cny/2;

	/// This is the output array before sinc padding
	casa::Array<double> cOut(casa::IPosition(4, cnx, cny, nPol, nChan));
	cOut.set(0.0);

	/// Work space
	casa::Matrix<casa::Complex> thisPlane(cnx, cny);

	/// itsSumWeights has one element for each separate data plane (feed, field, chan)
	/// itsConvFunc has overSampling**2 planes for each separate data plane (feed, field, chan)
	/// We choose the convolution function at zero fractional offset in u,v 
	for (int iz=0; iz<nZ; iz++) {
	  int plane=itsOverSample*itsOverSample*iz;
		thisPlane.set(0.0);

		bool hasData=false;
		for (int chan=0; chan<nChan; chan++) {
			for (int pol=0; pol<nPol; pol++) {
				float wt=itsSumWeights(iz, pol, chan);
				if(wt>0.0) {
				  hasData=true;
				  break;
				}
			}
		}

		if(hasData) {

		// Now fill the inner part of the uv plane with the convolution function
		// and transform to obtain the image. The uv sampling is fixed here
		// so the total field of view is itsOverSample times larger than the
		// original field of view.
		for (int iy=-itsSupport; iy<+itsSupport; iy++) {
			for (int ix=-itsSupport; ix<+itsSupport; ix++) {
				thisPlane(ix+ccenx, iy+cceny)=itsConvFunc[plane](ix+itsCCenter, iy+itsCCenter);
			}
		}

		thisPlane*=casa::Complex(double(cnx)*double(cny));
		/// The peak here should be unity
		fft2d(thisPlane, false);

		// Now we need to cut out only the part inside the field of view
		for (int chan=0; chan<nChan; chan++) {
			for (int pol=0; pol<nPol; pol++) {
				casa::IPosition ip(4, 0, 0, pol, chan);
				double wt=itsSumWeights(iz, pol, chan);
				for (int ix=0; ix<cnx; ix++) {
					ip(0)=ix;
					for (int iy=0; iy<cny; iy++) {
						ip(1)=iy;
						cOut(ip)+=float(wt)*casa::real(thisPlane(ix, iy)*conj(thisPlane(ix, iy)));
					}
				}
			}
		}
		}
	}
	fftPad(cOut, out);
}

void AProjectWStackVisGridder::fftPad(const casa::Array<double>& in,
		casa::Array<double>& out) {

	int inx=in.shape()(0);
	int iny=in.shape()(1);

	int onx=out.shape()(0);
	int ony=out.shape()(1);

	// Shortcut no-op
	if ((inx==onx)&&(iny==ony)) {
		out=in.copy();
		return;
	}

	ASKAPCHECK(onx>=inx, "Attempting to pad to smaller array");
	ASKAPCHECK(ony>=iny, "Attempting to pad to smaller array");

	/// Make an iterator that returns plane by plane
	casa::ReadOnlyArrayIterator<double> inIt(in, 2);
	casa::ArrayIterator<double> outIt(out, 2);
	while (!inIt.pastEnd()&&!outIt.pastEnd()) {
		casa::Matrix<casa::DComplex> inPlane(inx, iny);
		casa::Matrix<casa::DComplex> outPlane(onx, ony);
		casa::convertArray(inPlane, inIt.array());
		outPlane.set(0.0);
		fft2d(inPlane, false);
		for (int iy=0; iy<iny; iy++) {
			for (int ix=0; ix<inx; ix++) {
				outPlane(ix+(onx-inx)/2, iy+(ony-iny)/2) = inPlane(ix, iy);
			}
		}
		fft2d(outPlane, true);
		const casa::Array<casa::DComplex> constOutPlane(outPlane);
		casa::Array<double> outArray(outIt.array());

		casa::real(outArray, constOutPlane);

		inIt.next();
		outIt.next();
	}
}

int AProjectWStackVisGridder::cIndex(int row, int pol, int chan) {
	return itsCMap(row, pol, chan);
}

void AProjectWStackVisGridder::correctConvolution(casa::Array<double>& grid) {
}

}
}

