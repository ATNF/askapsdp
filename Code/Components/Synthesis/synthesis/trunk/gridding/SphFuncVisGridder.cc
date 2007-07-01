#include <gridding/SphFuncVisGridder.h>

namespace conrad
{
  namespace synthesis
  {

    SphFuncVisGridder::SphFuncVisGridder()
    {
      itsSupport=0;
    }

    SphFuncVisGridder::~SphFuncVisGridder()
    {
    }

    /// Initialize the convolution function into the cube. If necessary this
    /// could be optimized by using symmetries.
    void SphFuncVisGridder::initConvolutionFunction(IDataSharedIter& idi, 
      const casa::Vector<double>& cellSize,
      const casa::IPosition& shape)
    {
      initSphFunc();
    }

    void SphFuncVisGridder::initSphFunc() {
      if(itsSupport==3) return;
      itsSupport=3;
      itsOverSample=128;
      itsCSize=2*(itsSupport+1)*itsOverSample;    // 1024;
      itsCCenter=itsCSize/2-1;                    // 511
      itsC.resize(itsCSize, itsCSize, 1);
      for (int ix=0;ix<itsCSize;ix++)
      {
        double nux=std::abs(double(ix-itsCCenter))/double(itsSupport*itsOverSample);
        double fx=grdsf(nux)*(1.0-std::pow(nux,2));
        for (int iy=0;iy<itsCSize;iy++)
        {
          double nuy=std::abs(double(iy-itsCCenter))/double(itsSupport*itsOverSample);
          double fy=grdsf(nuy)*(1.0-std::pow(nuy,2));
          itsC(ix,iy,0)=fx*fy;
        }
      }
    }
    
    /// The Convolution function correction in image space is the
    /// the spheroidal function itself.
    void SphFuncVisGridder::correctConvolution(const scimath::Axes& axes,
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
      for (int iz=0;iz<nz;iz++)
      {
        for(int ix=0;ix<nx;ix++)
        {
          for(int iy=0;iy<ny;iy++)
          {
            grid(ix,iy,iz)*=ccfx(ix)*ccfy(iy);
          }
        }
      }
    }

    /// The Convolution function correction in image space is the
    /// the spheroidal function itself.
    void SphFuncVisGridder::applyConvolution(const scimath::Axes& axes,
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
      for (int iz=0;iz<nz;iz++)
      {
        for(int ix=0;ix<nx;ix++)
        {
          for(int iy=0;iy<ny;iy++)
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
    double SphFuncVisGridder::grdsf (double nu)
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

    int SphFuncVisGridder::cOffset(int row, int chan)
    {
      return 0;
    }

  }
}
