#include <gridding/AntennaIllumVisGridder.h>
#include <scimath/Mathematics/FFTServer.h>
#include <casa/BasicSL/Complex.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/ArrayMath.h>

#include <conrad/ConradError.h>

using namespace conrad;

namespace conrad
{
    namespace synthesis
    {

        AntennaIllumVisGridder::AntennaIllumVisGridder(const double diameter, const double blockage,
            const double wmax, const int nwplanes, const double cutoff,
            const int overSample, const int maxSupport) :
        WProjectVisGridder(wmax, nwplanes, cutoff, overSample, maxSupport),
            itsReferenceFrequency(0.0), itsDiameter(diameter), itsBlockage(blockage)

        {
            CONRADCHECK(diameter>0.0, "Blockage must be positive");
            CONRADCHECK(diameter>blockage, "Antenna diameter must be greater than blockage");
            CONRADCHECK(blockage>=0.0, "Blockage must be non-negative");
        }

        AntennaIllumVisGridder::~AntennaIllumVisGridder()
        {
            itsC.resize(0,0,0);
        }

/// Initialize the convolution function into the cube. If necessary this
/// could be optimized by using symmetries.
        void AntennaIllumVisGridder::initConvolutionFunction(IDataSharedIter& idi,
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

            int cenw=(itsNWPlanes-1)/2;
            for (int i=0;i<nSamples;i++)
            {
                double w=(uvw(i)(2))/(casa::C::c);
                for (int chan=0;chan<nChan;chan++)
                {
                    double freq=idi->frequency()[chan];
                    itsCMap(i,chan)=chan*itsNWPlanes+(cenw+nint(w*freq/itsWScale));
                }
            }
            if(itsSupport!=0) return;

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

            for (int chan=0;chan<nChan;chan++)
            {
/// Make the disk for this channel
                casa::Matrix<casa::Complex> disk(nx*itsOverSample,ny*itsOverSample);
                disk.set(0.0);
/// Calculate the size of one cell in meters
                double cell=cellSize(0)*(casa::C::c/idi->frequency()[chan])/double(itsOverSample);
                double rmax=std::pow(itsDiameter/cell,2);
                double rmin=std::pow(itsBlockage/cell,2);
                double sumDisk=0.0;
                for (int ix=0;ix<itsCSize;ix++)
                {
                    double nux2=std::pow(std::abs(double(ix-itsCCenter)), 2);
                    for (int iy=0;iy<itsCSize;iy++)
                    {
                        double nuy2=std::pow(std::abs(double(iy-itsCCenter)), 2);
                        double r=nux2+nuy2;
                        if((r>rmin)&&(r<rmax))
                        {
                            disk(ix,iy)=1.0;
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
                for (int iw=0;iw<=cenw;iw++)
                {

                    thisPlane.set(0.0);

                    float w=2.0f*casa::C::pi*float(iw-cenw)*itsWScale;
                    double freq=idi->frequency()[chan];
                    int zIndex=chan*itsNWPlanes+iw;
                    int zFlipIndex=chan*itsNWPlanes+(itsNWPlanes-1-iw);

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
//                            casa::Complex wt=disk(ix,iy)*casa::Complex(ccfx(iy-ceny+ny/2)*ccfy(ix-cenx+nx/2));
                            casa::Complex wt=casa::Complex(ccfx(iy-ceny+ny/2)*ccfy(ix-cenx+nx/2));
                            thisPlane(ix,iy)=wt*casa::Complex(cos(phase), sin(phase));
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
                        itsCSize=2*(itsSupport+1)*itsOverSample;
                        std::cout << "W support = " << itsSupport
                        << " pixels, convolution function size = " << itsCSize << " pixels"
                        << std::endl;
                        itsCCenter=itsCSize/2-1;
                        itsC.resize(itsCSize, itsCSize, itsNWPlanes*nChan);
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
                    itsC.xyPlane(zFlipIndex)=casa::conj(itsC.xyPlane(zIndex));
                }
            }
            std::cout << "Shape of convolution function = " << itsC.shape() << std::endl;
        }


        int AntennaIllumVisGridder::cOffset(int row, int chan)
        {
            return itsCMap(row, chan);
        }

    }
}
