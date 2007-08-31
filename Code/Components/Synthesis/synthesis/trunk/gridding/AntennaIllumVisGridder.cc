#include <gridding/AntennaIllumVisGridder.h>
#include <scimath/Mathematics/FFTServer.h>
#include <casa/BasicSL/Complex.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/ArrayMath.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/UVWMachine.h>

#include <conrad/ConradError.h>

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
			itsC.resize(0, 0, 0);
		}

		/// Initialize the convolution function into the cube. If necessary this
		/// could be optimized by using symmetries.
		/// @todo Make initConvolutionFunction more robust
		void AntennaIllumVisGridder::initConvolutionFunction(IDataSharedIter& idi,
		    const conrad::scimath::Axes& axes,
		    casa::Vector<casa::RigidVector<double, 3> >& uvw,
		    const casa::Vector<double>& cellSize, const casa::IPosition& shape)
		{
			/// We have to calculate the lookup function converting from
			/// row and channel to plane of the w-dependent convolution
			/// function
			const int nSamples = uvw.size();
			const int nChan = idi->frequency().size();
			itsCMap.resize(nSamples, nChan);
			itsCMap.set(0);
			/// @todo Select max feeds more carefully

			int cenw=(itsNWPlanes-1)/2;
			for (int i=0; i<nSamples; i++)
			{
				double w=(uvw(i)(2))/(casa::C::c);
				int feed=idi->feed1()(i);
				for (int chan=0; chan<nChan; chan++)
				{
					double freq=idi->frequency()[chan];
					itsCMap(i, chan)=feed+itsMaxFeeds*(chan*itsNWPlanes+(cenw+nint(w*freq/itsWScale)));
					CONRADCHECK(itsCMap(i, chan)<(itsNWPlanes*nChan*itsMaxFeeds),
							"W scaling error: recommend allowing larger range of w");
					CONRADCHECK(itsCMap(i, chan)>-1,
							"W scaling error: recommend allowing larger range of w");
				}
			}
			if (itsSupport!=0)
				return;

			/// Get the pointing direction
			casa::Matrix<double> slope;
			findCollimation(idi, axes, slope);
			int nFeeds=slope.nrow();

			casa::FFTServer<casa::Float,casa::Complex> ffts;
			itsSupport=0;

			/// These are the actual cell sizes used
			float cellx=1.0/(float(shape(0))*cellSize(0));
			float celly=1.0/(float(shape(1))*cellSize(1));

			/// Limit the size of the convolution function since
			/// we don't need it finely sampled in image space. This
			/// will reduce the time taken to calculate it.
			int nx=std::min(itsMaxSupport, shape(0));
			int ny=std::min(itsMaxSupport, shape(1));
			/// We want nx * ccellx = overSample * shape(0) * cellx

			// Find the actual cellsizes in x and y (radians) after over
			// oversampling (in uv space)
			float ccellx=float(itsOverSample)*float(shape(0))*cellx/float(nx);
			float ccelly=float(itsOverSample)*float(shape(1))*celly/float(ny);

			int qnx=nx/itsOverSample;
			int qny=ny/itsOverSample;

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

			for (int feed=0; feed<itsMaxFeeds; feed++)
			{
				for (int chan=0; chan<nChan; chan++)
				{
					/// Make the disk for this channel
					casa::Matrix<casa::Complex> disk(qnx, qny);
					disk.set(0.0);
					/// Calculate the size of one cell in meters
					double cell=cellSize(0)*(casa::C::c/idi->frequency()[chan])/double(itsOverSample);
					double rmax=std::pow(itsDiameter/(2.0*cell), 2);
					double rmin=std::pow(itsBlockage/(2.0*cell), 2);
					/// Slope is the delay per m 
					double ax=2.0f*casa::C::pi*cell*slope(0, feed)*idi->frequency()[chan]/casa::C::c;
					double ay=2.0f*casa::C::pi*cell*slope(1, feed)*idi->frequency()[chan]/casa::C::c;
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

					for (uint iy=0; iy<qny; iy++)
					{
						casa::Vector<casa::Complex> vec(disk.column(iy));
						ffts.fft(vec, true);
					}
					for (uint ix=0; ix<qnx; ix++)
					{
						casa::Vector<casa::Complex> vec(disk.row(ix));
						ffts.fft(vec, true);
					}
					disk=disk*conj(disk);
					float peak=casa::max(casa::real(disk));
					CONRADCHECK(peak>0.0, "Synthetic primary beam is empty");
					disk/=casa::Complex(peak);

					/// Calculate the total convolution function including
					/// the w term and the antenna convolution function
					casa::Matrix<casa::Complex> thisPlane(nx, ny);

					for (int iw=0; iw<itsNWPlanes; iw++)
					{
						thisPlane.set(0.0);

						float w=2.0f*casa::C::pi*float(iw-cenw)*itsWScale;
						double freq=idi->frequency()[chan];
						int zIndex=feed+itsMaxFeeds*(chan*itsNWPlanes+iw);

						// Loop over the central nx, ny region, setting it to the product
						// of the phase screen and the spheroidal function
						double maxCF=0.0;
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
								casa::Complex wt=disk(ix, iy)*casa::Complex(ccfx(iy)*ccfy(ix));
								thisPlane(ix-qnx/2+nx/2, iy-qny/2+ny/2)=wt*casa::Complex(
								    cos(phase), -sin(phase));
								maxCF+=casa::real(disk(ix, iy));
							}
						}
						maxCF/=(double(nx)*double(ny));
						//						std::cout << "Feed " << feed << " Freq " << freq << " W " << w << " Max CF = " << maxCF << std::endl;	  
						// At this point, we have the phase screen multiplied by the spheroidal
						// function, sampled on larger cellsize (itsOverSample larger) in image
						// space. Only the inner qnx, qny pixels have a non-zero value

						// Now we have to calculate the Fourier transform to get the
						// convolution function in uv space
						for (int iy=0; iy<ny; iy++)
						{
							casa::Vector<casa::Complex> vec(thisPlane.column(iy));
							ffts.fft(vec, true);
						}
						for (int ix=0; ix<nx; ix++)
						{
							casa::Vector<casa::Complex> vec(thisPlane.row(ix));
							ffts.fft(vec, true);
						}
						// If the support is not yet set, find it and size the
						// convolution function appropriately
						if (itsSupport==0)
						{
							// Find the support by starting from the edge and
							// working in
							for (int ix=0; ix<nx/2; ix++)
							{
								if (casa::abs(thisPlane(ix, ny/2))>itsCutoff*maxCF)
								{
									itsSupport=abs(ix-nx/2)/itsOverSample;
									break;
								}
							}
							itsSupport=(itsSupport<nx/2) ? itsSupport : nx/2;
							itsCSize=2*(itsSupport+1)*itsOverSample;
							std::cout << "Convolution function support = "<< itsSupport
							    << " pixels, convolution function size = "<< itsCSize
							    << " pixels"<< std::endl;
							std::cout << "Maximum extent = "<< itsCSize*cell
							    << " (m) sampled at "<< cell << " (m)"<< std::endl;
							itsCCenter=itsCSize/2-1;
							itsC.resize(itsCSize, itsCSize, itsMaxFeeds*nChan*itsNWPlanes);
							itsC.set(0.0);
						}
						// Now cut out the inner part of the convolution function and
						// insert it into the convolution function
						for (int iy=-itsOverSample*itsSupport; iy<+itsOverSample*itsSupport; iy++)
						{
							for (int ix=-itsOverSample*itsSupport; ix<+itsOverSample
							    *itsSupport; ix++)
							{
								itsC(ix+itsCCenter, iy+itsCCenter, zIndex)=thisPlane(ix+nx/2,
								    iy+ny/2);
							}
						}
					}
				}
			}
			std::cout << "Shape of convolution function = "<< itsC.shape()
			    << std::endl;
			if (itsName!="")
				save(itsName);
		}

		/// To finalize the transform of the weights, we use the following steps:
		/// 1. For each plane of the convolution function, transform to image plane
		/// and multiply by conjugate to get abs value squared.
		/// 2. Sum all planes weighted by the weight for that convolution function.
		void AntennaIllumVisGridder::finaliseReverseWeights(
		    casa::Matrix<double>& sumWeights, const conrad::scimath::Axes& axes,
		    casa::Cube<double>& out)
		{
			casa::FFTServer<float,casa::Complex> ffts;

			int nx=out.shape()(0);
			int ny=out.shape()(1);
			//			int nPol=out.shape()(2);
			int nPol=1;

			/// We must pad the convolution function to full size, reverse transform
			/// square, and sum multiplied by the corresponding weight
			int cnx=std::min(itsMaxSupport, nx);
			int cny=std::min(itsMaxSupport, ny);
			int ccenx=cnx/2;
			int cceny=cny/2;

			/// This is the output cube before sinc padding
			casa::Cube<double> cOut(cnx, cny, nPol);

			/// Work space
			casa::Matrix<casa::Complex> thisPlane(cnx, cny);

			int nZ=sumWeights.shape()(0);

			CONRADCHECK(sumWeights.shape()(1)!=nPol,
					"Number of polarizations do not match");

			out.set(0.0);
			cOut.set(0.0);

			for (int iz=0; iz<nZ; iz++)
			{
				if (max(sumWeights.row(iz))>0)
				{
					thisPlane.set(0.0);

					// Now fill the inner part of the uv plane with the convolution function
					// and transform to obtain the full size image
					for (int iy=-itsSupport*itsOverSample; iy<+itsOverSample*itsSupport; iy++)
					{
						for (int ix=-itsOverSample*itsSupport; ix<+itsOverSample*itsSupport; ix++)
						{
							thisPlane(ix+ccenx, iy+cceny)=itsC(ix+itsCCenter, iy+itsCCenter,
							    iz);
						}
					}

					// Now we have to calculate the Fourier transform to get the
					// convolution function in image space
					for (int iy=0; iy<cny; iy++)
					{
						casa::Vector<casa::Complex> vec(thisPlane.column(iy));
						ffts.fft(vec, false);
					}
					for (int ix=0; ix<cnx; ix++)
					{
						casa::Vector<casa::Complex> vec(thisPlane.row(ix));
						ffts.fft(vec, false);
					}
					double peak(casa::abs(casa::max(casa::real(thisPlane))));
					if (peak>0.0)
					{
						for (int pol=0; pol<nPol; pol++)
						{
							double weight=sumWeights(iz, pol)/peak;
							for (int ix=0; ix<cnx; ix++)
							{
								for (int iy=0; iy<cny; iy++)
								{
									cOut(ix, iy, pol)+=weight*real(thisPlane(ix, iy)
									    *conj(thisPlane(ix, iy)));
								}
							}
						}
					}
				}
			}
			fftPad(cOut, out);
		}

		void AntennaIllumVisGridder::fftPad(const casa::Cube<double>& in,
		    casa::Cube<double>& out)
		{

			casa::FFTServer<double,casa::DComplex> ffts;

			int inx=in.shape()(0);
			int iny=in.shape()(1);
			int inz=in.shape()(2);

			int onx=out.shape()(0);
			int ony=out.shape()(1);
			int onz=out.shape()(2);

			CONRADCHECK(onx>=inx, "Attempting to pad to smaller array");
			CONRADCHECK(ony>=iny, "Attempting to pad to smaller array");
			CONRADCHECK(inz==onz, "Number of z planes different in padding");

			for (int iz=0; iz<inz; iz++)
			{
				casa::Matrix<casa::DComplex> inPlane(inx, iny);
				casa::Matrix<casa::DComplex> outPlane(onx, ony);
				casa::convertArray(inPlane, in.xyPlane(iz));
				outPlane.set(0.0);
				for (int iy=0; iy<iny; iy++)
				{
					casa::Vector<casa::DComplex> vec(inPlane.column(iy));
					ffts.fft(vec, false);
				}
				for (int ix=0; ix<inx; ix++)
				{
					casa::Vector<casa::DComplex> vec(inPlane.row(ix));
					ffts.fft(vec, false);
				}
				for (int iy=0; iy<iny; iy++)
				{
					for (int ix=0; ix<inx; ix++)
					{
						outPlane(ix+(onx-inx)/2, iy+(ony-iny)/2) = inPlane(ix, iy);
					}
				}
				for (int iy=0; iy<ony; iy++)
				{
					casa::Vector<casa::DComplex> vec(outPlane.column(iy));
					ffts.fft(vec, true);
				}
				for (int ix=0; ix<onx; ix++)
				{
					casa::Vector<casa::DComplex> vec(outPlane.row(ix));
					ffts.fft(vec, true);
				}
				const casa::Array<casa::DComplex> constOutPlane(outPlane);
				casa::Array<double> outArray(out.xyPlane(iz));

				casa::real(outArray, constOutPlane);
				/// Rescale to compensate for increase in sampling
				double scale=(double(inx)*double(iny))/(double(onx)*double(ony));

				outArray*=scale;

			}
		}

		int AntennaIllumVisGridder::cOffset(int row, int chan)
		{
			return itsCMap(row, chan);
		}

		void AntennaIllumVisGridder::findCollimation(IDataSharedIter& idi,
		    const conrad::scimath::Axes& axes, casa::Matrix<double>& slope)
		{
			casa::Quantum<double>refLon((axes.start("RA")+axes.end("RA"))/2.0, "rad");
			casa::Quantum<double> refLat((axes.start("DEC")+axes.end("DEC"))/2.0,
			    "rad");
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
		}

	}
}
