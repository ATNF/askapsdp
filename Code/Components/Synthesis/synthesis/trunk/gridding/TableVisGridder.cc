#include <gridding/TableVisGridder.h>
#include <conrad/ConradError.h>
#include <conrad/ConradUtil.h>
#include <fft/FFTWrapper.h>

#include <casa/BasicSL/Constants.h>
#include <casa/Arrays/ArrayIter.h>
#include <casa/Arrays/Slicer.h>

#include <measures/Measures/MDirection.h>
#include <measures/Measures/UVWMachine.h>

#include <fitting/Params.h>
#include <fitting/ParamsCasaTable.h>

using namespace conrad::scimath;
using namespace conrad;

#include <stdexcept>
#include <ostream>

namespace conrad
{
	namespace synthesis
	{

		TableVisGridder::TableVisGridder() :
			itsName("")
		{
		}

		TableVisGridder::TableVisGridder(const int overSample, const int support,
		    const std::string& name) :
			itsOverSample(overSample), itsSupport(support), itsName(name)
		{
			CONRADCHECK(overSample>0, "Oversampling must be greater than 0");
			CONRADCHECK(support>0, "Maximum support must be greater than 0");
		}

		TableVisGridder::~TableVisGridder()
		{
		}

		/// Totally selfcontained gridding
		void TableVisGridder::gridKernel(casa::Matrix<casa::Complex>& grid,
		    double sumwt, const casa::Matrix<casa::Complex>& convFunc,
		    const casa::Complex& cVis, const int iu, const int iv, const int support,
		    const int overSample, const int cCenter, const int fracu,
		    const int fracv)
		{
			/// Gridding visibility to grid
			int voff=-overSample*support+fracv+cCenter;
			for (int suppv=-support; suppv<+support; suppv++)
			{
				int uoff=-overSample*support+fracu+cCenter;
				for (int suppu=-support; suppu<+support; suppu++)
				{
					casa::Complex wt=convFunc(uoff, voff);
					grid(iu+uoff, iv+voff)+=cVis*wt;
					sumwt+=casa::real(wt);
					uoff+=itsOverSample;
				}
				voff+=itsOverSample;
			}
		}

		/// Totally selfcontained degridding
		void TableVisGridder::degridKernel(casa::Complex& cVis,
		    const casa::Matrix<casa::Complex>& convFunc,
		    const casa::Matrix<casa::Complex>& grid, const int iu, const int iv,
		    const int support, const int overSample, const int cCenter,
		    const int fracu, const int fracv)
		{
			/// Degridding from grid to visibility
			double sumviswt=0.0;
			int voff=-overSample*support+fracv+cCenter;
			for (int suppv=-support; suppv<+support; suppv++)
			{
				int uoff=-overSample*support+fracu+cCenter;
				for (int suppu=-support; suppu<+support; suppu++)
				{
					casa::Complex wt=conj(convFunc(uoff, voff));
					cVis+=wt*grid(iu+uoff, iv+voff);
					sumviswt+=casa::real(wt);
					uoff+=itsOverSample;
				}
				voff+=itsOverSample;
			}
			if (sumviswt>0.0)
			{
				cVis=conj(cVis)/casa::Complex(sumviswt);
			}
			else
			{
				cVis=0.0;
			}
		}

		void TableVisGridder::save(const std::string& name)
		{
			conrad::scimath::ParamsCasaTable iptable(name, false);
			conrad::scimath::Params ip;
			for (int i=0; i<itsConvFunc.size(); i++)
			{
				casa::Array<double> realC(itsConvFunc(i).shape());
				toDouble(realC, itsConvFunc(i));
				std::ostringstream os;
				os<<"Real.Convolution"<<i;
				ip.add(os.str(), realC);
			}
			iptable.setParameters(ip);
		}

		/// This is a generic grid/degrid
		void TableVisGridder::generic(IDataSharedIter& idi, bool forward)
		{
			casa::Vector<casa::RigidVector<double, 3> > outUVW;
			casa::Vector<double> delay;
			rotateUVW(idi, outUVW, delay);

			initIndices(idi);
			initConvolutionFunction(idi);

			const int nSamples = idi->uvw().size();
			const int nChan = idi->frequency().size();
			const int nPol = idi->visibility().shape()(2);

			// Loop over all samples adding them to the grid
			// First scale to the correct pixel location
			// Then find the fraction of a pixel to the nearest pixel
			// Loop over the entire itsSupport, calculating weights from
			// the convolution function and adding the scaled
			// visibility to the grid.
			for (int i=0; i<nSamples; i++)
			{
				for (int chan=0; chan<nChan; chan++)
				{

					/// Scale U,V to integer pixels plus fractional terms
					double uScaled=idi->frequency()[chan]*idi->uvw()(i)(0)/(casa::C::c*itsUVCellSize(0));
					int iu = nint(uScaled);
					int fracu=nint(itsOverSample*(double(iu)-uScaled));
					iu+=itsShape(0)/2;
					double vScaled=idi->frequency()[chan]*idi->uvw()(i)(1)/(casa::C::c*itsUVCellSize(1));
					int iv = nint(vScaled);
					int fracv=nint(itsOverSample*(double(iv)-vScaled));
					iv+=itsShape(1)/2;

					/// Calculate the delay phasor
					double phase=2.0f*casa::C::pi*idi->frequency()[chan]*delay(i)/(casa::C::c);
					casa::Complex phasor(cos(phase), sin(phase));

					/// Now loop over all polarizations
					for (int pol=0; pol<nPol; pol++)
					{

						/// Make a slicer to extract just this plane
						casa::IPosition ipStart(4, 0, 0, pol, chan);
						casa::IPosition ipLength(4, itsShape(0), itsShape(1), 0, 0);
						casa::Slicer slicer(ipStart, ipLength);

						/// Lookup the portion of grid and convolution function to be
						/// used for this row, polarisation and channel

						int gInd=gIndex(i, pol, chan);

						int cInd=cIndex(i, pol, chan);
						const casa::Matrix<casa::Complex>& convFunc(itsConvFunc(cInd));

						/// Need to check if this point lies on the grid (taking into 
						/// account the support
						if (((iu-itsSupport)>0)&&((iv-itsSupport)>0)&&((iu+itsSupport)
						    <itsShape(0))&&((iv+itsSupport)<itsShape(1)))
						{
							if (forward)
							{
								casa::Complex cVis(idi->visibility()(i, chan, pol));
								const casa::Matrix<casa::Complex> grid(itsGrid(gInd)(slicer));
								degridKernel(cVis, convFunc, grid, iu, iv, itsSupport,
								    itsOverSample, itsCCenter, fracu, fracu);
								idi->rwVisibility()(i, chan, pol)=cVis*phasor;
							}
							else
							{
								/// Gridding visibility data onto grid
								const casa::Complex rVis=phasor*conj(idi->visibility()(i, chan,
								    pol));
								casa::Matrix<casa::Complex> grid(itsGrid(gInd)(slicer));
								gridKernel(grid, itsSumWeights(cInd, pol, chan), convFunc,
								    rVis, iu, iv, itsSupport, itsOverSample, itsCCenter, fracu,
								    fracu);
								if (itsDopsf)
								{
									casa::Matrix<casa::Complex> gridPSF(itsGridPSF(gInd)(slicer));
									double sumwt=0;
									const casa::Complex uVis(1.0);
									gridKernel(gridPSF, sumwt, convFunc, uVis, iu, iv,
									    itsSupport, itsOverSample, itsCCenter, fracu, fracu);
								}
							}
						}
					}
				}
			}
		}

		void TableVisGridder::degrid(IDataSharedIter& idi)
		{
			return generic(idi, true);
		}

		void TableVisGridder::grid(IDataSharedIter& idi)
		{
			return generic(idi, false);
		}

		void TableVisGridder::rotateUVW(IDataSharedIter& idi,
		    casa::Vector<casa::RigidVector<double, 3> >& outUVW,
		    casa::Vector<double>& delay)
		{
			casa::Quantum<double>refLon((itsAxes.start("RA")+itsAxes.end("RA"))/2.0,
			    "rad");
			casa::Quantum<double> refLat((itsAxes.start("DEC")+itsAxes.end("DEC"))
			    /2.0, "rad");
			casa::MDirection out(refLon, refLat, casa::MDirection::J2000);
			const int nSamples = idi->uvw().size();
			delay.resize(nSamples);
			outUVW.resize(nSamples);

			casa::Vector<double> uvw(3);
			for (int row=0; row<nSamples; row++)
			{
				/// @todo Decide what to do about pointingDir1!=pointingDir2
				for (int i=0; i<2; i++)
				{
					uvw(i)=-idi->uvw()(row)(i);
				}
				uvw(2)=idi->uvw()(row)(2);
				casa::UVWMachine machine(out, idi->pointingDir1()(row), false, true);
				machine.convertUVW(delay(row), uvw);
				for (int i=0; i<3; i++)
					outUVW(row)(i)=uvw(i);
			}
		}

		/// Convert from a double array to a casa::Complex array of the
		/// same size. No limits on dimensions.
		void TableVisGridder::toComplex(casa::Array<casa::Complex>& out,
		    const casa::Array<double>& in)
		{
			out.resize(in.shape());

			casa::ReadOnlyArrayIterator<double> inIt(in, 1);
			casa::ArrayIterator<casa::Complex> outIt(out, 1);
			while (!inIt.pastEnd()&&!outIt.pastEnd())
			{
				casa::Vector<double> inVec(inIt.array());
				casa::Vector<casa::Complex> outVec(outIt.array());
				for (uint ix=0; ix<inVec.size(); ix++)
				{
					outVec(ix)=casa::Complex(float(inVec(ix)));
				}
				inIt.next();
				outIt.next();
			}
		}

		/// Convert from a casa::Complex array to a double of the
		/// same size. No limits on dimensions.
		void TableVisGridder::toDouble(casa::Array<double>& out,
		    const casa::Array<casa::Complex>& in)
		{
			out.resize(in.shape());

			casa::ReadOnlyArrayIterator<casa::Complex> inIt(in, 1);
			casa::ArrayIterator<double> outIt(out, 1);
			while (!inIt.pastEnd()&&!outIt.pastEnd())
			{
				casa::Vector<casa::Complex> inVec(inIt.array());
				casa::Vector<double> outVec(outIt.array());
				for (uint ix=0; ix<inVec.size(); ix++)
				{
					outVec(ix)=double(casa::real(inVec(ix)));
				}
				inIt.next();
				outIt.next();
			}
		}

		void TableVisGridder::initialiseGrid(const scimath::Axes& axes,
		    const casa::IPosition& shape, const bool dopsf)
		{
			/// We only need one grid
			itsGrid.resize(1);
			itsGrid(0).resize(shape);
			itsGrid(0).set(0.0);
			itsGridPSF.resize(1);
			if (itsDopsf)
			{
				itsGrid(0).resize(shape);
				itsGrid(0).set(0.0);
			}

			itsAxes=axes;
			itsShape=shape;
			itsDopsf=dopsf;

			if (!itsAxes.has("RA")||!itsAxes.has("DEC"))
			{
				throw(std::invalid_argument("RA and DEC specification not present in axes"));
			}
			double raStart=itsAxes.start("RA");
			double raEnd=itsAxes.end("RA");

			double decStart=itsAxes.start("DEC");
			double decEnd=itsAxes.end("DEC");

			itsUVCellSize.resize(2);
			itsUVCellSize(0)=1.0/(raEnd-raStart);
			itsUVCellSize(1)=1.0/(decEnd-decStart);

		}

		/// This is the default implementation
		void TableVisGridder::finaliseGrid(casa::Array<double>& out)
		{
			out.set(0.0);
			/// Loop over all grids Fourier transforming and accumulating
			for (int i=0; i<itsGrid.size(); i++)
			{
				casa::Array<casa::Complex> scratch(itsGrid(i).copy());
				fft2d(scratch, false);
				if (i==0)
				{
					toDouble(out, scratch);
				}
				else
				{
					casa::Array<double> work(out.shape());
					toDouble(work, itsGrid(i));
					out+=work;
				}
			}
			// Now we can do the convolution correction
			correctConvolution(out);
		}

		/// This is the default implementation
		void TableVisGridder::finalisePSF(casa::Array<double>& out)
		{
			out.set(0.0);
			/// Loop over all grids Fourier transforming and accumulating
			for (int i=0; i<itsGrid.size(); i++)
			{
				casa::Array<casa::Complex> scratch(itsGridPSF(i).copy());
				fft2d(scratch, false);
				if (i==0)
				{
					toDouble(out, scratch);
				}
				else
				{
					casa::Array<double> work(out.shape());
					toDouble(work, itsGridPSF(i));
					out+=work;
				}
			}
			// Now we can do the convolution correction
			correctConvolution(out);
		}

		/// This is the default implementation
		void TableVisGridder::finaliseWeights(casa::Array<double>& out)
		{
			int nx=itsShape(0);
			int ny=itsShape(1);
			int nPol=itsShape(2);
			int nChan=itsShape(3);

			int nZ=itsSumWeights.shape()(0);

			casa::ArrayIterator<double> it(out);

			for (int chan=0; chan<nChan; chan++)
				for (int pol=0; pol<nPol; pol++)
				{
					double sumwt=0.0;
					for (int iz=0; iz<nZ; iz++)
					{
						sumwt+=itsSumWeights(iz, pol, chan);
					}
					sumwt=sumwt/(double(nx)*double(ny));
					it.array().set(sumwt);
					it.next();
				}
		}

		void TableVisGridder::initialiseDegrid(const scimath::Axes& axes,
		    const casa::Array<double>& in)
		{

			itsAxes=axes;
			itsShape=in.shape();

			if (!itsAxes.has("RA")||!itsAxes.has("DEC"))
			{
				throw(std::invalid_argument("RA and DEC specification not present in axes"));
			}
			double raStart=itsAxes.start("RA");
			double raEnd=itsAxes.end("RA");

			double decStart=itsAxes.start("DEC");
			double decEnd=itsAxes.end("DEC");

			itsUVCellSize.resize(2);
			itsUVCellSize(0)=1.0/(raEnd-raStart);
			itsUVCellSize(1)=1.0/(decEnd-decStart);

			/// We only need one grid
			itsGrid.resize(1);
			itsGrid(0).resize(itsShape);

			casa::Array<double> scratch(in.copy());
			correctConvolution(scratch);
			toComplex(itsGrid(0), scratch);
			fft2d(itsGrid(0), true);
		}

		/// This is the default implementation
		void TableVisGridder::finaliseDegrid()
		{
			/// Nothing to do
		}

		/// This is the default implementation
		int TableVisGridder::cIndex(int row, int pol, int chan)
		{
			return 0;
		}

		/// This is the default implementation
		int TableVisGridder::gIndex(int row, int pol, int chan)
		{
			return 0;
		}

	}

}
