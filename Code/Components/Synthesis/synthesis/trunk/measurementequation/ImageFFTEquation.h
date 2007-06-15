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
#include <fitting/Equation.h>

#include <gridding/IVisGridder.h>
#include <dataaccess/SharedIter.h>
#include <dataaccess/IDataIterator.h>

#include <casa/aips.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>

namespace conrad
{
  namespace synthesis
  {

    /// @brief FFT-based image equations
    ///
    /// This class does predictions and calculates normal equations
    /// images. Parameter names are image.{i,q,u,v}.*
    /// The transforms are done using gridding and FFTs.
    class ImageFFTEquation : public conrad::scimath::Equation
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
        
        /// @brief Use the specified gridder
        /// @param gridder Shared pointer to a gridder
        void setGridder(IVisGridder::ShPtr gridder);

/// Predict model visibility
        virtual void predict();

/// Calculate the normal equations
/// @param ne Normal equations
        virtual void calcEquations(conrad::scimath::NormalEquations& ne);

      private:
      
      /// Pointer to gridder
        IVisGridder::ShPtr itsGridder;
        
      /// Iterator giving access to the data
        IDataSharedIter itsIdi;

        void init();

        /// FFT helper function
        void cfft(casa::Cube<casa::Complex>& arr, bool toUV);
        
        /// Conversion helper function
        void toComplex(casa::Cube<casa::Complex>& out, 
          const casa::Array<double>& in);

        /// Conversion helper function
        void toDouble(casa::Array<double>& out, 
          const casa::Cube<casa::Complex>& in);

    };

  }

}
#endif
