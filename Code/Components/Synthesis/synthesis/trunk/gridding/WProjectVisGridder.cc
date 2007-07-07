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
    
    /// The Convolution function correction in image space is the
    /// the spheroidal function itself.
    void WProjectVisGridder::correctConvolution(const scimath::Axes& axes,
      casa::Cube<double>& grid)
    {
      int nx=grid.shape()(0);
      int ny=grid.shape()(1);
      int nz=grid.shape()(2);
      casa::Vector<double> ccfx(nx);
      casa::Vector<double> ccfy(ny);
      for (int ix=0;ix<nx;ix++)
      {
        double nux=std::abs(double(ix-nx/2))/double(nx/2);
        ccfx(ix)=1.0/grdsf(nux);
      }
      for (int iy=0;iy<ny;iy++)
      {
        double nuy=std::abs(double(iy-ny/2))/double(ny/2);
        ccfy(iy)=1.0/grdsf(nuy);
      }
      for(int ix=0;ix<nx;ix++)
      {
        for(int iy=0;iy<ny;iy++)
        {
          for (int iz=0;iz<nz;iz++)
          {
            grid(ix,iy,iz)*=ccfx(ix)*ccfy(iy);
          }
        }
      }
    }

    /// The Convolution function correction in image space is the
    /// the spheroidal function itself.
    void WProjectVisGridder::applyConvolution(const scimath::Axes& axes,
      casa::Cube<double>& grid)
    {
      int nx=grid.shape()(0);
      int ny=grid.shape()(1);
      int nz=grid.shape()(2);
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
      /// @todo Optimize grid correction
      for(int ix=0;ix<nx;ix++)
      {
        for(int iy=0;iy<ny;iy++)
        {
          for (int iz=0;iz<nz;iz++)
          {
            grid(ix,iy,iz)*=ccfx(ix)*ccfy(iy);
          }
        }
      }
    }

// find spheroidal function with m = 6, alpha = 1 using the rational
// approximations discussed by fred schwab in 'indirect imaging'.
// this routine was checked against fred's sphfn routine, and agreed
// to about the 7th significant digit.
// the gridding function is (1-nu**2)*grdsf(nu) where nu is the distance
// to the edge. the grid correction function is just 1/grdsf(nu) where nu
// is now the distance to the edge of the image.
    double WProjectVisGridder::grdsf (double nu)
    {

      double top, bot, delnusq, nuend;
      int k, part;
      int np, nq;
      np=4;
      nq=2;
      double p[2][5]    =
      {
        8.203343e-2, -3.644705e-1, 6.278660e-1,
        -5.335581e-1, 2.312756e-1,
        4.028559e-3, -3.697768e-2, 1.021332e-1,
        -1.201436e-1, 6.412774e-2
      };
      double q[2][3]=
      {
        1.0000000, 8.212018e-1, 2.078043e-1,
        1.0000000, 9.599102e-1, 2.918724e-1
      };
      double value = 0.0;

      if ((nu>=0.0)&&(nu<0.75))
      {
        part = 0;
        nuend = 0.75;
      }
      else if ((nu>=0.75)&&(nu<=1.00))
      {
        part = 1;
        nuend = 1.00;
      }
      else
      {
        value = 0.0;
        return value;
      }

      top = p[part][0];
      bot = q[part][0];
      delnusq = std::pow(nu, 2) - std::pow(nuend, 2);
      for (k = 1;k<= np;k++)
      {
        double factor=std::pow(delnusq, k);
        top += p[part][k] * factor;
      }
      for (k = 1;k<= nq;k++)
      {
        double factor=std::pow(delnusq, k);
        bot += q[part][k] * factor;
      }
      if (bot!=0.0)
      {
        value = top/bot;
      }
      else
      {
        value = 0.0;
      }
      if(value<0.0) value=0.0;
      return value;
    }

    int WProjectVisGridder::cOffset(int row, int chan)
    {
      return 0;
    }

  }
}
