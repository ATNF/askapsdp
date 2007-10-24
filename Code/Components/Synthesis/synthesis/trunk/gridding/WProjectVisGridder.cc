#include <gridding/WProjectVisGridder.h>

#include <conrad/ConradError.h>
#include <conrad/ConradUtil.h>

#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>

#include <casa/BasicSL/Constants.h>
#include <fft/FFTWrapper.h>

using namespace conrad;

#include <cmath>

namespace conrad
{
	namespace synthesis
	{

		WProjectVisGridder::WProjectVisGridder(const double wmax,
		    const int nwplanes, const double cutoff, const int overSample,
		    const int maxSupport, const std::string& name)
		{
			CONRADCHECK(wmax>0.0, "Baseline length must be greater than zero");
			CONRADCHECK(nwplanes>0, "Number of w planes must be greater than zero");
			CONRADCHECK(nwplanes%2==1, "Number of w planes must be odd");
			CONRADCHECK(overSample>0, "Oversampling must be greater than 0");
			CONRADCHECK(cutoff>0.0, "Cutoff must be positive");
			CONRADCHECK(cutoff<1.0, "Cutoff must be less than 1.0");
			CONRADCHECK(maxSupport>0, "Maximum support must be greater than 0")
			itsSupport=0;
			itsNWPlanes=nwplanes;
			itsWScale=wmax/double((nwplanes-1)/2);
			itsOverSample=overSample;
			itsCutoff=cutoff;
			itsMaxSupport=maxSupport;
			itsName=name;
		}

		WProjectVisGridder::~WProjectVisGridder()
		{
		}

		/// Clone a copy of this Gridder
		IVisGridder::ShPtr WProjectVisGridder::clone()
		{
			return IVisGridder::ShPtr(new WProjectVisGridder(*this));
		}

		/// Initialize the convolution function into the cube. If necessary this
		/// could be optimized by using symmetries.
		void WProjectVisGridder::initIndices(IDataSharedIter& idi)
		{
			/// We have to calculate the lookup function converting from
			/// row and channel to plane of the w-dependent convolution
			/// function
			const int nSamples = idi->uvw().size();
			const int nChan = idi->frequency().size();
			const int nPol = idi->visibility().shape()(2);

			itsCMap.resize(nSamples, nPol, nChan);
			int cenw=(itsNWPlanes-1)/2;
			for (int i=0; i<nSamples; i++)
			{
				double w=(idi->uvw()(i)(2))/(casa::C::c);
				for (int chan=0; chan<nChan; chan++)
				{
					for (int pol=0; pol<nPol; pol++)
					{
						double freq=idi->frequency()[chan];
						/// Calculate the index into the convolution functions
						if(itsNWPlanes>1) {
						  itsCMap(i, pol, chan)=cenw+nint(w*freq/itsWScale);
						}
						else {
						  itsCMap(i, pol, chan)=0;
						}
						if (itsCMap(i, pol, chan)<0)
						{
							std::cout << w << " "<< freq << " "<< itsWScale << " "<< itsCMap(
							    i, pol, chan) << std::endl;
						}
						CONRADCHECK(itsCMap(i,pol,chan)<itsNWPlanes, "W scaling error: recommend allowing larger range of w");
						CONRADCHECK(itsCMap(i,pol,chan)>-1, "W scaling error: recommend allowing larger range of w");
					}
				}
			}
		}

		/// Initialize the convolution function into the cube. If necessary this
		/// could be optimized by using symmetries.
		void WProjectVisGridder::initConvolutionFunction(IDataSharedIter& idi)
		{
			/// We have to calculate the lookup function converting from
			/// row and channel to plane of the w-dependent convolution
			/// function
			const int nSamples = idi->uvw().size();
			const int nChan = idi->frequency().size();
			int cenw=(itsNWPlanes-1)/2;

			if (itsSupport!=0)
			{
				return;
			}

			itsSupport=0;

			/// These are the actual cell sizes used
			float cellx=1.0/(float(itsShape(0))*itsUVCellSize(0));
			float celly=1.0/(float(itsShape(1))*itsUVCellSize(1));

			/// Limit the size of the convolution function since
			/// we don't need it finely sampled in image space. This
			/// will reduce the time taken to calculate it.
			int nx=std::min(itsMaxSupport, itsShape(0));
			int ny=std::min(itsMaxSupport, itsShape(1));
			/// We want nx * ccellx = overSample * itsShape(0) * cellx

			int qnx=nx/itsOverSample;
			int qny=ny/itsOverSample;

			// Find the actual cellsizes in x and y (radians) after over
			// oversampling (in uv space)
			float ccellx=float(itsShape(0))*cellx/float(qnx);
			float ccelly=float(itsShape(1))*celly/float(qny);

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

			// Now we step through the w planes, starting the furthest
			// out. We calculate the support for that plane and use it
			// for all the others.

			// We pad here to do sinc interpolation of the convolution
			// function in uv space
			casa::Matrix<casa::Complex> thisPlane(nx, ny);

			for (int iw=0; iw<=cenw; iw++)
			{
				thisPlane.set(0.0);

				float w=2.0f*casa::C::pi*float(iw-cenw)*itsWScale;
				// Loop over the central nx, ny region, setting it to the product
				// of the phase screen and the spheroidal function
				for (int iy=0; iy<qny; iy++)
				{
					float y2=float(iy-qny/2)*ccelly;
					y2*=y2;
					for (int ix=0; ix<qnx; ix++)
					{
						float x2=float(ix-qnx/2)*ccellx;
						x2*=x2;
						float r2=x2+y2;
						float phase=w*(1.0-sqrt(1.0-r2));
						float wt=ccfx(iy)*ccfy(ix);
						thisPlane(ix-qnx/2+nx/2, iy-qny/2+ny/2)=casa::Complex(
						    wt*cos(phase), -wt*sin(phase));
					}
				}
				// At this point, we have the phase screen multiplied by the spheroidal
				// function, sampled on larger cellsize (itsOverSample larger) in image
				// space. Only the inner qnx, qny pixels have a non-zero value

				// Now we have to calculate the Fourier transform to get the
				// convolution function in uv space
				fft2d(thisPlane, true);

				// Now thisPlane is filled with convolution function
				// sampled on a finer grid in u,v
				//
				// If the support is not yet set, find it and size the
				// convolution function appropriately
				float maxPlane=abs(thisPlane(nx/2, ny/2));
				if (itsSupport==0)
				{
					// Find the support by starting from the edge and
					// working in
					for (int ix=0; ix<nx/2; ix++)
					{
						/// Check on horizontal axis
						if ((casa::abs(thisPlane(ix, ny/2))>itsCutoff))
						{
							itsSupport=abs(ix-nx/2)/itsOverSample;
							break;
						}
						///  Check on diagonal
						if ((casa::abs(thisPlane(ix, ix))>itsCutoff))
						{
							itsSupport=abs(int(1.414*float(ix))-nx/2)/itsOverSample;
							break;
						}
						if (nx==ny)
						{
							/// Check on vertical axis
							if ((casa::abs(thisPlane(nx/2, ix))>itsCutoff))
							{
								itsSupport=abs(ix-ny/2)/itsOverSample;
								break;
							}
						}
					}
					CONRADCHECK(itsSupport>0, "Unable to determine support of convolution function");
					CONRADCHECK(itsSupport*itsOverSample<nx/2,
							"Overflowing convolution function - increase maxSupport or decrease overSample")
					itsCSize=2*(itsSupport+1)*itsOverSample;
					std::cout << "Convolution function support = "<< itsSupport
					    << " pixels, convolution function size = "<< itsCSize<< " pixels"
					    << std::endl;
					itsCCenter=itsCSize/2-1;
					itsConvFunc.resize(itsNWPlanes);
					itsSumWeights.resize(itsNWPlanes, itsShape(2), itsShape(3));
					itsSumWeights.set(casa::Complex(0.0));
				}
				itsConvFunc[iw].resize(itsCSize, itsCSize);
				itsConvFunc[iw].set(0.0);
				// Now cut out the inner part of the convolution function and
				// insert it into the convolution function
				for (int iy=-itsOverSample*itsSupport; iy<+itsOverSample*itsSupport; iy++)
				{
					for (int ix=-itsOverSample*itsSupport; ix<+itsOverSample*itsSupport; ix++)
					{
						itsConvFunc[iw](ix+itsCCenter, iy+itsCCenter)=thisPlane(ix+nx/2, iy
						    +ny/2);
					}
				}
				itsConvFunc[itsNWPlanes-1-iw]=casa::conj(itsConvFunc[iw]);
			}
			std::cout << "Shape of convolution function = "<< itsConvFunc[0].shape()
			    << " by "<< itsConvFunc.size() << " planes"<< std::endl;
			if (itsName!="")
				save(itsName);
		}

		int WProjectVisGridder::cIndex(int row, int pol, int chan)
		{
			return itsCMap(row, pol, chan);
		}

		/// To finalize the transform of the weights, we use the following steps:
		/// 1. For each plane of the convolution function, transform to image plane
		/// and multiply by conjugate to get abs value squared.
		/// 2. Sum all planes weighted by the weight for that convolution function.
		void WProjectVisGridder::finaliseWeights(casa::Array<double>& out)
		{

			std::cout << "Calculating sum of weights image" << std::endl;

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
				// so the total field of view is itsOverSample times larger than the
				// original field of view.
				for (int iy=-itsSupport; iy<+itsSupport; iy++)
				{
					for (int ix=-itsSupport; ix<+itsSupport; ix++)
					{
						thisPlane(ix+ccenx, iy+cceny)=itsConvFunc[iz](ix*itsOverSample
						    +itsCCenter, iy*itsOverSample+itsCCenter);
					}
				}

				thisPlane*=casa::Complex(double(cnx)*double(cny));
				/// The peak here should be unity
				fft2d(thisPlane, false);

				// Now we need to cut out only the part inside the field of view
				for (int chan=0; chan<nChan; chan++)
				{
					for (int pol=0; pol<nPol; pol++)
					{
						casa::IPosition ip(4, 0, 0, pol, chan);
						casa::Complex wt=itsSumWeights(iz, pol, chan);
						for (int ix=0; ix<cnx; ix++)
						{
							ip(0)=ix;
							for (int iy=0; iy<cny; iy++)
							{
								ip(1)=iy;
								cOut(ip)+=casa::real(wt*thisPlane(ix, iy)*conj(thisPlane(ix, iy)));
							}
						}
					}
				}
			}
			fftPad(cOut, out);
			// Need to correct twice!
			correctConvolution(out);
			correctConvolution(out);
		}

		void WProjectVisGridder::fftPad(const casa::Array<double>& in,
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

	}
}
