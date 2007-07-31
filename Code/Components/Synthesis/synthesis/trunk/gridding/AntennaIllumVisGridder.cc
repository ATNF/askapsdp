#include <gridding/AntennaIllumVisGridder.h>
#include <scimath/Mathematics/FFTServer.h>
#include <casa/BasicSL/Complex.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/ArrayMath.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/UVWMachine.h>

#include <conrad/ConradError.h>

using namespace conrad;

namespace conrad
{
    namespace synthesis
    {

        AntennaIllumVisGridder::AntennaIllumVisGridder(const double diameter, const double blockage,
            const double wmax, const int nwplanes, const double cutoff,
            const int overSample, const int maxSupport, const int maxFeeds) :
        WProjectVisGridder(wmax, nwplanes, cutoff, overSample, maxSupport),
            itsReferenceFrequency(0.0), itsDiameter(diameter), itsBlockage(blockage),
            itsMaxFeeds(maxFeeds)

        {
            CONRADCHECK(diameter>0.0, "Blockage must be positive");
            CONRADCHECK(diameter>blockage, "Antenna diameter must be greater than blockage");
            CONRADCHECK(blockage>=0.0, "Blockage must be non-negative");
            CONRADCHECK(maxFeeds>0, "Maximum number of feeds must be one or more");
        }

        AntennaIllumVisGridder::~AntennaIllumVisGridder()
        {
            itsC.resize(0,0,0);
        }

/// Initialize the convolution function into the cube. If necessary this
/// could be optimized by using symmetries.
        void AntennaIllumVisGridder::initConvolutionFunction(IDataSharedIter& idi,
        		const conrad::scimath::Axes& axes,
        		casa::Vector<casa::RigidVector<double, 3> >& uvw,
        		const casa::Vector<double>& cellSize,
            const casa::IPosition& shape)
        {
/// We have to calculate the lookup function converting from
/// row and channel to plane of the w-dependent convolution
/// function
            const int nSamples = uvw.size();
            const int nChan = idi->frequency().size();
            itsCMap.resize(nSamples, nChan);
            itsCMap.set(0);
            /// @todo Select max feeds more carefully
            itsMaxFeeds=casa::max(idi->feed1())+1;

            int cenw=(itsNWPlanes-1)/2;
            for (int i=0;i<nSamples;i++)
            {
                double w=(uvw(i)(2))/(casa::C::c);
                int feed=idi->feed1()(i);
                for (int chan=0;chan<nChan;chan++)
                {
                    double freq=idi->frequency()[chan];
                    itsCMap(i,chan)=feed+itsMaxFeeds*(chan*itsNWPlanes+(cenw+nint(w*freq/itsWScale)));
                }
            }
            if(itsSupport!=0) return;

            /// Get the pointing direction - for the moment just use the first one
            casa::Matrix<double> slope;
            findCollimation(idi, axes, slope);
            int nFeeds=slope.nrow();
            
            casa::FFTServer<casa::Float,casa::Complex> ffts;
            itsSupport=0;

/// Limit the size of the convolution function since
/// we don't need it finely sampled in image space. This
/// will reduce the time taken to calculate it.
            int nx=std::min(itsMaxSupport, shape(0)/itsOverSample);
            int ny=std::min(itsMaxSupport, shape(1)/itsOverSample);
            int cenx=nx/2;
            int ceny=ny/2;

            casa::Vector<float> ccfx(nx);
            casa::Vector<float> ccfy(ny);
            for (int ix=0;ix<nx;ix++)
            {
                float nux=std::abs(float(ix-cenx))/float(nx/2);
                ccfx(ix)=grdsf(nux)/float(nx);
            }
            for (int iy=0;iy<ny;iy++)
            {
                float nuy=std::abs(float(iy-ceny))/float(ny/2);
                ccfy(iy)=grdsf(nuy)/float(ny);
            }

// Now we step through the w planes, starting the furthest
// out. We calculate the support for that plane and use it
// for all the others.
// We pad here to do sinc interpolation of the convolution
// function in uv space
            casa::Matrix<casa::Complex> thisPlane(nx*itsOverSample, ny*itsOverSample);

            float cellx=1.0/(float(nx)*cellSize(0));
            float celly=1.0/(float(ny)*cellSize(1));

			for (int feed=0;feed<itsMaxFeeds;feed++) {
            for (int chan=0;chan<nChan;chan++)
            {
/// Make the disk for this channel
                casa::Matrix<casa::Complex> disk(nx*itsOverSample,ny*itsOverSample);
                disk.set(0.0);
/// Calculate the size of one cell in meters
                double cell=cellSize(0)*(casa::C::c/idi->frequency()[chan])/double(itsOverSample);
                double rmax=std::pow(itsDiameter/cell,2);
                double rmin=std::pow(itsBlockage/cell,2);
                /// Slope is the delay per m 
                double ax=2.0f*casa::C::pi*cell*slope(0,feed)*idi->frequency()[chan]/casa::C::c;
                double ay=2.0f*casa::C::pi*cell*slope(1,feed)*idi->frequency()[chan]/casa::C::c;
                double sumDisk=0.0;
                /// Calculate the antenna voltage pattern, including the
                /// phase shift due to pointing
                for (int ix=0;ix<nx;ix++)
                {
                	double nux=double(ix-cenx);
                    double nux2=std::pow(std::abs(nux), 2);
                    for (int iy=0;iy<ny;iy++)
                    {
                    	double nuy=double(iy-ceny);
                        double nuy2=std::pow(std::abs(nuy), 2);
                        double r=nux2+nuy2;
                        if((r>rmin)&&(r<rmax))
                        {
                        	double phase=ax*nux+ay*nuy;
                            disk(ix,iy)=casa::Complex(cos(phase), -sin(phase));
                            sumDisk+=1.0;
                        }
                    }
                }
                for (uint iy=0;iy<ny;iy++)
                {
                    casa::Array<casa::Complex> vec(disk.column(iy));
                    ffts.fft(vec, true);
                }
                for (uint ix=0;ix<nx;ix++)
                {
                    casa::Array<casa::Complex> vec(disk.row(ix));
                    ffts.fft(vec, true);
                }
                disk=disk*conj(disk);
                /// Calculate the total convolution function including
                /// the w term and the antenna convolution function
                for (int iw=0;iw<itsNWPlanes;iw++)
                {

                    thisPlane.set(0.0);

                    float w=2.0f*casa::C::pi*float(iw-cenw)*itsWScale;
                    double freq=idi->frequency()[chan];
                    int zIndex=feed+itsMaxFeeds*(chan*itsNWPlanes+iw);
                    int zFlipIndex=feed+itsMaxFeeds*(chan*itsNWPlanes+(itsNWPlanes-1-iw));

                    for(int iy=ceny-ny/2;iy<ceny+ny/2;iy++)
                    {
                        float y2=float(iy-ceny)*celly;
                        y2*=y2;
                        for(int ix=cenx-nx/2;ix<cenx+nx/2;ix++)
                        {
                            float x2=float(ix-cenx)*cellx;
                            x2*=x2;
                            float r2=x2+y2;
                            float phase=w*(1.0-sqrt(1.0-r2));
                            casa::Complex wt=disk(ix,iy)*casa::Complex(ccfx(iy-ceny+ny/2)*ccfy(ix-cenx+nx/2));
                            thisPlane(ix,iy)=wt*casa::Complex(cos(phase), -sin(phase));
                        }
                    }
// Now we have to calculate the Fourier transform to get the
// convolution function in uv space
// @todo Replace ffts by smarter versions - real/complex and FFTW
                    for (int iy=0;iy<itsOverSample*ny;iy++)
                    {
                        casa::Array<casa::Complex> vec(thisPlane.column(iy));
                        ffts.fft(vec, true);
                    }
                    for (int ix=0;ix<itsOverSample*nx;ix++)
                    {
                        casa::Array<casa::Complex> vec(thisPlane.row(ix));
                        ffts.fft(vec, true);
                    }
// If the support is not yet set, find it and size the
// convolution function appropriately
                    if(itsSupport==0)
                    {
// Find the support by starting from the edge and
// working in
                        for(int ix=0;ix<itsOverSample*cenx;ix++)
                        {
                            if(abs(thisPlane(ix,itsOverSample*ceny))>itsCutoff)
                            {
                                itsSupport=abs(ix-itsOverSample*cenx)/itsOverSample;
                                break;
                            }
                        }
                        itsSupport=(itsSupport<nx/2)?itsSupport:nx/2;
                        CONRADCHECK(itsSupport>0, "Derived support is zero");
                        itsCSize=2*(itsSupport+1)*itsOverSample;
                        std::cout << "W support = " << itsSupport
                        << " pixels, convolution function size = " << itsCSize << " pixels"
                        << std::endl;
                        itsCCenter=itsCSize/2-1;
                        itsC.resize(itsCSize, itsCSize, itsNWPlanes*nChan*itsMaxFeeds);
                        itsC.set(0.0);
                    }
// Now cut out the inner part of the convolution function
                    for (int iy=-itsSupport*itsOverSample;iy<+itsOverSample*itsSupport;iy++)
                    {
                        for (int ix=-itsOverSample*itsSupport;ix<+itsOverSample*itsSupport;ix++)
                        {
                            itsC(ix+itsCCenter,iy+itsCCenter,zIndex)=
                                thisPlane(ix+itsOverSample*cenx,iy+itsOverSample*ceny);
                        }
                    }
                }
            }
			}
            std::cout << "Shape of convolution function = " << itsC.shape() << std::endl;
        }


        int AntennaIllumVisGridder::cOffset(int row, int chan)
        {
            return itsCMap(row, chan);
        }

        void AntennaIllumVisGridder::findCollimation(IDataSharedIter& idi, const conrad::scimath::Axes& axes,
    		casa::Matrix<double>& slope) 
        {
            casa::Quantum<double> refLon((axes.start("RA")+axes.end("RA"))/2.0, "rad");
            casa::Quantum<double> refLat((axes.start("DEC")+axes.end("DEC"))/2.0, "rad");
    		casa::MDirection out(refLon, refLat, casa::MDirection::J2000);
            const int nSamples = idi->uvw().size();
            slope.resize(2, itsMaxFeeds);
            casa::Vector<bool> done(itsMaxFeeds);
            done.set(false);

            /// @todo Deal with changing pointing
        	casa::UVWMachine machine(out, idi->pointingDir1()(0), false, true);
            casa::Vector<double> uvw(3);
            int nDone=0;
            for (int row=0;row<nSamples;row++) {
                double delay;
                int feed=idi->feed1()(row);
                if(!done(feed)) {
					for (int i=0;i<2;i++) {
		                uvw.set(0.0);
        	        	uvw(i)=1.0;
	        	        machine.convertUVW(delay, uvw);
	            	    slope(i,feed)=delay;
					}
					done(feed)=true;
					nDone++;
					if (nDone==itsMaxFeeds) break;
				}
            }
        }

    }
}
