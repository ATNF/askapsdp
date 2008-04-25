#include <gridding/AWProjectVisGridder.h>

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

using namespace askap;

namespace askap {
namespace synthesis {

AWProjectVisGridder::AWProjectVisGridder(const double diameter,
		const double blockage, const double wmax, const int nwplanes,
		const double cutoff, const int overSample, const int maxSupport,
		const int maxFeeds, const int maxFields, const double pointingTol,
		const bool frequencyDependent, const std::string& name) :
	WProjectVisGridder(wmax, nwplanes, cutoff, overSample, maxSupport, name),
			itsReferenceFrequency(0.0), itsDiameter(diameter),
			itsBlockage(blockage), itsFreqDep(frequencyDependent),
			itsMaxFeeds(maxFeeds), itsMaxFields(maxFields),
			itsPointingTolerance(pointingTol)

{
	ASKAPCHECK(diameter>0.0, "Blockage must be positive");
	ASKAPCHECK(diameter>blockage,
			"Antenna diameter must be greater than blockage");
	ASKAPCHECK(blockage>=0.0, "Blockage must be non-negative");
	ASKAPCHECK(maxFeeds>0, "Maximum number of feeds must be one or more");
	ASKAPCHECK(maxFields>0, "Maximum number of fields must be one or more");
	ASKAPCHECK(overSample>0, "Oversampling must be greater than 0");
	ASKAPCHECK(maxSupport>0, "Maximum support must be greater than 0")
	ASKAPCHECK(pointingTol>0.0, "Pointing tolerance must be greater than 0.0");
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

AWProjectVisGridder::~AWProjectVisGridder() {
}

/// Clone a copy of this Gridder
IVisGridder::ShPtr AWProjectVisGridder::clone() {
	return IVisGridder::ShPtr(new AWProjectVisGridder(*this));
}

/// Initialize the indices into the cube.
void AWProjectVisGridder::initIndices(IDataSharedIter& idi) {

	// Validate cache using first row only
	bool newField=true;

	int firstFeed=idi->feed1()(0);
	ASKAPCHECK(firstFeed<itsMaxFeeds, "Too many feeds: increase maxfeeds");
	casa::MVDirection firstPointing=idi->pointingDir1()(0);

	for (int field=itsLastField; field>-1; field--) {
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
		ASKAPLOG_INFO_STR(logger, "Found new field " << itsCurrentField);
	}

	/// We have to calculate the lookup function converting from
	/// row and channel to plane of the w-dependent convolution
	/// function
	const int nSamples = idi->uvw().size();
	int nChan = idi->frequency().size();

	const int nPol = idi->rwVisibility().shape()(2);
	itsCMap.resize(nSamples, nPol, nChan);
	itsCMap.set(0);
	/// @todo Select max feeds more carefully

	int cenw=(itsNWPlanes-1)/2;

	for (int i=0; i<nSamples; i++) {
		int feed=idi->feed1()(i);

		ASKAPCHECK(feed<itsMaxFeeds,
				"Exceeded specified maximum number of feeds");
		ASKAPCHECK(feed>-1, "Illegal negative feed number");

		double w=(idi->uvw()(i)(2))/(casa::C::c);

		for (int chan=0; chan<nChan; chan++) {
			double freq=idi->frequency()[chan];
			int iw=0;
			if (itsNWPlanes>1) {
				iw=cenw+int(w*freq/itsWScale);
			}
			ASKAPCHECK(iw<itsNWPlanes,
					"W scaling error: recommend allowing larger range of w");
			ASKAPCHECK(iw>-1,
					"W scaling error: recommend allowing larger range of w");

			for (int pol=0; pol<nPol; pol++) {
				/// Order is (iw, chan, feed)
				if (itsFreqDep) {
					itsCMap(i, pol, chan)=iw+itsNWPlanes*(chan+nChan*(feed+itsMaxFeeds*itsCurrentField));
					ASKAPCHECK(itsCMap(i, pol, chan)<itsNWPlanes*itsMaxFeeds
							*itsMaxFields*nChan, "CMap index too large");
					ASKAPCHECK(itsCMap(i, pol, chan)>-1,
							"CMap index less than zero");
				} else {
					itsCMap(i, pol, chan)=iw+itsNWPlanes*(feed+itsMaxFeeds*itsCurrentField);
					ASKAPCHECK(itsCMap(i, pol, chan)<itsNWPlanes*itsMaxFeeds
							*itsMaxFields, "CMap index too large");
					ASKAPCHECK(itsCMap(i, pol, chan)>-1,
							"CMap index less than zero");
				}
			}
		}
	}
}

/// Initialize the convolution function into the cube. If necessary this
/// could be optimized by using symmetries.
/// @todo Make initConvolutionFunction more robust
void AWProjectVisGridder::initConvolutionFunction(IDataSharedIter& idi) {

	casa::Quantum<double> refLon((itsAxes.start("RA")+itsAxes.end("RA"))/2.0, "rad");
	casa::Quantum<double> refLat((itsAxes.start("DEC")+itsAxes.end("DEC")) /2.0, "rad");
	casa::MVDirection out(refLon, refLat);
	const int nSamples = idi->uvw().size();
	// exact formulae for l and m 

	/// We have to calculate the lookup function converting from
	/// row and channel to plane of the w-dependent convolution
	/// function
	int nChan=1;
	if (itsFreqDep) {
		nChan = idi->frequency().size();
	}
	int cenw=(itsNWPlanes-1)/2;

	/// Limit the size of the convolution function since
	/// we don't need it finely sampled in image space. This
	/// will reduce the time taken to calculate it.
	int nx=std::min(itsMaxSupport, itsShape(0));
	int ny=std::min(itsMaxSupport, itsShape(1));

	int qnx=nx/itsOverSample;
	int qny=ny/itsOverSample;

	// Find the actual cellsizes in x and y (radians) 
	// corresponding to the limited support
	double ccellx=1.0/(double(qnx)*itsUVCellSize(0));
	double ccelly=1.0/(double(qny)*itsUVCellSize(1));

	casa::Vector<float> ccfx(qnx);
	casa::Vector<float> ccfy(qny);
	for (int ix=0; ix<qnx; ix++) {
		float nux=std::abs(float(ix-qnx/2))/float(qnx/2);
		ccfx(ix)=grdsf(nux)/float(qnx);
	}
	for (int iy=0; iy<qny; iy++) {
		float nuy=std::abs(float(iy-qny/2))/float(qny/2);
		ccfy(iy)=grdsf(nuy)/float(qny);
	}

	int nDone=0;
	for (int row=0; row<nSamples; row++) {
		int feed=idi->feed1()(row);

		if (!itsDone(feed, itsCurrentField)) {
			itsDone(feed, itsCurrentField)=true;
			nDone++;
			casa::MVDirection offset(idi->pointingDir1()(row).getAngle());
			itsSlopes(0, feed, itsCurrentField) =sin(offset.getLong()
					-out.getLong()) *cos(offset.getLat());
			itsSlopes(1, feed, itsCurrentField)=sin(offset.getLat())
					*cos(out.getLat()) - cos(offset.getLat())*sin(out.getLat())
					*cos(offset.getLong()-out.getLong());

			double ax=2.0f*casa::C::pi*itsUVCellSize(0) *itsSlopes(0, feed,
					itsCurrentField);
			double ay=2.0f*casa::C::pi*itsUVCellSize(1) *itsSlopes(1, feed,
					itsCurrentField);

			for (int chan=0; chan<nChan; chan++) {

				/// Make the disk for this channel
				casa::Matrix<casa::Complex> disk(qnx, qny);
				disk.set(0.0);

				/// Calculate the size of one cell in m.
				double cell=std::abs(itsUVCellSize(0)*(casa::C::c
						/idi->frequency()[chan]));
				double rmax=std::pow(itsDiameter/(2.0*cell), 2);
				double rmin=std::pow(itsBlockage/(2.0*cell), 2);

				/// Calculate the antenna voltage pattern, including the
				/// phase shift due to pointing
				double sumdisk=0.0;
				for (int ix=0; ix<qnx; ix++) {
					double nux=double(ix-qnx/2);
					double nux2=nux*nux;
					for (int iy=0; iy<qny; iy++) {
						double nuy=double(iy-qny/2);
						double nuy2=nuy*nuy;
						double r=nux2+nuy2;
						if ((r>=rmin)&&(r<=rmax)) {
							double phase=ax*nux+ay*nuy;
							disk(ix, iy)=casa::Complex(cos(phase), -sin(phase));
							sumdisk+=1.0;
						}
					}
				}
				ASKAPCHECK(sumdisk>0.0, "Integral of disk should be non-zero");
				disk*=casa::Complex(float(qnx)*float(qny)/sumdisk);
				fft2d(disk, false);

				/// Calculate the total convolution function including
				/// the w term and the antenna convolution function
				casa::Matrix<casa::Complex> thisPlane(nx, ny);

				for (int iw=0; iw<itsNWPlanes; iw++) {
					thisPlane.set(0.0);

					// Loop over the central nx, ny region, setting it to the product
					// of the phase screen and the spheroidal function
					double maxCF=0.0;
					double w=2.0f*casa::C::pi*double(iw-cenw)*itsWScale;
					for (int iy=0; iy<qny; iy++) {
						double y2=double(iy-qny/2)*ccelly;
						y2*=y2;
						for (int ix=0; ix<qnx; ix++) {
							double x2=double(ix-qnx/2)*ccellx;
							x2*=x2;
							double r2=x2+y2;
							if (r2<1.0) {
								double phase=w*(1.0-sqrt(1.0-r2));
								casa::Complex wt=disk(ix, iy)
										*conj(disk(ix, iy))
										*casa::Complex(ccfx(ix)*ccfy(iy));
								thisPlane(ix-qnx/2+nx/2, iy-qny/2+ny/2)=wt
										*casa::Complex(cos(phase), -sin(phase));
								maxCF+=casa::abs(wt);
							}
						}
					}
					ASKAPCHECK(maxCF>0.0, "Convolution function is empty");
					// At this point, we have the phase screen multiplied by the spheroidal
					// function, sampled on larger cellsize (itsOverSample larger) in image
					// space. Only the inner qnx, qny pixels have a non-zero value

					// Now we have to calculate the Fourier transform to get the
					// convolution function in uv space
					fft2d(thisPlane, true);

					// If the support is not yet set, find it and size the
					// convolution function appropriately
					if (itsSupport==0) {
						// Find the support by starting from the edge and
						// working in
						for (int ix=0; ix<nx/2; ix++) {
							/// Check on horizontal axis
							if ((casa::abs(thisPlane(ix, ny/2))>itsCutoff*maxCF)) {
								itsSupport=abs(ix-nx/2)/itsOverSample;
								break;
							}
							///  Check on diagonal: ix, ix is correct!
							if ((casa::abs(thisPlane(ix, ix))>itsCutoff*maxCF)) {
								itsSupport=int(1.414*float(abs(ix-nx/2)/itsOverSample));
								break;
							}
							if (nx==ny) {
								/// Check on vertical axis
								if ((casa::abs(thisPlane(nx/2, ix))>itsCutoff*maxCF)) {
									itsSupport=abs(ix-ny/2)/itsOverSample;
									break;
								}
							}
						}
						ASKAPCHECK(itsSupport>0,
								"Unable to determine support of convolution function");
						ASKAPCHECK(itsSupport*itsOverSample<nx/2,
								"Overflowing convolution function - increase maxSupport or decrease overSample")
						itsCSize=2*itsSupport+1;
						ASKAPLOG_INFO_STR(
								logger,
								"Convolution function support = " << itsSupport
										<< " pixels, convolution function size = "
										<< itsCSize << " pixels");
						ASKAPLOG_INFO_STR(logger, "Maximum extent = "
								<< itsSupport*cell << " (m) sampled at "<< cell
								/itsOverSample << " (m)");
						itsCCenter=itsSupport;
						itsConvFunc.resize(itsOverSample*itsOverSample*itsMaxFeeds*itsMaxFields*nChan*itsNWPlanes);
						itsSumWeights.resize(itsMaxFeeds*itsMaxFields*nChan, itsShape(2), itsShape(3));
						itsSumWeights.set(casa::Complex(0.0));
					}
					int zIndex=iw+itsNWPlanes*(chan+nChan*(feed+itsMaxFeeds*itsCurrentField));

					for (int fracu=0; fracu<itsOverSample; fracu++) {
						for (int fracv=0; fracv<itsOverSample; fracv++) {
							int plane=fracu+itsOverSample*(fracv+itsOverSample
									*zIndex);
							itsConvFunc[plane].resize(itsCSize, itsCSize);
							itsConvFunc[plane].set(0.0);
							// Now cut out the inner part of the convolution function and
							// insert it into the convolution function
							for (int iy=-itsSupport; iy<itsSupport; iy++) {
								for (int ix=-itsSupport; ix<itsSupport; ix++) {
									itsConvFunc[plane](ix+itsCCenter, iy
											+itsCCenter) = thisPlane(ix
											*itsOverSample+fracu+nx/2, iy
											*itsOverSample+fracv+ny/2);
								}
							}
						}
					}
				} // w loop
			} // chan loop
		}
	}
	if (nDone==itsMaxFeeds*itsMaxFields*itsNWPlanes) {
		ASKAPLOG_INFO_STR(logger, "Shape of convolution function = "
				<< itsConvFunc[0].shape() << " by "<< itsConvFunc.size()
				<< " planes");
		if (itsName!="") {
			save(itsName);
		}
	}
}

/// To finalize the transform of the weights, we use the following steps:
/// 1. For each plane of the convolution function, transform to image plane
/// and multiply by conjugate to get abs value squared.
/// 2. Sum all planes weighted by the weight for that convolution function.
void AWProjectVisGridder::finaliseWeights(casa::Array<double>& out) {

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
				casa::Complex wt=itsSumWeights(iz, pol, chan);
				if(abs(wt)>0.0) {
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
				casa::Complex wt=itsSumWeights(iz, pol, chan);
				for (int ix=0; ix<cnx; ix++) {
					ip(0)=ix;
					for (int iy=0; iy<cny; iy++) {
						ip(1)=iy;
						cOut(ip)+=casa::real(wt*thisPlane(ix, iy)
								*conj(thisPlane(ix, iy)));
					}
				}
			}
		}
		}
	}
	fftPad(cOut, out);
	// We have to correct twice since this is the square!
	correctConvolution(out);
	correctConvolution(out);
}

void AWProjectVisGridder::fftPad(const casa::Array<double>& in,
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

int AWProjectVisGridder::cIndex(int row, int pol, int chan) {
	return itsCMap(row, pol, chan);
}

}
}
