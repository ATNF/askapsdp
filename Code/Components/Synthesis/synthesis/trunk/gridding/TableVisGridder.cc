#include <gridding/TableVisGridder.h>
#include <conrad/ConradError.h>
#include <conrad/ConradUtil.h>
#include <fft/FFTWrapper.h>

#include <casa/BasicSL/Constants.h>
#include <casa/Arrays/ArrayIter.h>
#include <casa/Arrays/Slicer.h>

#include <measures/Measures/MDirection.h>
#include <measures/Measures/UVWMachine.h>

#include <fitting/Params.h>
#include <fitting/ParamsCasaTable.h>

using namespace conrad::scimath;
using namespace conrad;

#include <ostream>

#include <casa/OS/Timer.h>

namespace conrad
{
  namespace synthesis
  {

    TableVisGridder::TableVisGridder() :
      itsName(""), itsModelIsEmpty(false),
      itsNumberGridded(0), itsNumberDegridded(0),
      itsTimeGridded(0.0), itsTimeDegridded(0.0)

    {
    }

    TableVisGridder::TableVisGridder(const int overSample, const int support,
        const std::string& name) :
      itsSupport(support), itsOverSample(overSample), itsName(name),
          itsModelIsEmpty(false),
          itsNumberGridded(0), itsNumberDegridded(0),
          itsTimeGridded(0.0), itsTimeDegridded(0.0)
    {
      CONRADCHECK(overSample>0, "Oversampling must be greater than 0");
      CONRADCHECK(support>0, "Maximum support must be greater than 0");
    }

    TableVisGridder::~TableVisGridder()
    {
      std::cout << "TableVisGridder statistics" << std::endl;
      if (itsNumberGridded>0)
      {
        std::cout << "   Total time gridding   = " << itsTimeGridded << " (s)"
            << std::endl;
        std::cout << "   Number gridded        = " << itsNumberGridded
            << std::endl;
        std::cout << "   Time per grid         = " << 1e9*itsTimeGridded/itsNumberGridded << " (ns)" << std::endl;
      }

      if (itsNumberDegridded>0)
      {
        std::cout << "   Total time degridding = " << itsTimeDegridded << " (s)"
            << std::endl;
        std::cout << "   Number degridded      = " << itsNumberDegridded
            << std::endl;
        std::cout << "   Time per degrid       = " << 1e9*itsTimeDegridded/itsNumberDegridded << " (ns)" << std::endl;
      }
    }

    /// Totally selfcontained gridding
    void TableVisGridder::gridKernel(casa::Matrix<casa::Complex>& grid,
        casa::Complex& sumwt, casa::Matrix<casa::Complex>& convFunc,
        const casa::Complex& cVis, const float& viswt, const int iu,
        const int iv, const int support, const int overSample,
        const int cCenter, const int fracu, const int fracv)
    {
      /// Gridding visibility to grid
      CONRADCHECK(-overSample*support+fracu+cCenter>-1,
          "Indexing outside of convolution function");
      CONRADCHECK(+overSample*support+fracu+cCenter<convFunc.shape()(0),
          "Indexing outside of convolution function");
      CONRADCHECK(-overSample*support+fracv+cCenter>-1,
          "Indexing outside of convolution function");
      CONRADCHECK(+overSample*support+fracv+cCenter<convFunc.shape()(1),
          "Indexing outside of convolution function");
#define GRID_WITH_POINTERS 1
#ifdef GRID_WITH_POINTERS
      for (int suppv=-support; suppv<+support; suppv++)
      {
        int voff=suppv*overSample+fracv+cCenter;
        int uoff=-support*overSample+fracu+cCenter;
        casa::Complex *wtPtr=&convFunc(uoff, voff);
        casa::Complex *gridPtr=&(grid(iu-support, iv+suppv));
        for (int suppu=-support; suppu<+support; suppu++)
        {
          (*gridPtr)+=cVis*(*wtPtr);
          wtPtr+=overSample;
          gridPtr++;
        }
      }
#else
      for (int suppv=-support; suppv<+support; suppv++)
      {
        int voff=suppv*overSample+fracv+cCenter;
        for (int suppu=-support; suppu<+support; suppu++)
        {
          int uoff=suppu*overSample+fracu+cCenter;
          casa::Complex wt=convFunc(uoff, voff);
          grid(iu+suppu, iv+suppv)+=cVis*wt;
        }
      }
#endif
      sumwt+=viswt;
    }

    /// Totally selfcontained degridding
    void TableVisGridder::degridKernel(casa::Complex& cVis,
        const casa::Matrix<casa::Complex>& convFunc,
        const casa::Matrix<casa::Complex>& grid, const int iu, const int iv,
        const int support, const int overSample, const int cCenter,
        const int fracu, const int fracv)
    {
      /// Degridding from grid to visibility. Here we just take a weighted sum of the visibility
      /// data using the convolution function as the weighting function. 
      cVis=0.0;
#ifdef GRID_WITH_POINTERS
      for (int suppv=-support; suppv<+support; suppv++)
      {
        int voff=suppv*overSample+fracv+cCenter;
        int uoff=-support*overSample+fracu+cCenter;
        const casa::Complex *wtPtr=&convFunc(uoff, voff);
        const casa::Complex *gridPtr=&(grid(iu-support, iv+suppv));
        for (int suppu=-support; suppu<+support; suppu++)
        {
          cVis+=(*wtPtr)*conj(*gridPtr);
          wtPtr+=overSample;
          gridPtr++;
        }
      }
#else
      for (int suppv=-support; suppv<+support; suppv++)
      {
        int voff=suppv*overSample+fracv+cCenter;
        for (int suppu=-support; suppu<+support; suppu++)
        {
          int uoff=suppu*overSample+fracu+cCenter;
          casa::Complex wt=convFunc(uoff, voff);
          cVis+=wt*conj(grid(iu+suppu, iv+suppv));
        }
      }
#endif
    }

    void TableVisGridder::save(const std::string& name)
    {
      conrad::scimath::ParamsCasaTable iptable(name, false);
      conrad::scimath::Params ip;
      for (unsigned int i=0; i<itsConvFunc.size(); i++)
      {
        {
          casa::Array<double> realC(itsConvFunc[i].shape());
          toDouble(realC, itsConvFunc[i]);
          std::ostringstream os;
          os<<"Real.Convolution";
          os.width(5);
          os.fill('0');
          os<<i;
          ip.add(os.str(), realC);
        }
        {
          casa::Array<double> realC(itsConvFunc[i].shape());
          toDouble(realC, itsConvFunc[i]);
          std::ostringstream os;
          os<<"Real.Convolution";
          os.width(5);
          os.fill('0');
          os<<i;
          ip.add(os.str(), realC);
        }
      }
      iptable.setParameters(ip);
    }

    /// This is a generic grid/degrid
    void TableVisGridder::generic(IDataSharedIter& idi, bool forward)
    {
      if (forward&&itsModelIsEmpty)
        return;

      casa::Vector<casa::RigidVector<double, 3> > outUVW;
      casa::Vector<double> delay;
      rotateUVW(idi, outUVW, delay);

      initIndices(idi);
      initConvolutionFunction(idi);

      CONRADCHECK(itsSupport>0, "Support must be greater than 0");

      const int nSamples = idi->uvw().size();
      const int nChan = idi->frequency().size();
      const int nPol = idi->visibility().shape()(2);

      const casa::IPosition onePlane4D(4, itsShape(0), itsShape(1), 1, 1);
      const casa::IPosition onePlane(2, itsShape(0), itsShape(1));

      // Loop over all samples adding them to the grid
      // First scale to the correct pixel location
      // Then find the fraction of a pixel to the nearest pixel
      // Loop over the entire itsSupport, calculating weights from
      // the convolution function and adding the scaled
      // visibility to the grid.
      casa::Timer timer;

      timer.mark();

      for (int i=0; i<nSamples; i++)
      {
        /// Temporarily fix to do MFS only
        int imageChan=0;
        int imagePol=0;

        for (int chan=0; chan<nChan; chan++)
        {

          /// Scale U,V to integer pixels plus fractional terms
          double uScaled=idi->frequency()[chan]*idi->uvw()(i)(0)/(casa::C::c *itsUVCellSize(0));
          int iu = nint(uScaled);
          const int fracu=nint(itsOverSample*(double(iu)-uScaled));
          iu+=itsShape(0)/2;
          double vScaled=idi->frequency()[chan]*idi->uvw()(i)(1)/(casa::C::c *itsUVCellSize(1));
          int iv = nint(vScaled);
          const int fracv=nint(itsOverSample*(double(iv)-vScaled));
          iv+=itsShape(1)/2;

          /// Calculate the delay phasor
          const double phase=2.0f*casa::C::pi*idi->frequency()[chan]*delay(i)/(casa::C::c);
          const casa::Complex phasor(cos(phase), sin(phase));

          /// Now loop over all visibility polarizations
          for (int pol=0; pol<nPol; pol++)
          {

            /// Make a slicer to extract just this plane
            /// @todo Enable pol and chan maps
            const casa::IPosition ipStart(4, 0, 0, imagePol, imageChan);
            const casa::Slicer slicer(ipStart, onePlane4D);

            /// Lookup the portion of grid and convolution function to be
            /// used for this row, polarisation and channel

            const int gInd=gIndex(i, pol, chan);
            CONRADCHECK(gInd>-1, "Index into image grid is less than zero");
            CONRADCHECK(gInd<itsGrid.size(),
                "Index into image grid exceeds number of planes");
            const int cInd=cIndex(i, pol, chan);
            CONRADCHECK(cInd>-1,
                "Index into convolution functions is less than zero");
            CONRADCHECK(cInd<itsConvFunc.size(),
                "Index into convolution functions exceeds number of planes");

            casa::Matrix<casa::Complex> & convFunc(itsConvFunc[cInd]);

            casa::Array<casa::Complex> aGrid(itsGrid[gInd](slicer));
            casa::Matrix<casa::Complex> grid(aGrid.nonDegenerate());

            /// Need to check if this point lies on the grid (taking into 
            /// account the support)
            if (((iu-itsSupport)>0)&&((iv-itsSupport)>0)&&((iu +itsSupport)
                <itsShape(0))&&((iv+itsSupport) <itsShape(1)))
            {
              if (forward)
              {
                casa::Complex cVis(idi->visibility()(i, chan, pol));
                degridKernel(cVis, convFunc, grid, iu, iv, itsSupport,
                    itsOverSample, itsCCenter, fracu, fracv);
                idi->rwVisibility()(i, chan, pol)=cVis*phasor;
              }
              else
              {
                /// Gridding visibility data onto grid
                const casa::Complex rVis=phasor *conj(idi->visibility()(i,
                    chan, pol));
                casa::Complex sumwt=0.0;
                const float wtVis(1.0);
                gridKernel(grid, sumwt, convFunc, rVis, wtVis, iu, iv,
                    itsSupport, itsOverSample, itsCCenter, fracu, fracv);

                itsSumWeights(cInd, imagePol, imageChan)+=sumwt;

                /// Grid PSF?
                if (itsDopsf)
                {
                  casa::Array<casa::Complex> aGridPSF(itsGridPSF[gInd](slicer));
                  casa::Matrix<casa::Complex> gridPSF(aGridPSF.nonDegenerate());
                  const casa::Complex uVis(1.0);
                  gridKernel(gridPSF, sumwt, convFunc, uVis, wtVis, iu, iv,
                      itsSupport, itsOverSample, itsCCenter, fracu, fracv);
                }
              }
            }
          }
        }
      }
      if (forward)
      {
        itsTimeDegridded+=timer.real();
        itsNumberDegridded+=double((2*itsSupport+1)*(2*itsSupport+1))*double(nSamples*nChan
            *nPol);
      }
      else
      {
        itsTimeGridded+=timer.real();
        itsNumberGridded+=double((2*itsSupport+1)*(2*itsSupport+1))*double(nSamples*nChan*nPol);
        if(itsDopsf) {
          itsNumberGridded+=double((2*itsSupport+1)*(2*itsSupport+1))*double(nSamples*nChan*nPol);
        }
      }
    }

    void TableVisGridder::degrid(IDataSharedIter& idi)
    {
      return generic(idi, true);
    }

    void TableVisGridder::grid(IDataSharedIter& idi)
    {
      return generic(idi, false);
    }

    void TableVisGridder::rotateUVW(IDataSharedIter& idi,
        casa::Vector<casa::RigidVector<double, 3> >& outUVW,
        casa::Vector<double>& delay)
    {
      casa::Quantum<double> refLon( (itsAxes.start("RA") +itsAxes.end("RA"))
          /2.0, "rad");
      casa::Quantum<double> refLat( (itsAxes.start("DEC") +itsAxes.end("DEC"))
          /2.0, "rad");
      casa::MDirection out(refLon, refLat, casa::MDirection::J2000);
      const int nSamples = idi->uvw().size();
      delay.resize(nSamples);
      outUVW.resize(nSamples);

      casa::Vector<double> uvw(3);
      for (int row=0; row<nSamples; row++)
      {
        /// @todo Decide what to do about pointingDir1!=pointingDir2
        for (int i=0; i<2; i++)
        {
          uvw(i)=-idi->uvw()(row)(i);
        }
        uvw(2)=idi->uvw()(row)(2);
        casa::UVWMachine machine(out, idi->pointingDir1()(row), false, true);
        machine.convertUVW(delay(row), uvw);
        for (int i=0; i<3; i++)
          outUVW(row)(i)=uvw(i);
      }
    }

    /// Convert from a double array to a casa::Complex array of the
    /// same size. No limits on dimensions.
    void TableVisGridder::toComplex(casa::Array<casa::Complex>& out,
        const casa::Array<double>& in)
    {
      out.resize(in.shape());
      int nx=in.shape()(0);
      int ny=in.shape()(0);

      casa::ReadOnlyArrayIterator<double> inIt(in, 2);
      casa::ArrayIterator<casa::Complex> outIt(out, 2);
      while (!inIt.pastEnd()&&!outIt.pastEnd())
      {
        casa::Matrix<double> inMat(inIt.array());
        casa::Matrix<casa::Complex> outMat(outIt.array());
        for (int iy=0; iy<ny; iy++)
        {
          for (int ix=0; ix<nx; ix++)
          {
            outMat(ix, iy)=casa::Complex(float(inMat(ix,iy)));
          }
        }
        inIt.next();
        outIt.next();
      }
    }

    /// Convert from a casa::Complex array to a double of the
    /// same size. No limits on dimensions.
    void TableVisGridder::toDouble(casa::Array<double>& out,
        const casa::Array<casa::Complex>& in)
    {
      out.resize(in.shape());
      int nx=in.shape()(0);
      int ny=in.shape()(0);

      casa::ReadOnlyArrayIterator<casa::Complex> inIt(in, 2);
      casa::ArrayIterator<double> outIt(out, 2);
      while (!inIt.pastEnd()&&!outIt.pastEnd())
      {
        casa::Matrix<casa::Complex> inMat(inIt.array());
        casa::Matrix<double> outMat(outIt.array());
        for (int iy=0; iy<ny; iy++)
        {
          for (int ix=0; ix<nx; ix++)
          {
            outMat(ix, iy)=double(casa::real(inMat(ix,iy)));
          }
        }
        inIt.next();
        outIt.next();
      }
    }

    void TableVisGridder::initialiseGrid(const scimath::Axes& axes,
        const casa::IPosition& shape, const bool dopsf)
    {
      itsAxes=axes;
      itsShape=shape;
      itsDopsf=dopsf;

      /// We only need one grid
      itsGrid.resize(1);
      itsGrid[0].resize(shape);
      itsGrid[0].set(0.0);
      if (itsDopsf)
      {
        itsGridPSF.resize(1);
        itsGridPSF[0].resize(shape);
        itsGridPSF[0].set(0.0);
      }

      itsSumWeights.resize(1, itsShape(2), itsShape(3));
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

    /// This is the default implementation
    void TableVisGridder::finaliseGrid(casa::Array<double>& out)
    {
      /// Loop over all grids Fourier transforming and accumulating
      for (unsigned int i=0; i<itsGrid.size(); i++)
      {
        casa::Array<casa::Complex> scratch(itsGrid[i].copy());
        fft2d(scratch, false);
        if (i==0)
        {
          toDouble(out, scratch);
        }
        else
        {
          casa::Array<double> work(out.shape());
          toDouble(work, scratch);
          out+=work;
        }
      }
      // Now we can do the convolution correction
      correctConvolution(out);
      out*=double(out.shape()(0))*double(out.shape()(1));
    }

    /// This is the default implementation
    void TableVisGridder::finalisePSF(casa::Array<double>& out)
    {
      /// Loop over all grids Fourier transforming and accumulating
      for (unsigned int i=0; i<itsGridPSF.size(); i++)
      {
        casa::Array<casa::Complex> scratch(itsGridPSF[i].copy());
        fft2d(scratch, false);
        if (i==0)
        {
          toDouble(out, scratch);
        }
        else
        {
          casa::Array<double> work(out.shape());
          toDouble(work, scratch);
          out+=work;
        }
      }
      // Now we can do the convolution correction
      correctConvolution(out);
      out*=double(out.shape()(0))*double(out.shape()(1));
    }

    /// This is the default implementation
    void TableVisGridder::finaliseWeights(casa::Array<double>& out)
    {
      int nPol=itsShape(2);
      int nChan=itsShape(3);

      int nZ=itsSumWeights.shape()(0);

      for (int chan=0; chan<nChan; chan++)
      {
        for (int pol=0; pol<nPol; pol++)
        {
          double sumwt=0.0;
          for (int iz=0; iz<nZ; iz++)
          {
            sumwt+=casa::real(itsSumWeights(iz, pol, chan));
          }

          casa::IPosition ipStart(4, 0, 0, pol, chan);
          casa::IPosition onePlane(4, itsShape(0), itsShape(1), 1, 1);
          casa::Slicer slicer(ipStart, onePlane);
          out(slicer).set(sumwt);
        }
      }
    }

    void TableVisGridder::initialiseDegrid(const scimath::Axes& axes,
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

      /// We only need one grid
      itsGrid.resize(1);
      itsGrid[0].resize(itsShape);

      if (casa::max(casa::abs(in))>0.0)
      {
        itsModelIsEmpty=false;
        casa::Array<double> scratch(in.copy());
        correctConvolution(scratch);
        toComplex(itsGrid[0], scratch);
        fft2d(itsGrid[0], true);
      }
      else
      {
        std::cout << "No need to degrid: model is empty" << std::endl;
        itsModelIsEmpty=true;
        itsGrid[0].set(casa::Complex(0.0));
      }
    }

    /// This is the default implementation
    void TableVisGridder::finaliseDegrid()
    {
      /// Nothing to do
    }

    /// This is the default implementation
    int TableVisGridder::cIndex(int row, int pol, int chan)
    {
      return 0;
    }

    /// This is the default implementation
    int TableVisGridder::gIndex(int row, int pol, int chan)
    {
      return 0;
    }

  }

}
