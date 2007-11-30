/// @file
///
/// AProjectWStackVisGridder: Grids visibility data using the self-convolution of 
/// the antenna illumination pattern.
///
/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef APROJECTWSTACKVISGRIDDER_H_
#define APROJECTWSTACKVISGRIDDER_H_

#include <gridding/WStackVisGridder.h>

namespace conrad
{
  namespace synthesis
  {
    /// @brief Gridder that is appropriate for mosaicing. 
    ///
    /// @details The visibilities are gridded using a convolution
    /// function derived from the antenna illumination pattern, 
    /// appropriately shifted in position for each feed.
    ///
    /// To correct for the w term in the full synthesis measurement equation 
    /// the data are first partitioned in w and then gridded onto separate
    /// planes. At the end, all planes are Fourier transformed and stacked
    /// after multiplication by the w-dependent complex phasor image.
    ///
    /// The scaling is fast in data points, slow in w planes.
    ///
    /// @ingroup gridding
    class AProjectWStackVisGridder : public WStackVisGridder
    {
  public:

      /// @brief Construct antenna illumination pattern gridder
      /// @param diameter Antenna diameter (meters)
      /// @param blockage Antenna blockage (meters)
      /// @param wmax Maximum baseline (wavelengths)
      /// @param nwplanes Number of w planes
      /// @param overSample Oversampling (currently limited to <=1)
      /// @param maxSupport Maximum support to be allowed
      /// @param maxFeeds Maximum number of feeds allowed
      /// @param frequencyDependent Frequency dependent gridding?
      /// @param name Name of table to save convolution function into
      AProjectWStackVisGridder(const double diameter, const double blockage,
          const double wmax, const int nwplanes, const int overSample,
          const int maxSupport, const int maxFeeds=1,
          const bool frequencyDependent=true, const std::string& name=std::string(""));

      virtual ~AProjectWStackVisGridder();

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

      /// Correct for gridding convolution function
      /// @param image image to be corrected
      virtual void correctConvolution(casa::Array<double>& image);

  private:
      /// Reference frequency for illumination pattern. 
      double itsReferenceFrequency;
      /// Antenna diameter
      double itsDiameter;
      /// Antenna blockage
      double itsBlockage;
      /// Maximum number of feeds
      int itsMaxFeeds;
      /// Is the convolution function frequency dependent?
      bool itsFreqDep;
      /// Maximum support to test
      int itsMaxSupport;
      /// Mapping from row, pol, and channel to planes of convolution function
      casa::Cube<int> itsCMap;

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
