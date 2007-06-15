#include <dataaccess/SharedIter.h>
#include <fitting/Params.h>
#include <measurementequation/ImageFFTEquation.h>
#include <gridding/SphFuncVisGridder.h>
#include <fitting/NormalEquations.h>
#include <fitting/DesignMatrix.h>
#include <fitting/Axes.h>

#include <gridding/SphFuncVisGridder.h>

#include <msvis/MSVis/StokesVector.h>
#include <scimath/Mathematics/RigidVector.h>
#include <scimath/Mathematics/FFTServer.h>
#include <casa/BasicSL/Constants.h>
#include <casa/BasicSL/Complex.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/ArrayMath.h>

#include <stdexcept>

using conrad::scimath::Params;
using conrad::scimath::Axes;
using conrad::scimath::NormalEquations;
using conrad::scimath::DesignMatrix;

namespace conrad
{
  namespace synthesis
  {

    ImageFFTEquation::ImageFFTEquation(const conrad::scimath::Params& ip,
      IDataSharedIter& idi) : 
      conrad::scimath::Equation(ip), itsIdi(idi) 
      {
        itsGridder = IVisGridder::ShPtr(new SphFuncVisGridder());
        init();
      };
        
    ImageFFTEquation::ImageFFTEquation(IDataSharedIter& idi) 
      : conrad::scimath::Equation(), itsIdi(idi) 
    {
      itsGridder = IVisGridder::ShPtr(new SphFuncVisGridder());
      init();
      itsParams=itsDefaultParams;
    }

    ImageFFTEquation::ImageFFTEquation(const conrad::scimath::Params& ip,
      IDataSharedIter& idi, IVisGridder::ShPtr gridder) : 
      conrad::scimath::Equation(ip), itsIdi(idi), itsGridder(gridder) 
      {
        init();
      };
        
    ImageFFTEquation::ImageFFTEquation(IDataSharedIter& idi,
      IVisGridder::ShPtr gridder) 
      : conrad::scimath::Equation(), itsIdi(idi), itsGridder(gridder) 
    {
      init();
      itsParams=itsDefaultParams;
    }

    ImageFFTEquation::~ImageFFTEquation() 
    {
    }

        
    void ImageFFTEquation::setGridder(IVisGridder::ShPtr gridder) 
    {
      itsGridder=gridder;
    }
    
    ImageFFTEquation::ImageFFTEquation(const ImageFFTEquation& other)
    {
      operator=(other);
    }

    ImageFFTEquation& ImageFFTEquation::operator=(const ImageFFTEquation& other)
    {
      if(this!=&other)
      {
        itsParams=other.itsParams;
        itsDefaultParams=other.itsDefaultParams;
        itsIdi=other.itsIdi;
        itsGridder = other.itsGridder;
      }
    }

    void ImageFFTEquation::init()
    {
// The default parameters serve as a holder for the patterns to match the actual
// parameters. Shell pattern matching rules apply.
      itsDefaultParams.reset();
      itsDefaultParams.add("image.i");
    }

    void ImageFFTEquation::predict()
    {
      if(parameters().isCongruent(itsDefaultParams))
      {
        throw std::invalid_argument("Parameters not consistent with this equation");
      }

      vector<string> completions(parameters().completions("image.i"));
      vector<string>::iterator it;

      for (itsIdi.init();itsIdi.hasMore();itsIdi.next())
      {

        const casa::Vector<double>& freq=itsIdi->frequency();
        const casa::Vector<double>& time=itsIdi->time();
        const uint nChan=freq.nelements();
        const uint nRow=itsIdi->nRow();
        casa::Matrix<double> vis(nRow,2*nChan);

        for (it=completions.begin();it!=completions.end();it++)
        {

          string imageName("image.i"+(*it));
          const Axes axes(parameters().axes(imageName));
          casa::Cube<double> imagePixels(parameters().value(imageName).copy());
          const casa::IPosition imageShape(imagePixels.shape());
          itsGridder->correctConvolution(axes, imagePixels);
          casa::Cube<casa::Complex> uvGrid(imageShape(0), imageShape(1), 1);
          toComplex(uvGrid, imagePixels);
          cfft(uvGrid, true);
          itsGridder->forward(itsIdi, axes, uvGrid);
        }
      }
    };

    void ImageFFTEquation::calcEquations(conrad::scimath::NormalEquations& ne)
    {
      if(parameters().isCongruent(itsDefaultParams))
      {
        throw std::invalid_argument("Parameters not consistent with this equation");
      }

// Loop over all completions i.e. all sources
      vector<string> completions(parameters().completions("image.i"));
      vector<string>::iterator it;

      for (itsIdi.init();itsIdi.hasMore();itsIdi.next())
      {

        const casa::Vector<double>& freq=itsIdi->frequency();
        const uint nChan=freq.nelements();
        const uint nRow=itsIdi->nRow();

        for (it=completions.begin();it!=completions.end();it++)
        {

          string imageName("image.i"+(*it));
          casa::Cube<double> imagePixels(parameters().value(imageName).copy());
          const casa::IPosition imageShape(imagePixels.shape());
          casa::Cube<casa::Complex> uvGrid(imageShape(0), imageShape(1), 1);
          uvGrid.set(0.0);

          casa::Cube<double> imageWeights(imageShape(0), imageShape(1), 1);
          casa::Cube<double> imagePSF(imageShape(0), imageShape(1), 1);
          casa::Cube<double> imageDeriv(imageShape(0), imageShape(1), 1);

          casa::Cube<casa::Complex> vis(itsIdi->visibility().copy());
          itsIdi->rwVisibility().set(0.0);

// Predict the model visibility
          Axes axes(parameters().axes(imageName));
          itsGridder->correctConvolution(axes, imagePixels);
          toComplex(uvGrid, imagePixels);
          cfft(uvGrid, true);
          itsGridder->forward(itsIdi, axes, uvGrid);
          itsIdi->rwVisibility()=vis-itsIdi->visibility();

// Calculate contribution to residual image
          {
            uvGrid.set(0.0);
            casa::Vector<float> uvWeights(1);
            itsGridder->reverse(itsIdi, axes, uvGrid, uvWeights);
            cfft(uvGrid, false);
            toDouble(imageDeriv, uvGrid);
            itsGridder->correctConvolution(axes, imageDeriv);
          }
// Calculate contribution to weights image (i.e. diagonal of normal matrix)
          {
            uvGrid.set(0.0);
            itsGridder->reverseWeights(itsIdi, axes, uvGrid);
            cfft(uvGrid, false);
            toDouble(imageWeights, uvGrid);
            itsGridder->correctConvolution(axes, imageWeights);
          }
// Calculate contribution to PSF (i.e. slice through normal matrix)
          {
            itsIdi->rwVisibility().set(casa::Complex(1.0));
            uvGrid.set(0.0);
            casa::Vector<float> uvWeights(1);
            itsGridder->reverse(itsIdi, axes, uvGrid, uvWeights);
            cfft(uvGrid, false);
            toDouble(imagePSF, uvGrid);
            itsGridder->correctConvolution(axes, imagePSF);
            itsIdi->rwVisibility()=vis;
          }
// Add everything found to the normal equations
          casa::IPosition reference(3, imageShape(0)/2, imageShape(1)/2, 0);
          {
            casa::IPosition vecShape(1, imagePSF.nelements());
            casa::Vector<double> imagePSFVec(imagePSF.reform(vecShape));
            casa::Vector<double> imageWeightsVec(imageWeights.reform(vecShape));
            casa::Vector<double> imageDerivVec(imageDeriv.reform(vecShape));
            ne.addSlice(imageName, imagePSFVec, imageWeightsVec, imageDerivVec, 
              imageShape, reference);
          }
        }
      }
    };

    void ImageFFTEquation::cfft(casa::Cube<casa::Complex>& arr, bool toUV)
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

    void ImageFFTEquation::toComplex(casa::Cube<casa::Complex>& out, const casa::Array<double>& in)
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

    void ImageFFTEquation::toDouble(casa::Array<double>& out, const casa::Cube<casa::Complex>& in)
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

  }

}
