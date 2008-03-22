#include <askap_synthesis.h>
#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, ".measurementequation");

#include <askap/AskapError.h>

#include <dataaccess/SharedIter.h>
#include <fitting/Params.h>
#include <measurementequation/ImageFFTEquation.h>
#include <gridding/SphFuncVisGridder.h>
#include <fitting/ImagingNormalEquations.h>
#include <fitting/DesignMatrix.h>
#include <fitting/Axes.h>

#include <gridding/SphFuncVisGridder.h>

#include <scimath/Mathematics/RigidVector.h>

#include <casa/BasicSL/Constants.h>
#include <casa/BasicSL/Complex.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/ArrayMath.h>

#include <stdexcept>

using askap::scimath::Params;
using askap::scimath::Axes;
using askap::scimath::ImagingNormalEquations;
using askap::scimath::DesignMatrix;

namespace askap
{
  namespace synthesis
  {

    ImageFFTEquation::ImageFFTEquation(const askap::scimath::Params& ip,
        IDataSharedIter& idi) : scimath::Equation(ip),
      askap::scimath::ImagingEquation(ip), itsIdi(idi)
    {
      itsGridder = IVisGridder::ShPtr(new SphFuncVisGridder());
      init();
    }
    ;

    ImageFFTEquation::ImageFFTEquation(IDataSharedIter& idi) :
      itsIdi(idi)
    {
      itsGridder = IVisGridder::ShPtr(new SphFuncVisGridder());
      rwParameters()=defaultParameters().clone();
      init();
    }

    ImageFFTEquation::ImageFFTEquation(const askap::scimath::Params& ip,
        IDataSharedIter& idi, IVisGridder::ShPtr gridder) :
      askap::scimath::ImagingEquation(ip), itsGridder(gridder), itsIdi(idi)
    {
      init();
    }
    ;

    ImageFFTEquation::ImageFFTEquation(IDataSharedIter& idi,
        IVisGridder::ShPtr gridder) :
      itsGridder(gridder), itsIdi(idi)
    {
      rwParameters()=defaultParameters().clone();
      init();
    }

    ImageFFTEquation::~ImageFFTEquation()
    {
    }

    askap::scimath::Params ImageFFTEquation::defaultParameters()
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
        static_cast<askap::scimath::Equation*>(this)->operator=(other);
        itsIdi=other.itsIdi;
        itsGridder = other.itsGridder;
      }
      return *this;
    }

    void ImageFFTEquation::init()
    {
    }
    
    /// Clone this into a shared pointer
    /// @return shared pointer to a copy
    ImageFFTEquation::ShPtr ImageFFTEquation::clone() const
    {
      return ImageFFTEquation::ShPtr(new ImageFFTEquation(*this));
    }
    

    void ImageFFTEquation::predict() const
    {
      const vector<string> completions(parameters().completions("image.i"));

      // To minimize the number of data passes, we keep copies of the gridders in memory, and
      // switch between these. This optimization may not be sufficient in the long run.

      itsIdi.chooseOriginal();
      ASKAPLOG_INFO_STR(logger, "Initialising for model degridding" );
      for (vector<string>::const_iterator it=completions.begin();it!=completions.end();it++)
      {
        string imageName("image.i"+(*it));
        const Axes axes(parameters().axes(imageName));
        casa::Array<double> imagePixels(parameters().value(imageName).copy());
        const casa::IPosition imageShape(imagePixels.shape());
        if(itsModelGridders.count(imageName)==0) {
          itsModelGridders[imageName]=itsGridder->clone();
        }
        itsModelGridders[imageName]->initialiseDegrid(axes, imagePixels);
      }
      // Loop through degridding the data
      ASKAPLOG_INFO_STR(logger, "Starting to degrid model" );
      for (itsIdi.init();itsIdi.hasMore();itsIdi.next())
      {
        itsIdi->rwVisibility().set(0.0);
        /*
        ASKAPDEBUGASSERT(itsIdi->nPol() == 4);
        for (casa::uInt p=1;p<3;++p) {
             itsIdi->rwVisibility().xyPlane(p).set(0.);
        }
        */
        for (vector<string>::const_iterator it=completions.begin();it!=completions.end();it++)
        {
          string imageName("image.i"+(*it));
          itsModelGridders[imageName]->degrid(itsIdi);
        }
      }
      ASKAPLOG_INFO_STR(logger, "Finished degridding model" );
    };
    
    /// @brief assign a different iterator
    /// @details This is a temporary method to assign a different iterator.
    /// All this business is a bit ugly, but should go away when all
    /// measurement equations are converted to work with accessors.
    /// @param idi shared pointer to a new iterator
    void ImageFFTEquation::setIterator(IDataSharedIter& idi)
    {
      itsIdi = idi;
    }
    

    // Calculate the residual visibility and image. We transform the model on the fly
    // so that we only have to read (and write) the data once. This uses more memory
    // but cuts down on IO
    void ImageFFTEquation::calcImagingEquations(askap::scimath::ImagingNormalEquations& ne) const
    {

      // We will need to loop over all completions i.e. all sources
      const vector<string> completions(parameters().completions("image.i"));

      // To minimize the number of data passes, we keep copies of the gridders in memory, and
      // switch between these. This optimization may not be sufficient in the long run.      
      // Set up initial gridders for model and for the residuals. This enables us to 
      // do both at the same time.

      for (vector<string>::const_iterator it=completions.begin();it!=completions.end();it++)
      {
        const string imageName("image.i"+(*it));
        const casa::IPosition imageShape(parameters().value(imageName).shape());
        const Axes axes(parameters().axes(imageName));
        casa::Array<double> imagePixels(parameters().value(imageName).copy());
        if(itsModelGridders.count(imageName)==0) {
           itsModelGridders[imageName]=itsGridder->clone();
        }
        if(itsResidualGridders.count(imageName)==0) {
          itsResidualGridders[imageName]=itsGridder->clone();
        }
      }
      // Now we initialise appropriately
      ASKAPLOG_INFO_STR(logger, "Initialising for model degridding and residual gridding" );
      for (vector<string>::const_iterator it=completions.begin();it!=completions.end();it++)
      {
        string imageName("image.i"+(*it));
        const Axes axes(parameters().axes(imageName));
        casa::Array<double> imagePixels(parameters().value(imageName).copy());
        const casa::IPosition imageShape(imagePixels.shape());
        /// First the model
        itsModelGridders[imageName]->initialiseDegrid(axes, imagePixels);
        /// Now the residual images
        itsResidualGridders[imageName]->initialiseGrid(axes, imageShape, true);
      }
      // Now we loop through all the data
      ASKAPLOG_INFO_STR(logger, "Starting degridding model and gridding residuals" );
      for (itsIdi.init();itsIdi.hasMore();itsIdi.next())
      {
        /// Accumulate model visibility for all models
        itsIdi.chooseBuffer("MODEL_DATA");
        itsIdi->rwVisibility().set(0.0);
        for (vector<string>::const_iterator it=completions.begin();it!=completions.end();++it)
        {
          string imageName("image.i"+(*it));
          itsModelGridders[imageName]->degrid(itsIdi);
        }
        /// Now we can calculate the residual visibility and image
        for (vector<string>::const_iterator it=completions.begin();it!=completions.end();it++)
        {
          const string imageName("image.i"+(*it));
          if(parameters().isFree(imageName))
          {
            itsIdi.chooseOriginal();
            itsIdi.buffer("RESIDUAL_DATA").rwVisibility()=itsIdi->visibility()-itsIdi.buffer("MODEL_DATA").visibility();
            itsIdi.chooseBuffer("RESIDUAL_DATA");
            itsResidualGridders[imageName]->grid(itsIdi);
          }
        }
      }
      ASKAPLOG_INFO_STR(logger, "Finished degridding model and gridding residuals" );

      // We have looped over all the data, so now we have to complete the 
      // transforms and fill in the normal equations with the results from the
      // residual gridders
      ASKAPLOG_INFO_STR(logger, "Adding residual image, PSF, and weights image to the normal equations" );
      for (vector<string>::const_iterator it=completions.begin();it!=completions.end();it++)
      {
        const string imageName("image.i"+(*it));
        const casa::IPosition imageShape(parameters().value(imageName).shape());

        casa::Array<double> imagePSF(imageShape);
        casa::Array<double> imageWeight(imageShape);
        casa::Array<double> imageDeriv(imageShape);

        itsResidualGridders[imageName]->finaliseGrid(imageDeriv);
        itsResidualGridders[imageName]->finalisePSF(imagePSF);
        itsResidualGridders[imageName]->finaliseWeights(imageWeight);
        {
          casa::IPosition reference(4, imageShape(0)/2, imageShape(1)/2, 0, 0);
          casa::IPosition vecShape(1, imagePSF.nelements());
          casa::Vector<double> imagePSFVec(imagePSF.reform(vecShape));
          casa::Vector<double> imageWeightVec(imageWeight.reform(vecShape));
          casa::Vector<double> imageDerivVec(imageDeriv.reform(vecShape));
          ne.addSlice(imageName, imagePSFVec, imageWeightVec, imageDerivVec,
              imageShape, reference);
        }
      }
    };

  }

}
