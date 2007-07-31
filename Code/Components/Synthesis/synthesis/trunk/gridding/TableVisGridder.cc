#include <gridding/TableVisGridder.h>
#include <conrad/ConradError.h>

#include <scimath/Mathematics/FFTServer.h>
#include <casa/BasicSL/Constants.h>

#include <measures/Measures/MDirection.h>
#include <measures/Measures/UVWMachine.h>

using namespace conrad::scimath;
using namespace conrad;

#include <stdexcept>

namespace conrad
{
  namespace synthesis
  {

    TableVisGridder::TableVisGridder()
    {
    }

    TableVisGridder::TableVisGridder(const int overSample, const int support) : 
  	  itsOverSample(overSample), itsSupport(support)
    {
        CONRADCHECK(overSample>0, "Oversampling must be greater than 0");
        CONRADCHECK(support>0, "Maximum support must be greater than 0");
    }

    TableVisGridder::~TableVisGridder()
    {
    }

/// Data to grid (MFS)
    void TableVisGridder::reverse(IDataSharedIter& idi,
      const conrad::scimath::Axes& axes,
      casa::Cube<casa::Complex>& grid,
      casa::Vector<double>& weights,
      bool dopsf)
    {
      /// @todo Address weighting in TableVisGridder
      casa::Cube<float> visweight(grid.shape()); visweight.set(1.0);
      casa::Vector<double> cellsize;
      findCellsize(cellsize, grid.shape(), axes);
      casa::Vector<casa::RigidVector<double, 3> > outUVW;
      casa::Vector<double> delay;
      rotateUVW(idi, axes, outUVW, delay);
      initConvolutionFunction(idi, axes, outUVW, cellsize, grid.shape());
      genericReverse(outUVW, delay, idi->visibility(), visweight, idi->frequency(),
        cellsize, grid, weights, dopsf);
    }

/// Data weights to grid (MFS)
    void TableVisGridder::reverseWeights(IDataSharedIter& idi,
      const conrad::scimath::Axes& axes,
      casa::Cube<casa::Complex>& grid,
      casa::Vector<double>& weights)
    {
      /// @todo Address weighting in TableVisGridder
      casa::Cube<float> visweight(grid.shape()); visweight.set(1.0);
      casa::Vector<double> cellsize;
      findCellsize(cellsize, grid.shape(), axes);
      casa::Vector<casa::RigidVector<double, 3> > outUVW;
      casa::Vector<double> delay;
      rotateUVW(idi, axes, outUVW, delay);
      initConvolutionFunction(idi, axes, outUVW, cellsize, grid.shape());
      genericReverseWeights(outUVW, delay, visweight, idi->frequency(),
        cellsize, grid, weights);
    }

/// Data to grid (spectral line)
    void TableVisGridder::reverse(IDataSharedIter& idi,
      const conrad::scimath::Axes& axes,
      casa::Array<casa::Complex>& grid,
      casa::Matrix<double>& weights,
      bool dopsf)
    {
      casa::Cube<float> visweight(grid.shape()); visweight.set(1.0);
      casa::Vector<double> cellsize;
      findCellsize(cellsize, grid.shape(), axes);
      casa::Vector<casa::RigidVector<double, 3> > outUVW;
      casa::Vector<double> delay;
      rotateUVW(idi, axes, outUVW, delay);
      initConvolutionFunction(idi, axes, outUVW, cellsize, grid.shape());
      genericReverse(outUVW, delay, idi->visibility(), visweight, idi->frequency(),
        cellsize, grid, weights, dopsf);
    }

/// Grid to data (MFS)
    void TableVisGridder::forward(IDataSharedIter& idi,
      const conrad::scimath::Axes& axes,
      const casa::Cube<casa::Complex>& grid)
    {
      casa::Cube<float> visweight(idi->visibility().shape());
      casa::Vector<double> cellsize;
      findCellsize(cellsize, grid.shape(), axes);
      casa::Vector<double> delay;
      casa::Vector<casa::RigidVector<double, 3> > outUVW;
      rotateUVW(idi, axes, outUVW, delay);
      initConvolutionFunction(idi, axes, outUVW, cellsize, grid.shape());
      genericForward(outUVW, delay, idi->rwVisibility(), visweight, idi->frequency(),
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
      casa::Vector<double> delay;
      casa::Vector<casa::RigidVector<double, 3> > outUVW;
      rotateUVW(idi, axes, outUVW, delay);
      initConvolutionFunction(idi, axes, outUVW, cellsize, grid.shape());
      genericForward(outUVW, delay, idi->rwVisibility(), visweight, idi->frequency(),
        cellsize, grid);
    }
    
    void TableVisGridder::rotateUVW(IDataSharedIter& idi, const conrad::scimath::Axes& axes,
		casa::Vector<casa::RigidVector<double, 3> >& outUVW, casa::Vector<double>& delay) 
    {
        casa::Quantum<double> refLon((axes.start("RA")+axes.end("RA"))/2.0, "rad");
        casa::Quantum<double> refLat((axes.start("DEC")+axes.end("DEC"))/2.0, "rad");
		casa::MDirection out(refLon, refLat, casa::MDirection::J2000);
        const int nSamples = idi->uvw().size();
        delay.resize(nSamples);
        outUVW.resize(nSamples);

        /// @todo Deal with changing pointing
    	casa::UVWMachine machine(out, idi->pointingDir1()(0), false, true);
        casa::Vector<double> uvw(3);
        for (int row=0;row<nSamples;row++) {
        	/// @todo Decide what to do about pointingDir1!=pointingDir2
            for (int i=0;i<2;i++) uvw(i)=-idi->uvw()(row)(i);
            uvw(2)=idi->uvw()(row)(2);
            machine.convertUVW(delay(row), uvw);
            for (int i=0;i<3;i++) outUVW(row)(i)=uvw(i);
        }
    }

/// Next are the implementation methods. Not all of these are implemented yet.
/// Data to grid (spectral line)
    void TableVisGridder::genericReverse(const casa::Vector<casa::RigidVector<double, 3> >& uvw,
	  	  const casa::Vector<double>& delay,
      const casa::Cube<casa::Complex>& visibility,
      const casa::Cube<float>& visweight,
      const casa::Vector<double>& freq,
      const casa::Vector<double>& cellsize,
      casa::Array<casa::Complex>& grid,
      casa::Matrix<double>& sumwt,
      bool dopsf)
    {
      /// @todo Implement TableVisGridder::genericReverse for spectral line
      CONRADTHROW(ConradError, "genericReverse not yet implemented");
    }

/// Data to grid (MFS)
    void TableVisGridder::genericReverse(const casa::Vector<casa::RigidVector<double, 3> >& uvw,
  	  	  const casa::Vector<double>& delay,
      const casa::Cube<casa::Complex>& visibility,
      const casa::Cube<float>& visweight,
      const casa::Vector<double>& freq,
      const casa::Vector<double>& cellsize,
      casa::Cube<casa::Complex>& grid,
      casa::Vector<double>& sumwt,
      bool dopsf)
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

          double phase=2.0f*casa::C::pi*freq[chan]*delay(i)/(casa::C::c);
          casa::Complex phasor(cos(phase), sin(phase));

          int coff=cOffset(i,chan);
          double uScaled=freq[chan]*uvw(i)(0)/(casa::C::c*cellsize(0));
          int iu = nint(uScaled);
          int fracu=nint(itsOverSample*(double(iu)-uScaled));
          iu+=gSize/2;
          double vScaled=freq[chan]*uvw(i)(1)/(casa::C::c*cellsize(1));
          int iv = nint(vScaled);
          int fracv=nint(itsOverSample*(double(iv)-vScaled));
          iv+=gSize/2;

          for (int pol=0;pol<nPol;pol++)
          {
      		casa::Complex rVis=phasor*conj(visibility(i,chan,pol));
      		if (dopsf) rVis=casa::Complex(1.0, 0.0);
            if(((iu-itsSupport)>0)&&((iv-itsSupport)>0)&&
              ((iu+itsSupport)<gSize)&&((iv+itsSupport)<gSize))
            {
              if(itsSupport==0)
              {
                casa::Complex wt=itsC(itsCCenter,itsCCenter,coff)*visweight(i,chan,pol);
                grid(iu,iv,pol)+=wt*rVis;
                sumwt(pol)+=real(wt);
              }
              else
              {
            	  /// Replacing this by direct pointer arithmetic makes no
            	  /// real difference in performance - not sure why
            	int voff=-itsOverSample*itsSupport+fracv+itsCCenter;
                for (int suppv=-itsSupport;suppv<+itsSupport;suppv++)
                {
                  int uoff=-itsOverSample*itsSupport+fracu+itsCCenter;
                  for (int suppu=-itsSupport;suppu<+itsSupport;suppu++)
                  {
                    casa::Complex wt=itsC(uoff,voff,coff)*visweight(i,chan,pol);
                    grid(iu+suppu,iv+suppv,pol)+=wt*rVis;
                    sumwt(pol)+=real(wt);
                    uoff+=itsOverSample;
                  }
                  voff+=itsOverSample;
                }
              }
            }
          }
        }
      }
    }


    /// Data to grid (MFS)
    void TableVisGridder::genericReverseWeights(const casa::Vector<casa::RigidVector<double, 3> >& uvw,
  	  	  const casa::Vector<double>& delay,
      const casa::Cube<float>& visweight,
      const casa::Vector<double>& freq,
      const casa::Vector<double>& cellsize,
      casa::Cube<casa::Complex>& grid,
      casa::Vector<double>& sumwt)
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

          double phase=2.0f*casa::C::pi*freq[chan]*delay(i)/(casa::C::c);
          casa::Complex phasor(cos(phase), sin(phase));

          int coff=cOffset(i,chan);
          double uScaled=freq[chan]*uvw(i)(0)/(casa::C::c*cellsize(0));
          int iu = nint(uScaled);
          int fracu=nint(itsOverSample*(double(iu)-uScaled));
          iu=gSize/2;
          double vScaled=freq[chan]*uvw(i)(1)/(casa::C::c*cellsize(1));
          int iv = nint(vScaled);
          int fracv=nint(itsOverSample*(double(iv)-vScaled));
          iv=gSize/2;

          for (int pol=0;pol<nPol;pol++)
          {
            if(((iu-itsSupport)>0)&&((iv-itsSupport)>0)&&
              ((iu+itsSupport)<gSize)&&((iv+itsSupport)<gSize))
            {
        	  if(itsSupport==0)
              {
                casa::Complex wt=itsC(itsCCenter,itsCCenter,coff)*visweight(i,chan,pol);
                grid(iu,iv,pol)+=wt;
                sumwt(pol)+=real(wt);
              }
              else
              {
            	  /// Replacing this by direct pointer arithmetic makes no
            	  /// real difference in performance - not sure why
                int voff=-itsOverSample*itsSupport+fracv+itsCCenter;
                for (int suppv=-itsSupport;suppv<+itsSupport;suppv++)
                {
                  int uoff=-itsOverSample*itsSupport+fracu+itsCCenter;
                  for (int suppu=-itsSupport;suppu<+itsSupport;suppu++)
                  {
                    casa::Complex wt=itsC(uoff,voff,coff)*visweight(i,chan,pol);
                    grid(iu+suppu,iv+suppv,pol)+=wt;
                    sumwt(pol)+=real(wt);
                    uoff+=itsOverSample;
                  }
                  voff+=itsOverSample;
                }
              }
            }
          }
        }
      }
    }


/// Grid to data (spectral line)
    void TableVisGridder::genericForward(const casa::Vector<casa::RigidVector<double, 3> >& uvw,
  	  	  const casa::Vector<double>& delay,
      casa::Cube<casa::Complex>& visibility,
      casa::Cube<float>& visweight,
      const casa::Vector<double>& freq,
      const casa::Vector<double>& cellsize,
      const casa::Array<casa::Complex>& grid)
    {
      /// @todo Implement TableVisGridder::genericForward for spectral line
      CONRADTHROW(ConradError, "genericForward not yet implemented");
    }

/// Grid to data (MFS)
    void TableVisGridder::genericForward(const casa::Vector<casa::RigidVector<double, 3> >& uvw,
  	  	  const casa::Vector<double>& delay,
      casa::Cube<casa::Complex>& visibility,
      casa::Cube<float>& visweight,
      const casa::Vector<double>& freq,
      const casa::Vector<double>& cellsize,
      const casa::Cube<casa::Complex>& grid)
    {

      const int gSize = grid.ncolumn();
      const int nSamples = uvw.size();
      const int nChan = freq.size();
      // @todo Fix polarization processing in TableVisGridder
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

          double phase=2.0f*casa::C::pi*freq[chan]*delay(i)/(casa::C::c);
          casa::Complex phasor(cos(phase), sin(phase));

          int coff=cOffset(i,chan);
          double uScaled=freq[chan]*uvw(i)(0)/(casa::C::c*cellsize(0));
          int iu = nint(uScaled);
          int fracu=nint(itsOverSample*(double(iu)-uScaled));
          iu+=gSize/2;
          double vScaled=freq[chan]*uvw(i)(1)/(casa::C::c*cellsize(1));
          int iv = nint(vScaled);
          int fracv=nint(itsOverSample*(double(iv)-vScaled));
          iv+=gSize/2;

          for (int pol=0;pol<nPol;pol++)
          {
            double sumviswt=0.0;
            visibility(i,chan,pol)=0.0;
            if(itsSupport>0)
            {
              if(((iu-itsSupport)>0)&&((iv-itsSupport)>0)&&
                ((iu+itsSupport)<gSize)&&((iv+itsSupport)<gSize))
              {
                if(itsSupport==0)
                {
                  casa::Complex wt=conj(itsC(itsCCenter,itsCCenter,coff));
                  visibility(i,chan,pol)+=wt*grid(iu,iv,pol);
                  sumviswt+=real(wt);
                }
                else
                {
                  int voff=-itsOverSample*itsSupport+fracv+itsCCenter;
                  for (int suppv=-itsSupport;suppv<+itsSupport;suppv++)
                  {
                    int uoff=-itsOverSample*itsSupport+fracu+itsCCenter;
                    for (int suppu=-itsSupport;suppu<+itsSupport;suppu++)
                    {
                      casa::Complex wt=conj(itsC(uoff,voff,coff));
                      visibility(i,chan,pol)+=wt*grid(iu+suppu,iv+suppv,pol);
                      sumviswt+=real(wt);
                      uoff+=itsOverSample;
                    }
                    voff+=itsOverSample;
                  }
                }
              }
              if(sumviswt>0.0)
              {
                visibility(i,chan,pol)=phasor*conj(visibility(i,chan,pol)/casa::Complex(sumviswt));
                visweight(i,chan,pol)=sumviswt;
              }
              else
              {
                visibility(i,chan,pol)=0.0;
              }
            }
            else
            {
              visibility(i,chan,pol)=conj(phasor)*grid(iu,iv,pol);
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
    
    void TableVisGridder::cfft(casa::Cube<casa::Complex>& arr, bool toUV)
    {

      casa::FFTServer<casa::Float,casa::Complex> ffts;
      uint nx=arr.shape()(0);
      uint ny=arr.shape()(1);
      uint nz=arr.shape()(2);
      for (uint iz=0;iz<nz;iz++)
      {
        casa::Matrix<casa::Complex> mat(arr.xyPlane(iz));
        for (uint iy=0;iy<ny;iy++)
        {
          casa::Array<casa::Complex> vec(mat.column(iy));
          ffts.fft(vec, toUV);
        }
        for (uint ix=0;ix<nx;ix++)
        {
          casa::Array<casa::Complex> vec(mat.row(ix));
          ffts.fft(vec, toUV);
        }
      }
    }

    void TableVisGridder::toComplex(casa::Cube<casa::Complex>& out, const casa::Array<double>& in)
    {
      uint nx=in.shape()(0);
      uint ny=in.shape()(1);
      casa::Cube<double> cube(in);
      for (uint iy=0;iy<ny;iy++)
      {
        casa::Vector<double> vec(cube.xyPlane(0).column(iy));
        for (uint ix=0;ix<nx;ix++)
        {
          out(ix,iy,0)=casa::Complex(float(vec(ix)));
        }
      }
    }

    void TableVisGridder::toDouble(casa::Array<double>& out, const casa::Cube<casa::Complex>& in)
    {
      uint nx=in.shape()(0);
      uint ny=in.shape()(1);
      casa::Cube<double> cube(out);
      casa::Matrix<casa::Complex> mat(in.xyPlane(0));
      for (uint iy=0;iy<ny;iy++)
      {
        casa::Vector<casa::Complex> vec(mat.column(iy));
        for (uint ix=0;ix<nx;ix++)
        {
          cube(ix,iy,0)=double(real(vec(ix)));
        }
      }
    }

    // Note that this alters in!
    void TableVisGridder::finaliseReverse(casa::Cube<casa::Complex>& in, 
    		const conrad::scimath::Axes& axes, casa::Cube<double>& out)
    {
      cfft(in, false);
      toDouble(out, in);
      correctConvolution(axes, out);
    }

    // Note that this alters in!
    void TableVisGridder::initialiseForward(casa::Cube<double>& in, 
    		const conrad::scimath::Axes& axes, casa::Cube<casa::Complex>& out)
    {
      correctConvolution(axes, in);
      toComplex(out, in);
      cfft(out, true);
    }

  }

}
