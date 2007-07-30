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
        grids[imageName]=new casa::Cube<casa::Complex>(imageShape(0), imageShape(1), 1);
// Since we just defined it, we know its there and can use the simple version of the lookup
        itsGridder->initialiseForward(imagePixels, axes, *grids[imageName]);
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
        itsIdi.chooseOriginal();
      }
      // Now clean up 
      for (std::map<string, casa::Cube<casa::Complex>* >::iterator it=grids.begin();
        it!=grids.end();++it) 
      {
        delete it->second;
      }
    };

// Calculate the residual visibility and image. We transform the model on the fly
// so that we only have to read (and write) the data once. This uses more memory
// but cuts down on IO
    void ImageFFTEquation::calcEquations(conrad::scimath::NormalEquations& ne)
    {
      
// We will need to loop over all completions i.e. all sources
      const vector<string> completions(parameters().completions("image.i"));
      
// To minimize the number of data passes, we keep copies of the grids in memory, and
// switch between these. This optimization may not be sufficient in the long run.      
// Set up initial grids for all the types we need to make
      vector<string> types(4);
      types[0]="model";
      types[1]="map";
      types[2]="psf";
      types[3]="weight";
      std::map<string, casa::Cube<casa::Complex>* > grids;
      std::map<string, casa::Vector<casa::Double>* > sumWeights;
      for (vector<string>::const_iterator it=completions.begin();it!=completions.end();it++)
      {
        const string imageName("image.i"+(*it));
        const casa::IPosition imageShape(parameters().value(imageName).shape());
        for (vector<string>::const_iterator type=types.begin();type!=types.end();++type) {
          grids[string(imageName+(*type))]=new casa::Cube<casa::Complex>(imageShape(0), imageShape(1), 1);
          grids[string(imageName+(*type))]->set(0.0);
          sumWeights[string(imageName+(*type))]=new casa::Vector<casa::Double>(1);
          sumWeights[string(imageName+(*type))]->set(0.0);
        }
      }
      // Now we fill in the models
      for (vector<string>::const_iterator it=completions.begin();it!=completions.end();it++)
      {
        string imageName("image.i"+(*it));
        const Axes axes(parameters().axes(imageName));
        casa::Cube<double> imagePixels(parameters().value(imageName).copy());
        const casa::IPosition imageShape(imagePixels.shape());
        string modelName("image.i"+(*it)+"model");
        itsGridder->initialiseForward(imagePixels, axes, *grids[modelName]);
      }      
// Now we loop through all the data
      bool first=true;
      for (itsIdi.init();itsIdi.hasMore();itsIdi.next())
      {
        /// Accumulate model visibility for all models
        itsIdi.chooseBuffer("MODEL_DATA");
        itsIdi->rwVisibility().set(0.0);
        for (vector<string>::const_iterator it=completions.begin();it!=completions.end();++it)
        {
          string modelName("image.i"+(*it)+"model");
          string imageName("image.i"+(*it));
          const Axes axes(parameters().axes(imageName));
          itsGridder->forward(itsIdi, axes, *grids[modelName]);
        }
        /// Now we can calculate the residual visibility and image
        for (vector<string>::const_iterator it=completions.begin();it!=completions.end();it++)
        {
          const string imageName("image.i"+(*it));
          const string mapName(imageName+"map");
          const string psfName(imageName+"psf");
          const string weightName(imageName+"weight");
          const Axes axes(parameters().axes(imageName));
          if(parameters().isFree(imageName)) {
            itsIdi.chooseOriginal();
            casa::Vector<float> uvWeights(1);
            itsIdi.buffer("RESIDUAL_DATA").rwVisibility()=itsIdi->visibility()-itsIdi.buffer("MODEL_DATA").visibility();
            itsIdi.chooseBuffer("RESIDUAL_DATA");
            itsGridder->reverse(itsIdi, axes, *grids[mapName], *sumWeights[mapName]);
            itsIdi.chooseBuffer("SCRATCH_DATA");
            itsIdi->rwVisibility().set(casa::Complex(1.0));
            uvWeights.set(0.0);
            itsGridder->reverse(itsIdi, axes, *grids[psfName], *sumWeights[psfName]);
            itsIdi.chooseOriginal();
          }
        }
      }
      

      // We have looped over all the data, so now we have to complete the 
      // transforms and fill in the normal equations
      for (vector<string>::const_iterator it=completions.begin();it!=completions.end();it++)
      {
        const string imageName("image.i"+(*it));
        const string mapName(imageName+"map");
        const string psfName(imageName+"psf");
        const string weightName(imageName+"weight");
        const casa::IPosition imageShape(parameters().value(imageName).shape());
        const Axes axes(parameters().axes(imageName));

        casa::Cube<double> imagePSF(imageShape(0), imageShape(1), 1);
        casa::Cube<double> imageWeight(imageShape(0), imageShape(1), 1);
        casa::Cube<double> imageDeriv(imageShape(0), imageShape(1), 1);
  
        itsGridder->finaliseReverse(*grids[mapName], axes, imageDeriv);
        itsGridder->finaliseReverse(*grids[psfName], axes, imagePSF);
		imageWeight.set((*sumWeights[mapName])(0)/(float(imageShape(0))*float(imageShape(1))));
        {
          casa::IPosition reference(3, imageShape(0)/2, imageShape(1)/2, 0);
          casa::IPosition vecShape(1, imagePSF.nelements());
          casa::Vector<double> imagePSFVec(imagePSF.reform(vecShape));
          casa::Vector<double> imageWeightVec(imageWeight.reform(vecShape));
          casa::Vector<double> imageDerivVec(imageDeriv.reform(vecShape));
          ne.addSlice(imageName, imagePSFVec, imageWeightVec, imageDerivVec, 
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

  }

}
