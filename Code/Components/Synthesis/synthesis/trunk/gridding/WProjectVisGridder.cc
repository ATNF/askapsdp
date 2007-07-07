#include <gridding/WProjectVisGridder.h>

#include <conrad/ConradError.h>

#include <casa/BasicSL/Constants.h>
#include <scimath/Mathematics/FFTServer.h>

using namespace conrad;

namespace conrad
{
  namespace synthesis
  {

    WProjectVisGridder::WProjectVisGridder(const double wmax, const int nwplanes)
    {
      CONRADCHECK(wmax>0.0, "Baseline length must be greater than zero");
      CONRADCHECK(nwplanes>0, "Number of w planes must be greater than zero");
      itsSupport=0;
      itsNWPlanes=nwplanes;
      itsWScale=wmax/double(itsNWPlanes);
    }

    WProjectVisGridder::~WProjectVisGridder()
    {
    }

    /// Initialize the convolution function into the cube. If necessary this
    /// could be optimized by using symmetries.
    void WProjectVisGridder::initConvolutionFunction(IDataSharedIter& idi, 
      const casa::Vector<double>& cellSize,
      const casa::IPosition& shape)
    {
      /// We have to calculate the lookup function converting from
      /// row and channel to plane of the w-dependent convolution
      /// function
      const int nSamples = idi->uvw().size();
      const int nChan = idi->frequency().size();
      itsCMap.resize(nSamples, nChan);
      itsCMap.set(0);
      
      for (int i=0;i<nSamples;i++)
      {
        double w=(idi->uvw()(i)(2))/(casa::C::c);
        for (int chan=0;chan<nChan;chan++)
        {
          double freq=idi->frequency()[chan];
          itsCMap(i,chan)=nint(abs(w*freq/itsWScale));
        }
      }
      if(itsSupport!=0) return;
      
      casa::FFTServer<casa::Float,casa::Complex> ffts;
      itsOverSample=1;
      itsSupport=0;

      int nx=shape(0)*itsOverSample;
      int ny=shape(1)*itsOverSample;

      casa::Vector<double> ccfx(nx);
      casa::Vector<double> ccfy(ny);
      for (int ix=0;ix<nx;ix++)
      {
        double nux=std::abs(double(ix-nx/2))/double(nx/2);
        ccfx(ix)=grdsf(nux);
      }
      for (int iy=0;iy<ny;iy++)
      {
        double nuy=std::abs(double(iy-ny/2))/double(ny/2);
        ccfy(iy)=grdsf(nuy);
      }
      
      // Now we step through the w planes, starting the furthest
      // out. We calculate the support for that plane and use it
      // for all the others.
      casa::Matrix<casa::Complex> thisPlane(nx, ny);
      
      for (int iw=itsNWPlanes-1;iw>-1;iw--) {
        double w=double(iw)*itsWScale;
        for(int ix=0;ix<nx;ix++)
        {
          double x2=double(ix-nx/2)*itsOverSample/(double(nx)*cellSize(0));
          x2*=x2;
          for(int iy=0;iy<ny;iy++)
          {
            double y2=double(iy-ny/2)*itsOverSample/(double(ny)*cellSize(1));
            y2*=y2;
            double r2=x2+y2;
            double phase=casa::C::pi*w*r2;
            double wt=ccfx(ix)*ccfy(iy);
            thisPlane(ix,iy)=casa::Complex(wt*cos(phase), 0.0);
          }
        }
        for (int iy=0;iy<ny;iy++)
        {
          casa::Array<casa::Complex> vec(thisPlane.column(iy));
          ffts.fft(vec, true);
        }
        for (int ix=0;ix<nx;ix++)
        {
          casa::Array<casa::Complex> vec(thisPlane.row(ix));
          ffts.fft(vec, true);
        }
        if(itsSupport==0) {
          float cmax=float(nx)*float(ny);
          {
            for (int ix=nx-1;ix>=nx/2;ix--)
            {
              if(abs(real(thisPlane(ix,ny/2)))>cmax*1e-4) {
                itsSupport=abs(ix-nx/2);
                break;
              }
            }
          }
          std::cout << "W support " << itsSupport << std::endl;
          itsCSize=2*(itsSupport+1)*itsOverSample;    // 1024;
          itsCCenter=itsCSize/2-1;                    // 511
          itsC.resize(itsCSize, itsCSize, itsNWPlanes);
          itsC.set(0.0);
          // Now cut out the inner part
          itsSupport=(itsSupport<nx/2)?itsSupport:nx/2;
        }
        for (int iy=-itsSupport;iy<+itsSupport;iy++)
        {
          for (int ix=-itsSupport;ix<+itsSupport;ix++)
          {
              itsC(ix+itsCCenter,iy+itsCCenter,iw)=real(thisPlane(ix+nx/2,iy+ny/2));
          }
        }
      }
      itsC=itsC/(float(nx)*float(ny));
    }
    
    int WProjectVisGridder::cOffset(int row, int chan)
    {
      return itsCMap(row, chan);
    }

  }
}
