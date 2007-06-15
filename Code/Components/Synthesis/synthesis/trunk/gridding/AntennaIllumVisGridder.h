/// @file
///
/// AntennaIllumVisGridder: Grids visibility data using
/// the self-convolution of the antenna illumination
/// pattern.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef ANTENNAILLUMGRIDDER_H_
#define ANTENNAILLUMGRIDDER_H_

#include <gridding/TableVisGridder.h>

namespace conrad
{
  namespace synthesis
  {

    class AntennaIllumVisGridder : public TableVisGridder
    {
      public:

/// Use antenna illumination pattern
/// @param diameter Antenna diameter (meters)
/// @param blockage Antenna blockage (meters)
        AntennaIllumVisGridder(const double diameter, const double blockage);

        virtual ~AntennaIllumVisGridder();

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
        virtual int cOffset(int, int);
        virtual void initConvolutionFunction(IDataSharedIter& idi, const casa::Vector<double>& cellSize,
          const casa::IPosition& shape);
      private:
        double itsReferenceFrequency;
        void selfConvolve(casa::Matrix<casa::Complex>& disk);
        double itsDiameter;
        double itsBlockage;

    };

  }
}
#endif
