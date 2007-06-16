/// @file
///
/// ImageDFTEquation: Equation for discrete Fourier transform of an image
///
/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef SYNIMAGEDFTEQUATION_H_
#define SYNIMAGEDFTEQUATION_H_

#include <fitting/Params.h>
#include <fitting/Equation.h>

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

    /// @brief Discrete Fourier Transform-based image equations
    ///
    /// This class does predictions and calculates normal equations
    /// images. Parameter names are image.{i,q,u,v}.*
    class ImageDFTEquation : public conrad::scimath::Equation
    {
      public:
        /// Standard constructor
        /// @param ip Parameters
        /// @param idi Data iterator
        ImageDFTEquation(const conrad::scimath::Params& ip,
          IDataSharedIter& idi);
        
        /// Constructor with default parameters
        /// @param idi Data iterator
        ImageDFTEquation(IDataSharedIter& idi);

        /// Copy constructor
        ImageDFTEquation(const ImageDFTEquation& other);
        
        /// Assignment operator
        ImageDFTEquation& operator=(const ImageDFTEquation& other);

        virtual ~ImageDFTEquation();

        /// Return the default parameters
        static conrad::scimath::Params defaultParameters();


/// Predict model visibility
        virtual void predict();

/// Calculate the normal equations
/// @param ne Normal equations
        virtual void calcEquations(conrad::scimath::NormalEquations& ne);

      private:
      /// Iterator giving access to the visibility data
        IDataSharedIter itsIdi;

        void init();
        /// Calculate visibility, and optionally the derivatives.
        /// @param imagepixels Image pixels
        /// @param raStart Start of the RA axis (rad)
        /// @param raEnd End of the RA axis (rad)
        /// @param raCells Number of cells on RA axis
        /// @param decStart Start of the Dec axis (rad)
        /// @param decEnd End of the Dec axis (rad)
        /// @param decCells Number of cells on Dec axis
        /// @param freq Observing frequency (Hz)
        /// @param uvw UVW in a vector of rigid vectors
        /// @param vis Output visibility
        /// @param doDeriv Do we want to calculate the derivative of the 
        /// visibility with respect to each pixel?
        /// @param imageDeriv Image derivative with respect to each pixel
        void calcVisDFT(const casa::Array<double>& imagePixels,
          const double raStart, const double raEnd, const int raCells,
          const double decStart, const double decEnd, const int decCells,
          const casa::Vector<double>& freq,
          const casa::Vector<casa::RigidVector<double, 3> >& uvw,
          casa::Matrix<double>& vis, bool doderiv, casa::Matrix<double>& imageDeriv);
    };

  }

}
#endif
