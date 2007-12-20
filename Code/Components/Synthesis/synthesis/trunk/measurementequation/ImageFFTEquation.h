/// @file
///
/// ImageDFTEquation: Equation for discrete Fourier transform of an image
/// using gridding and FFTs.
///
/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef SYNIMAGEFFTEQUATION_H_
#define SYNIMAGEFFTEQUATION_H_

#include <fitting/Params.h>
#include <fitting/ImagingEquation.h>

#include <gridding/IVisGridder.h>
#include <dataaccess/SharedIter.h>
#include <dataaccess/IDataIterator.h>

#include <casa/aips.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>

#include <map>

namespace conrad
{
  namespace synthesis
  {

    /// @brief FFT-based image equations
    ///
    /// @details This class does predictions and calculates normal equations
    /// images. Parameter names are image.{i,q,u,v}.*
    /// The transforms are done using gridding and FFTs.
    /// @ingroup measurementequation
    class ImageFFTEquation : public conrad::scimath::ImagingEquation
    {
      public:

        /// Standard constructor
        /// @param ip Parameters
        /// @param idi Data iterator
        ImageFFTEquation(const conrad::scimath::Params& ip,
          IDataSharedIter& idi);
        
        /// Constructor with default parameters
        /// @param idi Data iterator
        ImageFFTEquation(IDataSharedIter& idi);

        /// Standard constructor with specified gridder
        /// @param ip Parameters
        /// @param idi Data iterator
        /// @param gridder Shared pointer to a gridder
        ImageFFTEquation(const conrad::scimath::Params& ip,
          IDataSharedIter& idi, IVisGridder::ShPtr gridder);
        
        /// Constructor with default parameters with specified gridder
        /// @param idi Data iterator
        /// @param gridder Shared pointer to a gridder
        ImageFFTEquation(IDataSharedIter& idi, IVisGridder::ShPtr gridder);

        /// Copy constructor
        ImageFFTEquation(const ImageFFTEquation& other);
        
        /// Assignment operator
        ImageFFTEquation& operator=(const ImageFFTEquation& other);


        virtual ~ImageFFTEquation();
        
        /// Return the default parameters
        static conrad::scimath::Params defaultParameters();

/// Predict model visibility
        virtual void predict();

/// Calculate the normal equations
/// @param ne Normal equations
        virtual void calcImagingEquations(conrad::scimath::ImagingNormalEquations& ne);

        /// Clone this into a shared pointer
        /// @return shared pointer to a copy
        virtual ImageFFTEquation::ShPtr clone() const;

      private:
      
      /// Pointer to prototype gridder
        IVisGridder::ShPtr itsGridder;
        
        /// Map of gridders for the model
        std::map<string, IVisGridder::ShPtr> itsModelGridders;
        
        /// Map of gridders for the residuals
        std::map<string, IVisGridder::ShPtr> itsResidualGridders;

        /// Iterator giving access to the data
        IDataSharedIter itsIdi;

        void init();


    };

  }

}
#endif
