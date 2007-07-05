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
      const vector<string> completions(parameters().completions("image.i"));

// To minimize the number of data passes, we keep copies of the grids in memory, and
// switch between these. This optimization may not be sufficient in the long run.
      
      std::map<string, casa::Cube<casa::Complex>* > grids;
      for (vector<string>::const_iterator it=completions.begin();it!=completions.end();it++)
      {
        string imageName("image.i"+(*it));
        const Axes axes(parameters().axes(imageName));
        casa::Cube<double> imagePixels(parameters().value(imageName).copy());
        const casa::IPosition imageShape(imagePixels.shape());
        itsGridder->correctConvolution(axes, imagePixels);
        grids[imageName]=new casa::Cube<casa::Complex>(imageShape(0), imageShape(1), 1);
// Since we just defined it, we know its there and can use the simple version of the lookup
        toComplex(*grids[imageName], imagePixels);
        cfft(*grids[imageName], true);
      }
// Loop through degridding the data
      for (itsIdi.init();itsIdi.hasMore();itsIdi.next())
      {
        itsIdi.chooseBuffer("MODEL_DATA");
        itsIdi->rwVisibility().set(0.0);
        for (vector<string>::const_iterator it=completions.begin();it!=completions.end();it++)
        {
          string imageName("image.i"+(*it));
          const Axes axes(parameters().axes(imageName));
          itsGridder->forward(itsIdi, axes, *grids[imageName]);
        }
      }
      // Now clean up 
      for (std::map<string, casa::Cube<casa::Complex>* >::iterator it=grids.begin();
        it!=grids.end();++it) 
      {
        delete it->second;
      }
    };

    void ImageFFTEquation::calcEquations(conrad::scimath::NormalEquations& ne)
    {
      
// First we have to do the predict so that the residual data is filled in
      predict();
      
// We will need to loop over all completions i.e. all sources
      const vector<string> completions(parameters().completions("image.i"));
      
// To minimize the number of data passes, we keep copies of the grids in memory, and
// switch between these. This optimization may not be sufficient in the long run.      
// Set up initial grids for all the types we need to make
      vector<string> types(3);
      types[0]="map";
      types[1]="weight";
      types[2]="psf";
      std::map<string, casa::Cube<casa::Complex>* > grids;
      for (vector<string>::const_iterator it=completions.begin();it!=completions.end();it++)
      {
        const string imageName("image.i"+(*it));
        const casa::IPosition imageShape(parameters().value(imageName).shape());
        for (vector<string>::const_iterator type=types.begin();type!=types.end();++type) {
          grids[string(imageName+(*type))]=new casa::Cube<casa::Complex>(imageShape(0), imageShape(1), 1);
          grids[string(imageName+(*type))]->set(0.0);
        }
      }
// Now we loop through all the data
      bool first=true;
      for (itsIdi.init();itsIdi.hasMore();itsIdi.next())
      {
        for (vector<string>::const_iterator it=completions.begin();it!=completions.end();++it)
        {
          const string imageName("image.i"+(*it));
          const Axes axes(parameters().axes(imageName));
          if(parameters().isFree(imageName)) {
            casa::Vector<float> uvWeights(1);
            itsIdi.buffer("RESIDUAL_DATA").rwVisibility()=itsIdi->visibility()-itsIdi.buffer("MODEL_DATA").visibility();
            uvWeights.set(0.0);
            itsGridder->reverse(itsIdi, axes, *grids[string(imageName+"map")], uvWeights);
            itsGridder->reverseWeights(itsIdi, axes, *grids[string(imageName+"weight")]);
            itsIdi.chooseBuffer("SCRATCH_DATA");
            itsIdi->rwVisibility().set(casa::Complex(1.0));
            uvWeights.set(0.0);
            itsGridder->reverse(itsIdi, axes, *grids[string(imageName+"psf")], uvWeights);
            itsIdi.chooseOriginal();
          }
        }
      }

// Now we have to complete the transforms and fill in the normal equations
      for (vector<string>::const_iterator it=completions.begin();it!=completions.end();it++)
      {
        const string imageName("image.i"+(*it));
        const casa::IPosition imageShape(parameters().value(imageName).shape());
        const Axes axes(parameters().axes(imageName));

        casa::Cube<double> imageWeights(imageShape(0), imageShape(1), 1);
        casa::Cube<double> imagePSF(imageShape(0), imageShape(1), 1);
        casa::Cube<double> imageDeriv(imageShape(0), imageShape(1), 1);
  
        {
          casa::Cube<casa::Complex>* uvGrid(grids[string(imageName+"map")]);
          cfft(*uvGrid, false);
          toDouble(imageDeriv, *uvGrid);
          itsGridder->correctConvolution(axes, imageDeriv);
        }
        {
          casa::Cube<casa::Complex>* uvGrid(grids[string(imageName+"weight")]);
          cfft(*uvGrid, false);
          toDouble(imageWeights, *uvGrid);
          itsGridder->correctConvolution(axes, imageWeights);
          /// @todo Understand the correct use of the weights for various gridding functions
          imageWeights.set(imageWeights(casa::IPosition(3, imageShape(0)/2, imageShape(1)/2, 0)));
        }
        {
          casa::Cube<casa::Complex>* uvGrid(grids[string(imageName+"psf")]);
          cfft(*uvGrid, false);
          toDouble(imagePSF, *uvGrid);
          itsGridder->correctConvolution(axes, imagePSF);
        }
        {
          casa::IPosition reference(3, imageShape(0)/2, imageShape(1)/2, 0);
          casa::IPosition vecShape(1, imagePSF.nelements());
          casa::Vector<double> imagePSFVec(imagePSF.reform(vecShape));
          casa::Vector<double> imageWeightsVec(imageWeights.reform(vecShape));
          casa::Vector<double> imageDerivVec(imageDeriv.reform(vecShape));
          ne.addSlice(imageName, imagePSFVec, imageWeightsVec, imageDerivVec, 
            imageShape, reference);
        }
      }
      // Now delete all the grids to avoid memory leaks
      for (std::map<string, casa::Cube<casa::Complex>* >::iterator it=grids.begin();
        it!=grids.end();++it) 
      {
        delete it->second;
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
