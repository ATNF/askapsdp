/// @file
///
/// SphFuncVisGridder: Spheroidal function-based visibility gridder.
/// The gridding function is a prolate spheroidal function identical to the
/// one used in AIPS, AIPS++, and probably other packages. At some point
/// we should revisit the tradeoffs since the choice to use this was made
/// about twenty years ago and computers are quite different now.
///
/// The spheroidal function has m = 6, alpha = 1 using the rational
/// approximations discussed by fred schwab in 'indirect imaging'.
/// The gridding function is (1-nu**2)*grdsf(nu) where nu is the distance
/// to the edge. the grid correction function is just 1/grdsf(nu) where nu
/// is now the distance to the edge of the image.

/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef SPHVISGRIDDER_H_
#define SPHVISGRIDDER_H_

#include <gridding/TableVisGridder.h>

namespace conrad
{
  namespace synthesis
  {
    /// Spheroidal function gridder suitable for bog-standard gridding.
    class SphFuncVisGridder : public TableVisGridder
    {
      public:

// Standard two dimensional gridding
        SphFuncVisGridder();

        virtual ~SphFuncVisGridder();

/// Correct for gridding convolution function
/// @param axes axes specifications
/// @param image image to be corrected
        virtual void correctConvolution(const scimath::Axes& axes,
          casa::Cube<double>& image);

/// Apply gridding convolution function in image space
/// @param axes axes specifications
/// @param image image to be corrected
        virtual void applyConvolution(const scimath::Axes& axes,
          casa::Cube<double>& image);

      protected:
      /// Offset into convolution function
      /// @param row Row number
      /// @param chan Channel number
        virtual int cOffset(int row, int chan);
        /// Initialize convolution function
        /// @param idi Data access iterator
        /// @param cellSize Cell size in wavelengths
        /// @param shape Shape of grid
        virtual void initConvolutionFunction(IDataSharedIter& idi, 
          const casa::Vector<double>& cellSize,
          const casa::IPosition& shape);
      private:
      /// Calculate prolate spheroidal function
      /// @param nu Argument for spheroidal function
        double grdsf(double nu);
        /// Initialize lookup table for spheriodal function
        void initSphFunc();
    };
  }
}
#endif
