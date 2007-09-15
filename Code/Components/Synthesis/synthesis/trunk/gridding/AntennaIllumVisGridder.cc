#include <gridding/AntennaIllumVisGridder.h>

#include <casa/BasicSL/Complex.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/ArrayMath.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/UVWMachine.h>

#include <conrad/ConradError.h>
#include <conrad/ConradUtil.h>

#include <fft/FFTWrapper.h>

using namespace conrad;

namespace conrad
{
	namespace synthesis
	{

		AntennaIllumVisGridder::AntennaIllumVisGridder(const double diameter,
		    const double blockage, const double wmax, const int nwplanes,
		    const double cutoff, const int overSample, const int maxSupport,
		    const int maxFeeds, const std::string& name) :
			WProjectVisGridder(wmax, nwplanes, cutoff, overSample, maxSupport, name),
			    itsReferenceFrequency(0.0), itsDiameter(diameter),
			    itsBlockage(blockage), itsMaxFeeds(maxFeeds)

		{
			CONRADCHECK(diameter>0.0, "Blockage must be positive");
			CONRADCHECK(diameter>blockage,
					"Antenna diameter must be greater than blockage");
			CONRADCHECK(blockage>=0.0, "Blockage must be non-negative");
			CONRADCHECK(maxFeeds>0, "Maximum number of feeds must be one or more");
		}

		AntennaIllumVisGridder::~AntennaIllumVisGridder()
		{
		}

		/// Clone a copy of this Gridder
		IVisGridder::ShPtr AntennaIllumVisGridder::clone()
		{
			return IVisGridder::ShPtr(new AntennaIllumVisGridder(*this));
		}

		/// Initialize the indices into the cube.
		void AntennaIllumVisGridder::initIndices(IDataSharedIter& idi)
		{
			/// We have to calculate the lookup function converting from
			/// row and channel to plane of the w-dependent convolution
			/// function
			const int nSamples = idi->uvw().size();
			const int nChan = idi->frequency().size();
			const int nPol = idi->rwVisibility().shape()(2);
			itsCMap.resize(nSamples, nPol, nChan);
			itsCMap.set(0);
			/// @todo Select max feeds more carefully

			int cenw=(itsNWPlanes-1)/2;
			for (int i=0; i<nSamples; i++)
			{
				int feed=idi->feed1()(i);
				CONRADCHECK(feed<itsMaxFeeds, "Exceeded specified maximum number of feeds");
				CONRADCHECK(feed>-1, "Illegal negative feed number");

				for (int chan=0; chan<nChan; chan++)
				{
					double freq=idi->frequency()[chan];
					double w=(idi->uvw()(i)(2))/(casa::C::c);
					int iw=cenw+int(w*freq/itsWScale);
					CONRADCHECK(iw<itsNWPlanes,
							"W scaling error: recommend allowing larger range of w");
					CONRADCHECK(iw>-1,
							"W scaling error: recommend allowing larger range of w");

					for (int pol=0; pol<nPol; pol++)
					{
						/// Order is (iw, chan, feed)
						itsCMap(i, pol, chan)=iw+itsNWPlanes*chan+nChan*itsNWPlanes*feed;
					}
				}
			}
		}

		/// Initialize the convolution function into the cube. If necessary this
		/// could be optimized by using symmetries.
		/// @todo Make initConvolutionFunction more robust
		void AntennaIllumVisGridder::initConvolutionFunction(IDataSharedIter& idi)
		{
			if (itsSupport!=0)
				return;

			double refFreq((itsAxes.start("FREQUENCY")+itsAxes.end("FREQUENCY"))/2.0);

			/// We have to calculate the lookup function converting from
			/// row and channel to plane of the w-dependent convolution
			/// function
			const int nSamples = idi->uvw().size();
			const int nChan = idi->frequency().size();
			int cenw=(itsNWPlanes-1)/2;

			/// Get the pointing direction
			casa::Matrix<double> slope;
			findCollimation(idi, slope);
			int nFeeds=slope.nrow();

			itsSupport=0;

			/// These are the actual image pixel sizes used
			double cellx=1.0/(double(itsShape(0))*itsUVCellSize(0));
			double celly=1.0/(double(itsShape(1))*itsUVCellSize(1));

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
			for (int ix=0; ix<qnx; ix++)
			{
				float nux=std::abs(float(ix-qnx/2))/float(qnx/2);
				ccfx(ix)=grdsf(nux)/float(qnx);
			}
			for (int iy=0; iy<qny; iy++)
			{
				float nuy=std::abs(float(iy-qny/2))/float(qny/2);
				ccfy(iy)=grdsf(nuy)/float(qny);
			}

			int zIndex=0;
			for (int feed=0; feed<itsMaxFeeds; feed++)
			{

				for (int chan=0; chan<nChan; chan++)
				{
					/// Slope is the turns per wavelength so we need to convert from the
					/// image reference frequency to the channel frequency
					double ax=2.0f*casa::C::pi*itsUVCellSize(0)*slope(0, feed)
					    *idi->frequency()[chan]/refFreq;
					double ay=2.0f*casa::C::pi*itsUVCellSize(1)*slope(1, feed)
					    *idi->frequency()[chan]/refFreq;

					/// Make the disk for this channel
					casa::Matrix<casa::Complex> disk(qnx, qny);
					disk.set(0.0);

					/// Calculate the size of one cell in m.
					double cell=std::abs(itsUVCellSize(0)*(casa::C::c/idi->frequency()[chan]));
					double rmax=std::pow(itsDiameter/(2.0*cell), 2);
					double rmin=std::pow(itsBlockage/(2.0*cell), 2);

					/// Calculate the antenna voltage pattern, including the
					/// phase shift due to pointing
					for (int ix=0; ix<qnx; ix++)
					{
						double nux=double(ix-qnx/2);
						double nux2=nux*nux;
						for (int iy=0; iy<qny; iy++)
						{
							double nuy=double(iy-qny/2);
							double nuy2=nuy*nuy;
							double r=nux2+nuy2;
							if ((r>=rmin)&&(r<=rmax))
							{
								double phase=ax*nux+ay*nuy;
								disk(ix, iy)=casa::Complex(cos(phase), -sin(phase));
							}
						}
					}
					// Ensure that there is always one point filled
					disk(qnx/2, qny/2)=casa::Complex(1.0);
					fft2d(disk, false);
					disk=disk*conj(disk);
					float peak=casa::real(casa::max(casa::abs(disk)));
					CONRADCHECK(peak>0.0, "Synthetic primary beam is empty");
					disk/=casa::Complex(peak);

					/// Calculate the total convolution function including
					/// the w term and the antenna convolution function
					casa::Matrix<casa::Complex> thisPlane(nx, ny);

					for (int iw=0; iw<itsNWPlanes; iw++)
					{
						thisPlane.set(0.0);

						// Loop over the central nx, ny region, setting it to the product
						// of the phase screen and the spheroidal function
						double maxCF=0.0;
						double w=2.0f*casa::C::pi*double(iw-cenw)*itsWScale;
						double freq=idi->frequency()[chan];
						for (int iy=0; iy<qny; iy++)
						{
							double y2=double(iy-qny/2)*ccelly;
							y2*=y2;
							for (int ix=0; ix<qnx; ix++)
							{
								double x2=double(ix-qnx/2)*ccellx;
								x2*=x2;
								double r2=x2+y2;
								double phase=w*(1.0-sqrt(1.0-r2));
								casa::Complex wt=disk(ix, iy)*casa::Complex(ccfx(iy)*ccfy(ix));
								thisPlane(ix-qnx/2+nx/2, iy-qny/2+ny/2)=wt*casa::Complex(
								    cos(phase), -sin(phase));
								maxCF+=casa::abs(disk(ix, iy));
							}
						}
						maxCF/=(double(nx)*double(ny));
						// At this point, we have the phase screen multiplied by the spheroidal
						// function, sampled on larger cellsize (itsOverSample larger) in image
						// space. Only the inner qnx, qny pixels have a non-zero value

						// Now we have to calculate the Fourier transform to get the
						// convolution function in uv space
						fft2d(thisPlane, true);

						// If the support is not yet set, find it and size the
						// convolution function appropriately
						if (itsSupport==0)
						{
							// Find the support by starting from the edge and
							// working in
							for (int ix=0; ix<nx/2; ix++)
							{
								/// Check on horizontal axis
								if ((casa::abs(thisPlane(ix, ny/2))>itsCutoff*maxCF))
								{
									itsSupport=abs(ix-nx/2)/itsOverSample;
									break;
								}
								///  Check on diagonal
								if ((casa::abs(thisPlane(ix, ix))>itsCutoff*maxCF))
								{
									itsSupport=abs(int(1.414*float(ix))-nx/2)/itsOverSample;
									break;
								}
								if (nx==ny)
								{
									/// Check on vertical axis
									if ((casa::abs(thisPlane(nx/2, ix))>itsCutoff*maxCF))
									{
										itsSupport=abs(ix-ny/2)/itsOverSample;
										break;
									}
								}
							}
							CONRADCHECK(itsSupport*itsOverSample<nx/2, "Overflowing convolution function - increase maxSupport or decrease overSample")
							itsCSize=2*(itsSupport+1)*itsOverSample;
							std::cout << "Convolution function support = "<< itsSupport
							    << " pixels, convolution function size = "<< itsCSize
							    << " pixels"<< std::endl;
							std::cout << "Maximum extent = "<< itsCSize*cell/itsOverSample
							    << " (m) sampled at "<< cell << " (m)"<< std::endl;
							itsCCenter=itsCSize/2-1;
							itsConvFunc.resize(itsMaxFeeds*nChan*itsNWPlanes);
							itsSumWeights.resize(itsMaxFeeds*nChan*itsNWPlanes, itsShape(2),
							    itsShape(3));
							itsSumWeights.set(0.0);
						}
						zIndex=iw+itsNWPlanes*chan+nChan*itsNWPlanes*feed;

						itsConvFunc[zIndex].resize(itsCSize, itsCSize);
						itsConvFunc[zIndex].set(0.0);
						// Now cut out the inner part of the convolution function and
						// insert it into the convolution function
						for (int iy=-itsOverSample*itsSupport; iy<+itsOverSample*itsSupport; iy++)
						{
							for (int ix=-itsOverSample*itsSupport; ix<+itsOverSample
							    *itsSupport; ix++)
							{
								itsConvFunc[zIndex](ix+itsCCenter, iy+itsCCenter)=thisPlane(ix
								    +nx/2, iy+ny/2);
							}
						}
					} // w loop
				} // chan loop
			} // feed loop
			std::cout << "Shape of convolution function = "<< itsConvFunc[0].shape()
			    << " by "<< itsConvFunc.size()<< " planes"<< std::endl;
			if (itsName!="")
				save(itsName);
		}

		/// To finalize the transform of the weights, we use the following steps:
		/// 1. For each plane of the convolution function, transform to image plane
		/// and multiply by conjugate to get abs value squared.
		/// 2. Sum all planes weighted by the weight for that convolution function.
		void AntennaIllumVisGridder::finaliseWeights(casa::Array<double>& out)
		{

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

			for (int iz=0; iz<nZ; iz++)
			{
				thisPlane.set(0.0);

				// Now fill the inner part of the uv plane with the convolution function
				// and transform to obtain the image. The uv sampling is fixed here
				// so when we transform to image spaces with a smaller number of
				// pixels, we end up with an image with larger pixels. Therefore
				// we will have to FFT pad to get to the full resolution.
				for (int iy=-itsSupport; iy<+itsSupport; iy++)
				{
					for (int ix=-itsSupport; ix<+itsSupport; ix++)
					{
						thisPlane(ix+ccenx, iy+cceny)=itsConvFunc[iz](itsOverSample*ix
						    +itsCCenter, itsOverSample*iy+itsCCenter);
					}
				}

				thisPlane*=casa::Complex(double(cnx)*double(cny));
				/// The peak here should be unity
				fft2d(thisPlane, false);

				for (int chan=0; chan<nChan; chan++)
				{
					for (int pol=0; pol<nPol; pol++)
					{
						for (int ix=0; ix<cnx; ix++)
						{
							for (int iy=0; iy<cny; iy++)
							{
								cOut(casa::IPosition(4, ix, iy, pol, chan))+=itsSumWeights(iz,
								    pol, chan)*real(thisPlane(ix, iy)*conj(thisPlane(ix, iy)));
							}
						}
					}
				}
			}
			fftPad(cOut, out);
		}

		void AntennaIllumVisGridder::fftPad(const casa::Array<double>& in,
		    casa::Array<double>& out)
		{

			int inx=in.shape()(0);
			int iny=in.shape()(1);

			int onx=out.shape()(0);
			int ony=out.shape()(1);

			// Shortcut no-op
			if ((inx==onx)&&(iny==ony))
			{
				out=in.copy();
				return;
			}

			CONRADCHECK(onx>=inx, "Attempting to pad to smaller array");
			CONRADCHECK(ony>=iny, "Attempting to pad to smaller array");

			/// Make an iterator that returns plane by plane
			casa::ReadOnlyArrayIterator<double> inIt(in, 2);
			casa::ArrayIterator<double> outIt(out, 2);
			while (!inIt.pastEnd()&&!outIt.pastEnd())
			{
				casa::Matrix<casa::DComplex> inPlane(inx, iny);
				casa::Matrix<casa::DComplex> outPlane(onx, ony);
				casa::convertArray(inPlane, inIt.array());
				outPlane.set(0.0);
				fft2d(inPlane, false);
				for (int iy=0; iy<iny; iy++)
				{
					for (int ix=0; ix<inx; ix++)
					{
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

		int AntennaIllumVisGridder::cIndex(int row, int pol, int chan)
		{
			return itsCMap(row, pol, chan);
		}

		void AntennaIllumVisGridder::findCollimation(IDataSharedIter& idi,
		    casa::Matrix<double>& slope)
		{
			casa::Quantum<double>refLon((itsAxes.start("RA")+itsAxes.end("RA"))/2.0,
			    "rad");
			casa::Quantum<double> refLat((itsAxes.start("DEC")+itsAxes.end("DEC"))
			    /2.0, "rad");
			casa::MDirection out(refLon, refLat, casa::MDirection::J2000);
			const int nSamples = idi->uvw().size();
			slope.resize(2, itsMaxFeeds);
			slope.set(0.0);
			casa::Vector<bool> done(itsMaxFeeds);
			done.set(false);

			/// @todo Deal with changing pointing
			casa::Vector<double> uvw(3);
			int nDone=0;
			for (int row=0; row<nSamples; row++)
			{
				double delay;
				int feed=idi->feed1()(row);
				CONRADCHECK(feed<itsMaxFeeds, "Too many feeds: increase maxfeeds");
				if (!done(feed))
				{
					casa::UVWMachine machine(out, idi->pointingDir1()(row), false, true);
					for (int i=0; i<2; i++)
					{
						uvw.set(0.0);
						uvw(i)=1.0;
						machine.convertUVW(delay, uvw);
						slope(i, feed)=delay;
					}
					done(feed)=true;
					nDone++;
					if (nDone==itsMaxFeeds)
						break;
				}
			}
			CONRADCHECK(nDone==itsMaxFeeds, "Failed to find pointing for all feeds");
		}

	}
}
