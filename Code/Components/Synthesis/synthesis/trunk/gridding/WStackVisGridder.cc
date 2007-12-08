#include <gridding/WStackVisGridder.h>

#include <conrad_synthesis.h>
#include <conrad/ConradLogging.h>
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

    WStackVisGridder::WStackVisGridder(const double wmax, const int nwplanes)
    {
      CONRADCHECK(wmax>0.0, "Baseline length must be greater than zero");
      CONRADCHECK(nwplanes>0, "Number of w planes must be greater than zero");
      CONRADCHECK(nwplanes%2==1, "Number of w planes must be odd");

      itsNWPlanes=nwplanes;
      itsWScale=wmax/double((nwplanes-1)/2);
    }

    WStackVisGridder::~WStackVisGridder()
    {
    }

    /// Clone a copy of this Gridder
    IVisGridder::ShPtr WStackVisGridder::clone()
    {
      return IVisGridder::ShPtr(new WStackVisGridder(*this));
    }

    /// Initialize the convolution function into the cube. If necessary this
    /// could be optimized by using symmetries.
    void WStackVisGridder::initIndices(IDataSharedIter& idi)
    {
      /// We have to calculate the lookup function converting from
      /// row and channel to plane of the w-dependent convolution
      /// function
      const int nSamples = idi->uvw().size();
      const int nChan = idi->frequency().size();
      const int nPol = idi->visibility().shape()(2);

      itsGMap.resize(nSamples, nPol, nChan);
      int cenw=(itsNWPlanes-1)/2;
      for (int i=0; i<nSamples; i++)
      {
        double w=(idi->uvw()(i)(2))/(casa::C::c);
        for (int chan=0; chan<nChan; chan++)
        {
          for (int pol=0; pol<nPol; pol++)
          {
            double freq=idi->frequency()[chan];
            /// Calculate the index into the grids
            itsGMap(i, pol, chan)=cenw+nint(w*freq/itsWScale);
            if (itsGMap(i, pol, chan)<0)
            {
              CONRADLOG_INFO_STR(w << " "<< freq << " "<< itsWScale << " "<< itsGMap(i, pol, chan) );
            }
            CONRADCHECK(itsGMap(i, pol, chan)<itsNWPlanes,
                "W scaling error: recommend allowing larger range of w");
            CONRADCHECK(itsGMap(i, pol, chan)>-1,
                "W scaling error: recommend allowing larger range of w");
          }
        }
      }
    }

    void WStackVisGridder::initialiseGrid(const scimath::Axes& axes,
        const casa::IPosition& shape, const bool dopsf)
    {
      itsAxes=axes;
      itsShape=shape;
      itsDopsf=dopsf;

      /// We only need one grid
      itsGrid.resize(itsNWPlanes);
      for (int i=0; i<itsNWPlanes; i++)
      {
        itsGrid[i].resize(shape);
        itsGrid[i].set(0.0);
      }
      if (itsDopsf)
      {
        itsGridPSF.resize(itsNWPlanes);
        for (int i=0; i<itsNWPlanes; i++)
        {
          itsGridPSF[i].resize(shape);
          itsGridPSF[i].set(0.0);
        }
      }

      itsSumWeights.resize(itsNWPlanes, itsShape(2), itsShape(3));
      itsSumWeights.set(casa::Complex(0.0));

      CONRADCHECK(itsAxes.has("RA")&&itsAxes.has("DEC"),
          "RA and DEC specification not present in axes");

      double raStart=itsAxes.start("RA");
      double raEnd=itsAxes.end("RA");

      double decStart=itsAxes.start("DEC");
      double decEnd=itsAxes.end("DEC");

      itsUVCellSize.resize(2);
      itsUVCellSize(0)=1.0/(raEnd-raStart);
      itsUVCellSize(1)=1.0/(decEnd-decStart);

    }

    void WStackVisGridder::multiply(casa::Array<casa::Complex>& scratch, int i)
    {
      /// These are the actual cell sizes used
      float cellx=1.0/(float(itsShape(0))*itsUVCellSize(0));
      float celly=1.0/(float(itsShape(1))*itsUVCellSize(1));

      int nx=itsShape(0);
      int ny=itsShape(1);

      int cenw=(itsNWPlanes-1)/2;

      float w=2.0f*casa::C::pi*float(i-cenw)*itsWScale;

      casa::ArrayIterator<casa::Complex> it(scratch, 2);
      while (!it.pastEnd())
      {
        casa::Matrix<casa::Complex> mat(it.array());

        /// @todo Optimise multiply loop
        for (int iy=0; iy<ny; iy++)
        {
          float y2=float(iy-ny/2)*celly;
          y2*=y2;
          for (int ix=0; ix<nx; ix++)
          {
            if (casa::abs(mat(ix, iy))>0.0)
            {
              float x2=float(ix-nx/2)*cellx;
              x2*=x2;
              float r2=x2+y2;
              float phase=w*(1.0-sqrt(1.0-r2));
              mat(ix, iy)*=casa::Complex(cos(phase), -sin(phase));
            }
          }
        }
        it.next();
      }
    }

    /// This is the default implementation
    void WStackVisGridder::finaliseGrid(casa::Array<double>& out)
    {

      CONRADLOG_INFO_STR("Stacking " << itsNWPlanes
                         << " planes of W stack to get final image");

      /// Loop over all grids Fourier transforming and accumulating
      bool first=true;
      for (int i=0; i<itsGrid.size(); i++)
      {
        if (casa::max(casa::amplitude(itsGrid[i]))>0.0)
        {
          casa::Array<casa::Complex> scratch(itsGrid[i].copy());
          fft2d(scratch, false);
          multiply(scratch, i);

          if (first)
          {
            first=false;
            toDouble(out, scratch);
          }
          else
          {
            casa::Array<double> work(out.shape());
            toDouble(work, scratch);
            out+=work;
          }
        }
      }
      // Now we can do the convolution correction
      correctConvolution(out);
      out*=double(out.shape()(0))*double(out.shape()(1));
    }

    /// This is the default implementation
    void WStackVisGridder::finalisePSF(casa::Array<double>& out)
    {

      /// Loop over all grids Fourier transforming and accumulating
      bool first=true;
      for (int i=0; i<itsGrid.size(); i++)
      {
        if (casa::max(casa::amplitude(itsGridPSF[i]))>0.0)
        {
          casa::Array<casa::Complex> scratch(itsGridPSF[i].copy());
          fft2d(scratch, false);
          multiply(scratch, i);

          if (first)
          {
            first=false;
            toDouble(out, scratch);
          }
          else
          {
            casa::Array<double> work(out.shape());
            toDouble(work, scratch);
            out+=work;
          }
        }
      }
      // Now we can do the convolution correction
      correctConvolution(out);
      out*=double(out.shape()(0))*double(out.shape()(1));
    }

    void WStackVisGridder::initialiseDegrid(const scimath::Axes& axes,
        const casa::Array<double>& in)
    {

      itsAxes=axes;
      itsShape=in.shape();

      CONRADCHECK(itsAxes.has("RA")&&itsAxes.has("DEC"),
          "RA and DEC specification not present in axes");

      double raStart=itsAxes.start("RA");
      double raEnd=itsAxes.end("RA");

      double decStart=itsAxes.start("DEC");
      double decEnd=itsAxes.end("DEC");

      itsUVCellSize.resize(2);
      itsUVCellSize(0)=1.0/(raEnd-raStart);
      itsUVCellSize(1)=1.0/(decEnd-decStart);

      itsGrid.resize(itsNWPlanes);
      if (casa::max(casa::abs(in))>0.0)
      {
        itsModelIsEmpty=false;
        CONRADLOG_INFO_STR("Filling " << itsNWPlanes
                           << " planes of W stack with model");
        casa::Array<double> scratch(in.copy());
        correctConvolution(scratch);
        for (int i=0; i<itsNWPlanes; i++)
        {
          itsGrid[i].resize(itsShape);
          toComplex(itsGrid[i], scratch);
          multiply(itsGrid[i], i);
          /// Need to conjugate to get sense of w correction correct
          itsGrid[i]=casa::conj(itsGrid[i]);
          fft2d(itsGrid[i], true);
        }
      }
      else
      {
        itsModelIsEmpty=true;
        CONRADLOG_INFO_STR("No need to fill W stack: model is empty");
        for (int i=0; i<itsNWPlanes; i++)
        {
          itsGrid[i].resize(casa::IPosition(1, 1));
          itsGrid[i].set(casa::Complex(0.0));
        }
      }
    }

    int WStackVisGridder::gIndex(int row, int pol, int chan)
    {
      return itsGMap(row, pol, chan);
    }

  }
}
