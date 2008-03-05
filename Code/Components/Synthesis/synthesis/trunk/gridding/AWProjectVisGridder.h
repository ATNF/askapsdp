/// @file
///
/// AWProjectVisGridder: Grids visibility data using the self-convolution of 
/// the antenna illumination pattern.
///
/// (c) 2007 ASKAP, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef AWPROJECTVISGRIDDER_H_
#define AWPROJECTVISGRIDDER_H_

#include <gridding/WProjectVisGridder.h>

namespace askap
{
  namespace synthesis
  {
    /// @brief Gridder that is appropriate for mosaicing. 
    ///
    /// @details The visibilities are gridded using a convolution
    /// function derived from the antenna illumination pattern,
    /// appropriately shifted in position for each feed, and
    /// incorporating the Fresnel term needed to correct for the
    /// w-term in the full measurement equation.
    ///
    /// The scaling is slow in data points, slow in w planes 
    /// (since the calculation of the convolution function
    /// usually dominates).
    ///
    /// @ingroup gridding
    class AWProjectVisGridder : public WProjectVisGridder
    {
  public:

      /// @brief Construct antenna illumination pattern/W term gridder
      /// @param diameter Antenna diameter (meters)
      /// @param blockage Antenna blockage (meters)
      /// @param wmax Maximum baseline (wavelengths)
      /// @param nwplanes Number of w planes
      /// @param cutoff Cutoff in determining support e.g. 10^-3 of the peak
      /// @param overSample Oversampling (currently limited to <=1)
      /// @param maxSupport Maximum support to be allowed
      /// @param frequencyDependent Frequency dependent gridding?
      /// @param maxFeeds Maximum number of feeds allowed
      /// @param name Name of table to save convolution function into
      AWProjectVisGridder(const double diameter, const double blockage,
          const double wmax, const int nwplanes, const double cutoff,
          const int overSample, const int maxSupport,
          const int maxFeeds=1, const bool frequencyDependent=true, 
          const std::string& name=std::string(""));

      virtual ~AWProjectVisGridder();

      /// Clone a copy of this Gridder
      virtual IVisGridder::ShPtr clone();

      /// Form the sum of the convolution function squared, multiplied by the weights for each
      /// different convolution function. This is used in the evaluation of the second derivative.
      /// @param out Output double precision grid
      virtual void finaliseWeights(casa::Array<double>& out);

  protected:
      /// @brief Initialise the indices
      virtual void initIndices(IDataSharedIter& idi);

      /// Index into convolution function
      /// @param row Row number
      /// @param pol Polarization id
      /// @param chan Channel number
      virtual int cIndex(int row, int pol, int chan);

      /// Initialize convolution function
      /// @param idi Data access iterator
      virtual void initConvolutionFunction(IDataSharedIter& idi);

  private:
      /// Reference frequency for illumination pattern. 
      double itsReferenceFrequency;
      /// Antenna diameter
      double itsDiameter;
      /// Antenna blockage
      double itsBlockage;
      /// Is the convolution function frequency dependent?
      bool itsFreqDep;
      /// Maximum number of feeds
      int itsMaxFeeds;

      /// Find the slopes needed to repoint the antenna
      /// @param idi Data iterator
      /// @param slope Matrix of slopes at 1m east and north
      void findCollimation(IDataSharedIter& idi, casa::Matrix<double>& slope);
      /// Pad up in size using FFT
      /// @param in Input Array
      /// @param out Output Array
      void fftPad(const casa::Array<double>& in, casa::Array<double>& out);
    };

  }
}
#endif
