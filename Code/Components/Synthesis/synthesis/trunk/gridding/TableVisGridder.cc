#include <gridding/TableVisGridder.h>
#include <askap_synthesis.h>
#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, ".gridding");

#include <askap/AskapError.h>
#include <askap/AskapUtil.h>
#include <fft/FFTWrapper.h>

#include <casa/BasicSL/Constants.h>
#include <casa/Arrays/ArrayIter.h>
#include <casa/Arrays/Slicer.h>

#include <measures/Measures/MDirection.h>
#include <measures/Measures/UVWMachine.h>

#include <fitting/Params.h>
#include <fitting/ParamsCasaTable.h>

#include <gridding/GridKernel.h>

using namespace askap::scimath;
using namespace askap;

#include <ostream>

#include <casa/OS/Timer.h>

namespace askap {
namespace synthesis {

TableVisGridder::TableVisGridder() :
	itsName(""), itsModelIsEmpty(false), itsSamplesGridded(0),
			itsSamplesDegridded(0), itsNumberGridded(0), itsNumberDegridded(0),
			itsTimeGridded(0.0), itsTimeDegridded(0.0)

{
}

TableVisGridder::TableVisGridder(const int overSample, const int support,
		const std::string& name) :
	itsSupport(support), itsOverSample(overSample), itsName(name),
			itsModelIsEmpty(false), itsSamplesGridded(0),
			itsSamplesDegridded(0), itsNumberGridded(0), itsNumberDegridded(0),
			itsTimeGridded(0.0), itsTimeDegridded(0.0) {
	ASKAPCHECK(overSample>0, "Oversampling must be greater than 0");
	ASKAPCHECK(support>0, "Maximum support must be greater than 0");
}

TableVisGridder::~TableVisGridder() {
	if (itsNumberGridded>0) {
		ASKAPLOG_INFO_STR(logger, "TableVisGridder gridding statistics");
		ASKAPLOG_INFO_STR(logger, "   " << GridKernel::info());
		ASKAPLOG_INFO_STR(logger, "   Total time gridding   = "
				<< itsTimeGridded << " (s)");
		ASKAPLOG_INFO_STR(logger, "   Samples gridded       = "
				<< itsSamplesGridded);
		ASKAPLOG_INFO_STR(logger, "   Time per Sample       = " << 1e9
				*itsTimeGridded/itsSamplesGridded << " (ns)");
		ASKAPLOG_INFO_STR(logger, "   Points gridded        = "
				<< itsNumberGridded);
		ASKAPLOG_INFO_STR(logger, "   Time per point        = " << 1e9
				*itsTimeGridded/itsNumberGridded << " (ns)");
	}

	if (itsNumberDegridded>0) {
		ASKAPLOG_INFO_STR(logger, "TableVisGridder degridding statistics");
		ASKAPLOG_INFO_STR(logger, "   " << GridKernel::info());
		ASKAPLOG_INFO_STR(logger, "   Total time degridding = "
				<< itsTimeDegridded << " (s)");
		ASKAPLOG_INFO_STR(logger, "   Samples degridded     = "
				<< itsSamplesDegridded);
		ASKAPLOG_INFO_STR(logger, "   Time per Sample       = " << 1e9
				*itsTimeDegridded/itsSamplesDegridded << " (ns)");
		ASKAPLOG_INFO_STR(logger, "   Points degridded      = "
				<< itsNumberDegridded);
		ASKAPLOG_INFO_STR(logger, "   Time per point        = " << 1e9
				*itsTimeDegridded/itsNumberDegridded << " (ns)");
	}
}

void TableVisGridder::save(const std::string& name) {
	askap::scimath::ParamsCasaTable iptable(name, false);
	askap::scimath::Params ip;
	for (unsigned int i=0; i<itsConvFunc.size(); i++) {
		{
			casa::Array<double> realC(itsConvFunc[i].shape());
			toDouble(realC, itsConvFunc[i]);
			std::ostringstream os;
			os<<"Real.Convolution";
			os.width(5);
			os.fill('0');
			os<<i;
			ip.add(os.str(), realC);
		}
	}
	iptable.setParameters(ip);
}

/// This is a generic grid/degrid
void TableVisGridder::generic(IDataSharedIter& idi, bool forward) {
	if (forward&&itsModelIsEmpty)
		return;

	casa::Vector<casa::RigidVector<double, 3> > outUVW;
	casa::Vector<double> delay;
	rotateUVW(*idi, outUVW, delay);

	initIndices(idi);
	initConvolutionFunction(idi);

	ASKAPCHECK(itsSupport>0, "Support must be greater than 0");
	ASKAPCHECK(itsUVCellSize.size()==2, "UV cell sizes not yet set");

	const int nSamples = idi->uvw().size();
	ASKAPDEBUGASSERT(nSamples>0);
	const int nChan = idi->frequency().size();
	ASKAPDEBUGASSERT(nChan>0);
	const int nPol = idi->visibility().shape()(2);
	ASKAPDEBUGASSERT(nPol>0);

	ASKAPDEBUGASSERT(itsShape.nelements()>=2);
	const casa::IPosition onePlane4D(4, itsShape(0), itsShape(1), 1, 1);
	const casa::IPosition onePlane(2, itsShape(0), itsShape(1));

	// Loop over all samples adding them to the grid
	// First scale to the correct pixel location
	// Then find the fraction of a pixel to the nearest pixel
	// Loop over the entire itsSupport, calculating weights from
	// the convolution function and adding the scaled
	// visibility to the grid.
	casa::Timer timer;

	timer.mark();

	long int nGood=0;

	ASKAPDEBUGASSERT(casa::uInt(nChan) <= idi->frequency().nelements());
	ASKAPDEBUGASSERT(casa::uInt(nSamples) == idi->uvw().nelements());

	for (int i=0; i<nSamples; i++) {
		/// Temporarily fix to do MFS only
		int imageChan=0;
		int imagePol=0;

		for (int chan=0; chan<nChan; chan++) {

			/// Scale U,V to integer pixels plus fractional terms
			double uScaled=idi->frequency()[chan]*idi->uvw()(i)(0)/(casa::C::c *itsUVCellSize(0));
			int iu = askap::nint(uScaled);
			int fracu=askap::nint(itsOverSample*(double(iu)-uScaled));
			if (fracu<0) {
				iu+=1;
			}
			if (fracu>=itsOverSample) {
				iu-=1;
			}
			fracu=askap::nint(itsOverSample*(double(iu)-uScaled));
			ASKAPCHECK(fracu>-1, "Fractional offset in u is negative");
			ASKAPCHECK(fracu<itsOverSample,
					"Fractional offset in u exceeds oversampling");
			iu+=itsShape(0)/2;

			double vScaled=idi->frequency()[chan]*idi->uvw()(i)(1)/(casa::C::c *itsUVCellSize(1));
			int iv = askap::nint(vScaled);
			int fracv=askap::nint(itsOverSample*(double(iv)-vScaled));
			if (fracv<0) {
				iv+=1;
			}
			if (fracv>=itsOverSample) {
				iv-=1;
			}
			fracv=askap::nint(itsOverSample*(double(iv)-vScaled));
			ASKAPCHECK(fracv>-1, "Fractional offset in v is negative");
			ASKAPCHECK(fracv<itsOverSample,
					"Fractional offset in v exceeds oversampling");
			iv+=itsShape(1)/2;

			/// Calculate the delay phasor
			const double phase=2.0f*casa::C::pi*idi->frequency()[chan]*delay(i)/(casa::C::c);
			const casa::Complex phasor(cos(phase), sin(phase));

			bool allPolGood=true;
			for (int pol=0; pol<nPol; pol++) {
				if (idi->flag()(i, chan, pol))
					allPolGood=false;
			}
			/// Now loop over all visibility polarizations
			for (int pol=0; pol<nPol; pol++) {
				/// Ensure that we only use unflagged data
				/// @todo Be more careful about matching polarizations
				if (allPolGood) {
					// Lookup the portion of grid to be
					// used for this row, polarisation and channel
					const int gInd=gIndex(i, pol, chan);
					ASKAPCHECK(gInd>-1,
							"Index into image grid is less than zero");
					ASKAPCHECK(gInd<int(itsGrid.size()),
							"Index into image grid exceeds number of planes");

					// MFS override of imagePol applies to gridding only
					// degridding should treat polarisations independently
					if (forward) {
						imagePol = pol;
						const casa::IPosition gridShape = itsGrid[gInd].shape();
						bool noXPols = false;
						if (gridShape.nelements()<=2) {
							// grid is a 2D image, treat the source as unpolarised
							noXPols = true;
							imagePol = 0;
						} else {
							// we need a better way to handle the order of polarisations
							ASKAPCHECK((gridShape[2] == 1) || (gridShape[2]
									== 2) || (gridShape[2] == 4),
									"only 1,2 and 4 polarisations are supported, "
										"current grid shape is "<<gridShape);
							if (gridShape[2] == 1) {
								noXPols = true; // unpolarized source
								imagePol = 0;
							}
							if (gridShape[2] == 2) {
								noXPols = true; // grid has 2 polarisations, treat them as parallel-hand products
								imagePol = (pol == 3) ? 1 : 0;
							}
						}
						// we need a better way to handle the order of polarisations
						if (((pol == 1) || (pol == 2)) && noXPols) {
							continue; // skip polarisation, nothing is degridded
						}
					}
					/// Make a slicer to extract just this plane
					/// @todo Enable pol and chan maps
					const casa::IPosition ipStart(4, 0, 0, imagePol, imageChan);
					const casa::Slicer slicer(ipStart, onePlane4D);

					// Lookup the convolution function to be
					// used for this row, polarisation and channel
					const int cInd=fracu+itsOverSample*(fracv+itsOverSample
							*cIndex(i, pol, chan));
					ASKAPCHECK(cInd>-1,
							"Index into convolution functions is less than zero");
					ASKAPCHECK(cInd<int(itsConvFunc.size()),
							"Index into convolution functions exceeds number of planes");

					casa::Matrix<casa::Complex> & convFunc(itsConvFunc[cInd]);

					casa::Array<casa::Complex> aGrid(itsGrid[gInd](slicer));
					casa::Matrix<casa::Complex> grid(aGrid.nonDegenerate());

					/// Need to check if this point lies on the grid (taking into 
					/// account the support)
					if (((iu-itsSupport)>0)&&((iv-itsSupport)>0)&&((iu
							+itsSupport) <itsShape(0))&&((iv+itsSupport)
							<itsShape(1))) {
						nGood+=1;
						if (forward) {
							casa::Complex cVis(idi->visibility()(i, chan, pol));
							GridKernel::degrid(cVis, convFunc, grid, iu, iv,
									itsSupport);
							idi->rwVisibility()(i, chan, pol)=cVis*phasor;
						} else {
							/// Gridding visibility data onto grid
							const casa::Complex rVis=phasor
									*conj(idi->visibility()(i, chan, pol));
							casa::Complex sumwt=0.0;
							const float wtVis(1.0);
							GridKernel::grid(grid, sumwt, convFunc, rVis,
									wtVis, iu, iv, itsSupport);

							itsSumWeights(cInd, imagePol, imageChan)+=sumwt;

							/// Grid PSF?
							if (itsDopsf) {
								ASKAPDEBUGASSERT(gInd<int(itsGridPSF.size()));
								casa::Array<casa::Complex>
										aGridPSF(itsGridPSF[gInd](slicer));
								casa::Matrix<casa::Complex>
										gridPSF(aGridPSF.nonDegenerate());
								const casa::Complex uVis(1.0);
								GridKernel::grid(gridPSF, sumwt, convFunc,
										uVis, wtVis, iu, iv, itsSupport);
							}
						}
					}
				}
			}
		}
	}
	if (forward) {
		itsTimeDegridded+=timer.real();
		itsSamplesDegridded+=double(nGood);
		itsNumberDegridded+=double((2*itsSupport+1)*(2*itsSupport+1))*double(nGood);
	} else {
		itsTimeGridded+=timer.real();
		itsSamplesGridded+=double(nGood);
		itsNumberGridded+=double((2*itsSupport+1)*(2*itsSupport+1))*double(nGood);
		if (itsDopsf) {
			itsSamplesGridded+=double(nGood);
			itsNumberGridded+=double((2*itsSupport+1)*(2*itsSupport+1))*double(nGood);
		}
	}
}

void TableVisGridder::degrid(IDataSharedIter& idi) {
	return generic(idi, true);
}

void TableVisGridder::grid(IDataSharedIter& idi) {
	return generic(idi, false);
}

/// @brief Find the change in delay required
/// @details
/// @param[in] acc data accessor to take the input data from
/// @param[out] outUVW Rotated uvw
/// @param[out] delay Delay change (m)
/// @note output vectors are resized to match the accessor's number of rows
void TableVisGridder::rotateUVW(const IConstDataAccessor& acc,
		casa::Vector<casa::RigidVector<double, 3> >& outUVW,
		casa::Vector<double>& delay) const {
	const casa::Quantum<double> refLon( (itsAxes.start("RA") +itsAxes.end("RA"))
			/2.0, "rad");
	const casa::Quantum<double> refLat( (itsAxes.start("DEC")
			+itsAxes.end("DEC")) /2.0, "rad");
	const casa::MDirection out(refLon, refLat, casa::MDirection::J2000);
	const casa::uInt nSamples = acc.uvw().size();
	delay.resize(nSamples);
	outUVW.resize(nSamples);

	const casa::Vector<casa::RigidVector<double, 3> >& uvwVector = acc.uvw();
	const casa::Vector<casa::MVDirection>& pointingDir1Vector =
			acc.pointingDir1();
	for (casa::uInt row=0; row<nSamples; ++row) {
		const casa::RigidVector<double, 3> &uvwRow = uvwVector(row);
		casa::Vector<double> uvw(3);
		/// @todo Decide what to do about pointingDir1!=pointingDir2
		for (int i=0; i<2; ++i) {
			uvw(i)=-uvwRow(i);
		}
		uvw(2)=uvwRow(2);

		casa::UVWMachine machine(out, pointingDir1Vector(row), false, true);
		machine.convertUVW(delay(row), uvw);

		for (int i=0; i<3; ++i) {
			outUVW(row)(i)=uvw(i);
		}
	}
}

/// Convert from a double array to a casa::Complex array of the
/// same size. No limits on dimensions.
void TableVisGridder::toComplex(casa::Array<casa::Complex>& out,
		const casa::Array<double>& in) {
	out.resize(in.shape());
	int nx=in.shape()(0);
	int ny=in.shape()(1);

	casa::ReadOnlyArrayIterator<double> inIt(in, 2);
	casa::ArrayIterator<casa::Complex> outIt(out, 2);
	while (!inIt.pastEnd()&&!outIt.pastEnd()) {
		casa::Matrix<double> inMat(inIt.array());
		casa::Matrix<casa::Complex> outMat(outIt.array());
		for (int iy=0; iy<ny; iy++) {
			for (int ix=0; ix<nx; ix++) {
				outMat(ix, iy)=casa::Complex(float(inMat(ix,iy)));
			}
		}
		inIt.next();
		outIt.next();
	}
}

/// Convert from a casa::Complex array to a double of the
/// same size. No limits on dimensions.
void TableVisGridder::toDouble(casa::Array<double>& out,
		const casa::Array<casa::Complex>& in) {
	out.resize(in.shape());
	int nx=in.shape()(0);
	int ny=in.shape()(1);

	casa::ReadOnlyArrayIterator<casa::Complex> inIt(in, 2);
	casa::ArrayIterator<double> outIt(out, 2);
	while (!inIt.pastEnd()&&!outIt.pastEnd()) {
		casa::Matrix<casa::Complex> inMat(inIt.array());
		casa::Matrix<double> outMat(outIt.array());
		for (int iy=0; iy<ny; iy++) {
			for (int ix=0; ix<nx; ix++) {
				outMat(ix, iy)=double(casa::real(inMat(ix,iy)));
			}
		}
		inIt.next();
		outIt.next();
	}
}

void TableVisGridder::initialiseGrid(const scimath::Axes& axes,
		const casa::IPosition& shape, const bool dopsf) {
	itsAxes=axes;
	itsShape=shape;
	itsDopsf=dopsf;

	/// We only need one grid
	itsGrid.resize(1);
	itsGrid[0].resize(shape);
	itsGrid[0].set(0.0);
	if (itsDopsf) {
		itsGridPSF.resize(1);
		itsGridPSF[0].resize(shape);
		itsGridPSF[0].set(0.0);
	}

	itsSumWeights.resize(1, itsShape(2), itsShape(3));
	itsSumWeights.set(casa::Complex(0.0));

	ASKAPCHECK(itsAxes.has("RA")&&itsAxes.has("DEC"),
			"RA and DEC specification not present in axes");

	double raStart=itsAxes.start("RA");
	double raEnd=itsAxes.end("RA");

	double decStart=itsAxes.start("DEC");
	double decEnd=itsAxes.end("DEC");

	itsUVCellSize.resize(2);
	itsUVCellSize(0)=1.0/(raEnd-raStart);
	itsUVCellSize(1)=1.0/(decEnd-decStart);

}

/// This is the default implementation
void TableVisGridder::finaliseGrid(casa::Array<double>& out) {
	/// Loop over all grids Fourier transforming and accumulating
	for (unsigned int i=0; i<itsGrid.size(); i++) {
		casa::Array<casa::Complex> scratch(itsGrid[i].copy());
		fft2d(scratch, false);
		if (i==0) {
			toDouble(out, scratch);
		} else {
			casa::Array<double> work(out.shape());
			toDouble(work, scratch);
			out+=work;
		}
	}
	// Now we can do the convolution correction
	correctConvolution(out);
	out*=double(out.shape()(0))*double(out.shape()(1));
}

/// This is the default implementation
void TableVisGridder::finalisePSF(casa::Array<double>& out) {
	/// Loop over all grids Fourier transforming and accumulating
	for (unsigned int i=0; i<itsGridPSF.size(); i++) {
		casa::Array<casa::Complex> scratch(itsGridPSF[i].copy());
		fft2d(scratch, false);
		if (i==0) {
			toDouble(out, scratch);
		} else {
			casa::Array<double> work(out.shape());
			toDouble(work, scratch);
			out+=work;
		}
	}
	// Now we can do the convolution correction
	correctConvolution(out);
	out*=double(out.shape()(0))*double(out.shape()(1));
}

/// This is the default implementation
void TableVisGridder::finaliseWeights(casa::Array<double>& out) {
	int nPol=itsShape(2);
	int nChan=itsShape(3);

	int nZ=itsSumWeights.shape()(0);

	for (int chan=0; chan<nChan; chan++) {
		for (int pol=0; pol<nPol; pol++) {
			double sumwt=0.0;
			for (int iz=0; iz<nZ; iz++) {
				sumwt+=casa::real(itsSumWeights(iz, pol, chan));
			}

			casa::IPosition ipStart(4, 0, 0, pol, chan);
			casa::IPosition onePlane(4, itsShape(0), itsShape(1), 1, 1);
			casa::Slicer slicer(ipStart, onePlane);
			out(slicer).set(sumwt);
		}
	}
}

void TableVisGridder::initialiseDegrid(const scimath::Axes& axes,
		const casa::Array<double>& in) {

	itsAxes=axes;
	itsShape=in.shape();

	ASKAPCHECK(itsAxes.has("RA")&&itsAxes.has("DEC"),
			"RA and DEC specification not present in axes");

	double raStart=itsAxes.start("RA");
	double raEnd=itsAxes.end("RA");

	double decStart=itsAxes.start("DEC");
	double decEnd=itsAxes.end("DEC");

	itsUVCellSize.resize(2);
	itsUVCellSize(0)=1.0/(raEnd-raStart);
	itsUVCellSize(1)=1.0/(decEnd-decStart);

	/// We only need one grid
	itsGrid.resize(1);
	itsGrid[0].resize(itsShape);

	if (casa::max(casa::abs(in))>0.0) {
		itsModelIsEmpty=false;
		casa::Array<double> scratch(in.copy());
		correctConvolution(scratch);
		toComplex(itsGrid[0], scratch);
		fft2d(itsGrid[0], true);
	} else {
		ASKAPLOG_INFO_STR(logger, "No need to degrid: model is empty");
		itsModelIsEmpty=true;
		itsGrid[0].set(casa::Complex(0.0));
	}
}

/// This is the default implementation
void TableVisGridder::finaliseDegrid() {
	/// Nothing to do
}

/// This is the default implementation
int TableVisGridder::cIndex(int row, int pol, int chan) {
	return 0;
}

/// This is the default implementation
int TableVisGridder::gIndex(int row, int pol, int chan) {
	return 0;
}

}

}
