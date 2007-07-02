#include <gridding/TableVisGridder.h>
#include <conrad/ConradError.h>

#include <casa/BasicSL/Constants.h>

using namespace conrad::scimath;
using namespace conrad;

#include <stdexcept>

namespace conrad
{
  namespace synthesis
  {

    TableVisGridder::TableVisGridder() : itsInM(false)
    {
    }

    TableVisGridder::~TableVisGridder()
    {
    }

/// Data to grid (MFS)
    void TableVisGridder::reverse(IDataSharedIter& idi,
      const conrad::scimath::Axes& axes,
      casa::Cube<casa::Complex>& grid,
      casa::Vector<float>& weights)
    {
      casa::Cube<float> visweight(grid.shape()); visweight.set(1.0);
      casa::Vector<double> cellsize;
      findCellsize(cellsize, grid.shape(), axes);
      initConvolutionFunction(idi, cellsize, grid.shape());
      genericReverse(idi->uvw(), idi->visibility(), visweight, idi->frequency(),
        cellsize, grid, weights);
    }

/// Data to grid (spectral line)
    void TableVisGridder::reverse(IDataSharedIter& idi,
      const conrad::scimath::Axes& axes,
      casa::Array<casa::Complex>& grid,
      casa::Matrix<float>& weights)
    {
      casa::Cube<float> visweight(grid.shape()); visweight.set(1.0);
      casa::Vector<double> cellsize;
      findCellsize(cellsize, grid.shape(), axes);
      initConvolutionFunction(idi, cellsize, grid.shape());
      genericReverse(idi->uvw(), idi->visibility(), visweight, idi->frequency(),
        cellsize, grid, weights);
    }

/// Data weights to grid (MFS)
    void TableVisGridder::reverseWeights(IDataSharedIter& idi,
      const conrad::scimath::Axes& axes,
      casa::Cube<casa::Complex>& grid)
    {
      casa::Cube<float> visweight(grid.shape()); visweight.set(1.0);
      casa::Vector<double> cellsize;
      findCellsize(cellsize, grid.shape(), axes);
      initConvolutionFunction(idi, cellsize, grid.shape());
      genericReverseWeights(idi->uvw(), visweight, idi->frequency(),
        cellsize, grid);
    }

/// Data weights to grid (spectral line)
    void TableVisGridder::reverseWeights(IDataSharedIter& idi,
      const conrad::scimath::Axes& axes,
      casa::Array<casa::Complex>& grid)
    {
      casa::Cube<float> visweight(grid.shape()); visweight.set(1.0);
      casa::Vector<double> cellsize;
      findCellsize(cellsize, grid.shape(), axes);
      initConvolutionFunction(idi, cellsize, grid.shape());
      genericReverseWeights(idi->uvw(), visweight, idi->frequency(),
        cellsize, grid);
    }

/// Grid to data (MFS)
    void TableVisGridder::forward(IDataSharedIter& idi,
      const conrad::scimath::Axes& axes,
      const casa::Cube<casa::Complex>& grid)
    {
      casa::Cube<float> visweight(idi->visibility().shape());
      casa::Vector<double> cellsize;
      findCellsize(cellsize, grid.shape(), axes);
      initConvolutionFunction(idi, cellsize, grid.shape());
      genericForward(idi->uvw(), idi->rwVisibility(), visweight, idi->frequency(),
        cellsize, grid);
    }

/// Grid to data (spectral line)
    void TableVisGridder::forward(IDataSharedIter& idi,
      const conrad::scimath::Axes& axes,
      const casa::Array<casa::Complex>& grid)
    {
      casa::Cube<float> visweight(idi->visibility().shape());
      casa::Vector<double> cellsize;
      findCellsize(cellsize, grid.shape(), axes);
      initConvolutionFunction(idi, cellsize, grid.shape());
      genericForward(idi->uvw(), idi->rwVisibility(), visweight, idi->frequency(),
        cellsize, grid);
    }

/// Next are the implementation methods. Not all of these are implemented yet.
/// Data to grid (MFS)
    void TableVisGridder::genericReverse(const casa::Vector<casa::RigidVector<double, 3> >& uvw,
      const casa::Cube<casa::Complex>& visibility,
      const casa::Cube<float>& visweight,
      const casa::Vector<double>& freq,
      const casa::Vector<double>& cellsize,
      casa::Array<casa::Complex>& grid,
      casa::Matrix<float>& sumwt)
    {
      CONRADTHROW(ConradError, "genericReverse not yet implemented");
    }

/// Data to grid (spectral line)
    void TableVisGridder::genericReverse(const casa::Vector<casa::RigidVector<double, 3> >& uvw,
      const casa::Cube<casa::Complex>& visibility,
      const casa::Cube<float>& visweight,
      const casa::Vector<double>& freq,
      const casa::Vector<double>& cellsize,
      casa::Cube<casa::Complex>& grid,
      casa::Vector<float>& sumwt)
    {

      const int gSize = grid.ncolumn();
      const int nSamples = uvw.size();
      const int nChan = freq.size();
//      const int nPol = visibility.shape()(2);
      const int nPol = 1;

// Loop over all samples adding them to the grid
// First scale to the correct pixel location
// Then find the fraction of a pixel to the nearest pixel
// Loop over the entire itsSupport, calculating weights from
// the convolution function and adding the scaled
// visibility to the grid.
      for (int i=0;i<nSamples;i++)
      {
        for (int chan=0;chan<nChan;chan++)
        {

          int overSample=itsOverSample;
          if(itsInM)
          {
            overSample=int(double(itsOverSample)*freq[0]/freq[chan]+0.5);
          }

          for (int pol=0;pol<nPol;pol++)
          {

            int coff=cOffset(i,chan);
            double uScaled=freq[chan]*uvw(i)(0)/(casa::C::c*cellsize(0));
            int iu = nint(uScaled);
            int fracu=nint(overSample*(double(iu)-uScaled));
            iu+=gSize/2;
            double vScaled=freq[chan]*uvw(i)(1)/(casa::C::c*cellsize(1));
            int iv = nint(vScaled);
            int fracv=nint(overSample*(double(iv)-vScaled));
            iv+=gSize/2;

            if(((iu-itsSupport)>0)&&((iv-itsSupport)>0)&&
              ((iu+itsSupport)<gSize)&&((iv+itsSupport)<gSize))
            {
              if(itsSupport==0)
              {
                float wt=itsC(itsCCenter,itsCCenter,coff);
                grid(iu,iv,pol)+=wt*visibility(i,chan,pol);
                sumwt(pol)+=wt;
              }
              else
              {
                int uoff=-itsOverSample*itsSupport+fracu+itsCCenter;
                for (int suppu=-itsSupport;suppu<+itsSupport;suppu++)
                {
                  int voff=-itsOverSample*itsSupport+fracv+itsCCenter;
                  for (int suppv=-itsSupport;suppv<+itsSupport;suppv++)
                  {
                    float wt=itsC(uoff,voff,coff);
                    grid(iu+suppu,iv+suppv,pol)+=wt*visibility(i,chan,pol);
                    sumwt(pol)+=wt;
                    voff+=itsOverSample;
                  }
                  uoff+=itsOverSample;
                }
              }
            }
          }
        }
      }
    }

/// Data weights to grid (spectral line)
    void TableVisGridder::genericReverseWeights(const casa::Vector<casa::RigidVector<double, 3> >& uvw,
      const casa::Cube<float>& visweight,
      const casa::Vector<double>& freq,
      const casa::Vector<double>& cellsize,
      casa::Array<casa::Complex>& grid)
    {
      CONRADTHROW(ConradError, "genericReverseWeights not yet implemented");
    }

/// Data weights to grid (MFS)
    void TableVisGridder::genericReverseWeights(const casa::Vector<casa::RigidVector<double, 3> >& uvw,
      const casa::Cube<float>& visweight,
      const casa::Vector<double>& freq,
      const casa::Vector<double>& cellsize,
      casa::Cube<casa::Complex>& grid)
    {

      const int gSize = grid.ncolumn();
      const int nSamples = uvw.size();
      const int nChan = freq.size();
      const int nPol = 1;

// Loop over all samples adding them to the grid
// First scale to the correct pixel location
// Then find the fraction of a pixel to the nearest pixel
// Loop over the entire itsSupport, calculating weights from
// the convolution function and adding the scaled
// visibility to the grid.
      for (int i=0;i<nSamples;i++)
      {
        for (int chan=0;chan<nChan;chan++)
        {

          int overSample=itsOverSample;
          if(itsInM)
          {
            overSample=int(double(itsOverSample)*freq[0]/freq[chan]+0.5);
          }

          for (int pol=0;pol<nPol;pol++)
          {

            int coff=cOffset(i,chan);
            double uScaled=freq[chan]*uvw(i)(0)/(casa::C::c*cellsize(0));
            int iu = nint(uScaled);
            int fracu=nint(overSample*(double(iu)-uScaled));
            iu+=gSize/2;
            double vScaled=freq[chan]*uvw(i)(1)/(casa::C::c*cellsize(1));
            int iv = nint(vScaled);
            int fracv=nint(overSample*(double(iv)-vScaled));
            iv+=gSize/2;            
            if(((iu-itsSupport)>0)&&((iv-itsSupport)>0)&&
              ((iu+itsSupport)<gSize)&&((iv+itsSupport)<gSize))
            {
              if(itsSupport==0)
              {
                float wt=itsC(itsCCenter,itsCCenter,coff);
                grid(gSize/2,gSize/2,pol)+=wt;
              }
              else
              {
                int uoff=-itsOverSample*itsSupport+fracu+itsCCenter;
                for (int suppu=-itsSupport;suppu<+itsSupport;suppu++)
                {
                  int voff=-itsOverSample*itsSupport+fracv+itsCCenter;
                  for (int suppv=-itsSupport;suppv<+itsSupport;suppv++)
                  {
                    float wt=itsC(uoff,voff,coff);
                    grid(gSize/2+suppu,gSize/2+suppv,pol)+=wt;
                    voff+=itsOverSample;
                  }
                  uoff+=itsOverSample;
                }
              }
            }
          }
        }
      }
    }

/// Grid to data (spectral line)
    void TableVisGridder::genericForward(const casa::Vector<casa::RigidVector<double, 3> >& uvw,
      casa::Cube<casa::Complex>& visibility,
      casa::Cube<float>& visweight,
      const casa::Vector<double>& freq,
      const casa::Vector<double>& cellsize,
      const casa::Array<casa::Complex>& grid)
    {
      CONRADTHROW(ConradError, "genericForward not yet implemented");
    }

/// Grid to data (MFS)
    void TableVisGridder::genericForward(const casa::Vector<casa::RigidVector<double, 3> >& uvw,
      casa::Cube<casa::Complex>& visibility,
      casa::Cube<float>& visweight,
      const casa::Vector<double>& freq,
      const casa::Vector<double>& cellsize,
      const casa::Cube<casa::Complex>& grid)
    {

      const int gSize = grid.ncolumn();
      const int nSamples = uvw.size();
      const int nChan = freq.size();
//      const int nPol = visibility.shape()(2);
      const int nPol = 1;

// Loop over all samples adding them to the grid
// First scale to the correct pixel location
// Then find the fraction of a pixel to the nearest pixel
// Loop over the entire itsSupport, calculating weights from
// the convolution function and adding the scaled
// visibility to the grid.
      for (int i=0;i<nSamples;i++)
      {
        for (int chan=0;chan<nChan;chan++)
        {

          int overSample=itsOverSample;
          if(itsInM)
          {
            overSample=int(double(itsOverSample)*freq[0]/freq[chan]+0.5);
          }

          for (int pol=0;pol<nPol;pol++)
          {

            int coff=cOffset(i,chan);
            double uScaled=freq[chan]*uvw(i)(0)/(casa::C::c*cellsize(0));
            int iu = nint(uScaled);
            int fracu=nint(overSample*(double(iu)-uScaled));
            iu+=gSize/2;
            double vScaled=freq[chan]*uvw(i)(1)/(casa::C::c*cellsize(1));
            int iv = nint(vScaled);
            int fracv=nint(overSample*(double(iv)-vScaled));
            iv+=gSize/2;
                        
            double sumviswt=0.0;
            visibility(i,chan,pol)=0.0;
            if(itsSupport>0)
            {
              if(((iu-itsSupport)>0)&&((iv-itsSupport)>0)&&
                ((iu+itsSupport)<gSize)&&((iv+itsSupport)<gSize))
              {
                if(itsSupport==0)
                {
                  float wt=itsC(itsCCenter,itsCCenter,coff);
                  visibility(i,chan,pol)+=wt*grid(iu,iv,pol);
                  sumviswt+=wt;
                }
                else
                {
                  int uoff=-itsOverSample*itsSupport+fracu+itsCCenter;
                  for (int suppu=-itsSupport;suppu<+itsSupport;suppu++)
                  {
                    int voff=-itsOverSample*itsSupport+fracv+itsCCenter;
                    for (int suppv=-itsSupport;suppv<+itsSupport;suppv++)
                    {
                      float wt=itsC(uoff,voff,coff);
                      visibility(i,chan,pol)+=wt*grid(iu+suppu,iv+suppv,pol);
                      sumviswt+=wt;
                      voff+=itsOverSample;
                    }
                    uoff+=itsOverSample;
                  }
                }
              }
              if(sumviswt>0.0)
              {
                visibility(i,chan,pol)=visibility(i,chan,pol)/casa::Complex(sumviswt);
                visweight(i,chan,pol)=sumviswt;
              }
              else
              {
                visibility(i,chan,pol)=0.0;
              }
            }
            else
            {
              visibility(i,chan,pol)=grid(iu,iv,pol);
              sumviswt=1.0;
            }
//                if((chan==0)&&(pol==0)) {
//                    std::cout << uvw(i)
//                    << " " << iu << " " << iv << " " << coff << " " << fracu << " " << fracv
//                    << " " << visibility(i, chan, pol)
//                    << " " << visweight(i, chan, pol) << std::endl;
//                }
          }
        }
      }
    }

    void TableVisGridder::findCellsize(casa::Vector<double>& cellsize,
      const casa::IPosition& imageShape,
      const conrad::scimath::Axes& axes)
    {

      if(!axes.has("RA")||!axes.has("DEC"))
      {
        throw(std::invalid_argument("RA and DEC specification not present in axes"));
      }
      double raStart=axes.start("RA");
      double raEnd=axes.end("RA");

      double decStart=axes.start("DEC");
      double decEnd=axes.end("DEC");

      cellsize.resize(2);
      cellsize(0)=1.0/(raEnd-raStart);
      cellsize(1)=1.0/(decEnd-decStart);

    }

  }

}
