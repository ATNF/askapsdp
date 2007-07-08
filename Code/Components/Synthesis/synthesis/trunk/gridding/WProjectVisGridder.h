/// @file
///
/// WProjectVisGridder: W projection gridding

/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef CONRAD_SYNTHESIS_WPROJECTVISGRIDDER_H_
#define CONRAD_SYNTHESIS_WPROJECTVISGRIDDER_H_

#include <gridding/SphFuncVisGridder.h>

namespace conrad
{
  namespace synthesis
  {
    /// W projection gridder
    class WProjectVisGridder : public SphFuncVisGridder
    {
      public:

// W projection
// @param wmax Maximum baseline (wavelengths)
// @param nwplanes Number of w planes
        WProjectVisGridder(const double wmax, const int nwplanes,
          const double cutoff, const int overSample);

        virtual ~WProjectVisGridder();

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
        /// Scaling
        double itsWScale;
        /// Number of w planes
        int itsNWPlanes;
        /// Threshold for cutoff of convolution function
        double itsCutoff;
        /// Mapping from row and channel to planes
        casa::Matrix<int> itsCMap;
    };
  }
}
#endif
