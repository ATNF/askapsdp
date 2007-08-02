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

#include <gridding/WProjectVisGridder.h>

namespace conrad
{
  namespace synthesis
  {
    /// @brief Gridder that is appropriate for mosaicing. 
    ///
    /// @details The antenna primary beam is used for gridding, though we actually 
    /// work from the illumination pattern since it is better behaved.
    /// @ingroup gridding
    class AntennaIllumVisGridder : public WProjectVisGridder
    {
      public:

/// @brief Construct antenna illumination pattern gridder
/// @param diameter Antenna diameter (meters)
/// @param blockage Antenna blockage (meters)
/// @param wmax Maximum baseline (wavelengths)
/// @param nwplanes Number of w planes
/// @param cutoff Cutoff in determining support e.g. 10^-3 of the peak
/// @param overSample Oversampling (currently limited to <=1)
/// @param maxSupport Maximum support to be allowed
/// @param maxFeeds Maximum number of feeds allowed
	  AntennaIllumVisGridder(const double diameter, const double blockage,
			  const double wmax, const int nwplanes, const double cutoff,
    		const int overSample, const int maxSupport, const int maxFeeds=1);

        virtual ~AntennaIllumVisGridder();

      protected:
      /// Offset into convolution function
      /// @param row Row number
      /// @param chan Channel number
        virtual int cOffset(int row, int chan);
        /// Initialize convolution function
        /// @param idi Data access iterator
        /// @param uvw Input uvw (may be rotated so we cannot use the iterator version)
        /// @param cellSize Cell size in wavelengths
        /// @param shape Shape of grid
        virtual void initConvolutionFunction(IDataSharedIter& idi, 
        		const conrad::scimath::Axes& axes,
        		casa::Vector<casa::RigidVector<double, 3> >& uvw,
        		const casa::Vector<double>& cellSize,
          const casa::IPosition& shape);
      private:
        /// Reference frequency for illumination pattern. 
        double itsReferenceFrequency;
        /// Antenna diameter
        double itsDiameter;
        /// Antenna blockage
        double itsBlockage;
        /// Maximum number of feeds
        int itsMaxFeeds;
        
        /// Find the slopes needed to repoint the antenna
        /// @param idi Data iterator
        /// @param axes Image axes
        /// @param slope Matrix of slopes at 1m east and north
        void findCollimation(IDataSharedIter& idi, 
        		const conrad::scimath::Axes& axes,
	    		casa::Matrix<double>& slope);

    };

  }
}
#endif
