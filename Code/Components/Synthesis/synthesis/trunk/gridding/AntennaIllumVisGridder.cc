#include <gridding/AntennaIllumVisGridder.h>
#include <scimath/Mathematics/FFTServer.h>
#include <casa/BasicSL/Complex.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/ArrayMath.h>

namespace conrad
{
  namespace synthesis
  {

    AntennaIllumVisGridder::AntennaIllumVisGridder(const double diameter, const double blockage) : TableVisGridder(), itsReferenceFrequency(0.0),
      itsDiameter(diameter), itsBlockage(blockage)
    {
      TableVisGridder::itsInM=true;
    }

    AntennaIllumVisGridder::~AntennaIllumVisGridder()
    {
      itsC.resize(0,0,0);
    }

/// Initialize the convolution function for the disk.
    void AntennaIllumVisGridder::initConvolutionFunction(IDataSharedIter& idi, const casa::Vector<double>& cellSize,
      const casa::IPosition& shape)
    {

      if(idi->frequency()[0]!=itsReferenceFrequency)
      {
        itsReferenceFrequency=idi->frequency()[0];
// Cellsize is in wavelengths so we convert to the physical length (m) at the reference
// frequency (the first channel)
// WARNING: Ignoring different cellsizes!
        double toM=cellSize(0)*casa::C::c/itsReferenceFrequency;
        double rmax=std::pow(itsDiameter*toM,2);
        double rmin=std::pow(itsBlockage*toM,2);
        itsSupport=3;
        itsOverSample=128;
        itsCSize=2*(itsSupport+1)*itsOverSample;  // 1024;
        itsCCenter=itsCSize/2-1;                  // 511
        casa::Matrix<casa::Complex> disk(itsCSize, itsCSize);
        disk.set(0.0);
        for (int ix=0;ix<itsCSize;ix++)
        {
          double nux2=std::pow(std::abs(double(ix-itsCCenter))/double(itsOverSample), 2);
          for (int iy=0;iy<itsCSize;iy++)
          {
            double nuy2=std::pow(std::abs(double(iy-itsCCenter))/double(itsOverSample), 2);
            double r=nux2+nuy2;
            if((r>rmin)&&(r<rmax))
            {
              disk(ix,iy)=1.0;
            }
          }
        }
        itsC.resize(itsCSize, itsCSize, 1);
        selfConvolve(disk);
        itsC.xyPlane(0)=real(disk)/real(disk(itsCCenter, itsCCenter));
      }
    }

/// Convolve the disk with itself
    void AntennaIllumVisGridder::selfConvolve(casa::Matrix<casa::Complex>& disk)
    {

      casa::FFTServer<casa::Float,casa::Complex> ffts;
      uint nx=disk.shape()(0);
      uint ny=disk.shape()(1);
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
      for (uint iy=0;iy<ny;iy++)
      {
        casa::Array<casa::Complex> vec(disk.column(iy));
        ffts.fft(vec, false);
      }
      for (uint ix=0;ix<nx;ix++)
      {
        casa::Array<casa::Complex> vec(disk.row(ix));
        ffts.fft(vec, false);
      }

    }

    void AntennaIllumVisGridder::correctConvolution(const scimath::Axes& axes,
      casa::Cube<double>& grid)
    {
    }

    void AntennaIllumVisGridder::applyConvolution(const scimath::Axes& axes,
      casa::Cube<double>& grid)
    {
    }

    int AntennaIllumVisGridder::cOffset(int row, int chan)
    {
      return 0;
    }

  }
}
