/// @file
///
/// WProjectVisGridder: W projection gridding

/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef WPROJECTGRIDDER_H_
#define WPROJECTGRIDDER_H_

#include <gridding/TableVisGridder.h>

namespace conrad
{
  namespace synthesis
  {
    /// W projection gridder
    class WProjectVisGridder : public TableVisGridder
    {
      public:

// W projection
// @param wmax Maximum baseline (wavelengths)
// @param nwplanes Number of w planes
        WProjectVisGridder(const double wmax, const int nwplanes);

        virtual ~WProjectVisGridder();

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
        void initWProject();
        /// Scaling
        double itsWScale;
        /// Number of w planes
        int itsNWPlanes;
        /// Mapping from row and channel to planes
        casa::Matrix<int> itsCMap;
    };
  }
}
#endif
