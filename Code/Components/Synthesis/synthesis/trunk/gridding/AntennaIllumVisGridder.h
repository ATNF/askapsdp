/// @file
///
/// AntennaIllumVisGridder: Grids visibility data using the self-convolution of 
/// the antenna illumination pattern.
///
/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef ANTENNAILLUMGRIDDER_H_
#define ANTENNAILLUMGRIDDER_H_

#include <gridding/TableVisGridder.h>

namespace conrad
{
  namespace synthesis
  {
    /// @brief Gridder that is appropriate for mosaicing. 
    ///
    /// The antenna primary beam is used for gridding, though we actually 
    /// work from the illumination pattern since it is better behaved.
    class AntennaIllumVisGridder : public TableVisGridder
    {
      public:

/// @brief Construct antenna illumination pattern gridder
/// @param diameter Antenna diameter (meters)
/// @param blockage Antenna blockage (meters)
        AntennaIllumVisGridder(const double diameter, const double blockage);

        virtual ~AntennaIllumVisGridder();

/// @brief Correct for gridding convolution function in image space.
/// This is a no-op since it should be corrected using the normal
/// equations mechanism
/// @param axes axes specifications
/// @param image image to be corrected
        virtual void correctConvolution(const scimath::Axes& axes,
          casa::Cube<double>& image);

/// @brief Apply gridding convolution function in image space
/// This is a no-op.
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
        /// Reference frequency for illumination pattern. 
        double itsReferenceFrequency;
        /// Convolve disk with itself to get the convolution function
        void selfConvolve(casa::Matrix<casa::Complex>& disk);
        /// Antenna diameter
        double itsDiameter;
        /// Antenna blockage
        double itsBlockage;

    };

  }
}
#endif
