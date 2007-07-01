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
      itsParams=defaultParameters().clone();
      init();
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
      itsParams=defaultParameters().clone();
      init();
    }

    ImageFFTEquation::~ImageFFTEquation() 
    {
    }
    
    conrad::scimath::Params ImageFFTEquation::defaultParameters()
    {
      Params ip;
      ip.add("image");
      return ip;
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
        itsIdi=other.itsIdi;
        itsGridder = other.itsGridder;
      }
    }

    void ImageFFTEquation::init()
    {
    }

    void ImageFFTEquation::predict()
    {
      vector<string> completions(parameters().completions("image.i"));
      vector<string>::iterator it;
      
//      itsIdi.chooseBuffer("model");
      /// @todo Minimise ffts in predict
      for (itsIdi.init();itsIdi.hasMore();itsIdi.next())
      {
        itsIdi.chooseBuffer("MODEL_DATA");
        itsIdi->rwVisibility().set(0.0);
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
// Loop over all completions i.e. all sources
      const vector<string> completions(parameters().completions("image.i"));
      
      /// @todo Minimize ffts in calcEquations
      for (itsIdi.init();itsIdi.hasMore();itsIdi.next())
//      itsIdi.init();
      {
        for (vector<string>::const_iterator it=completions.begin();it!=completions.end();it++)
        {
          string imageName("image.i"+(*it));
          if(parameters().isFree(imageName)) {

            casa::Cube<double> imagePixels(parameters().value(imageName).copy());
            const casa::IPosition imageShape(imagePixels.shape());
            casa::Cube<casa::Complex> uvGrid(imageShape(0), imageShape(1), 1);
            uvGrid.set(0.0);
  
            casa::Cube<double> imageWeights(imageShape(0), imageShape(1), 1);
            casa::Cube<double> imagePSF(imageShape(0), imageShape(1), 1);
            casa::Cube<double> imageDeriv(imageShape(0), imageShape(1), 1);
            
            itsIdi.chooseOriginal();
            const casa::Cube<casa::Complex> vis(itsIdi->visibility().copy());
  
  // Predict the model visibility
            itsIdi.chooseBuffer("SCRATCH_DATA");
            itsIdi->rwVisibility().set(casa::Complex(0.0));
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
