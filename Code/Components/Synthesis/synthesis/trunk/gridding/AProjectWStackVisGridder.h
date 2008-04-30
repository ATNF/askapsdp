/// @file
///
/// AProjectWStackVisGridder: Grids visibility data using the self-convolution of 
/// the antenna illumination pattern.
///
/// (c) 2007 ASKAP, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef APROJECTWSTACKVISGRIDDER_H_
#define APROJECTWSTACKVISGRIDDER_H_

#include <gridding/WStackVisGridder.h>
#include <gridding/IBasicIllumination.h>

#include <boost/shared_ptr.hpp>

namespace askap
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
      /// @param illum Antenna illumination model
      /// @param wmax Maximum baseline (wavelengths)
      /// @param nwplanes Number of w planes
      /// @param overSample Oversampling (currently limited to <=1)
      /// @param maxSupport Maximum support to be allowed
      /// @param maxFeeds Maximum number of feeds allowed
      /// @param maxFields Maximum number of fields allowed
      /// @param pointingTol Pointing tolerance in radians
      /// @param frequencyDependent Frequency dependent gridding?
      /// @param name Name of table to save convolution function into
      AProjectWStackVisGridder(const boost::shared_ptr<IBasicIllumination const> &illum,
          const double wmax, const int nwplanes, const int overSample,
          const int maxSupport, const int maxFeeds=1, const int maxFields=1,
          const double pointingTol=0.0001, const bool frequencyDependent=true, 
          const std::string& name=std::string(""));

      /// @brief copy constructor
      /// @details It is required to decouple internal arrays between input object
      /// and this copy.
      /// @param[in] other input object
      AProjectWStackVisGridder(const AProjectWStackVisGridder &other);
    
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
      /// Antenna illumination model
      boost::shared_ptr<IBasicIllumination const> itsIllumination;
      /// Maximum number of feeds
      int itsMaxFeeds;
      /// Maximum number of fields
      int itsMaxFields;
      /// Pointing tolerance in radians
      double itsPointingTolerance;
      /// Last field processed
      int itsLastField;
      /// Current field processed
      int itsCurrentField;
      /// Is the convolution function frequency dependent?
      bool itsFreqDep;
      /// Maximum support to test
      int itsMaxSupport;
      /// Mapping from row, pol, and channel to planes of convolution function
      casa::Cube<int> itsCMap;
      /// Cube of slopes
      casa::Cube<double> itsSlopes;
      /// Has this feed and field been filled in?
      casa::Matrix<bool> itsDone;
      /// Pointing for this feed and field
      casa::Matrix<casa::MVDirection> itsPointings;

      /// Pad up in size using FFT
      /// @param in Input Array
      /// @param out Output Array
      void fftPad(const casa::Array<double>& in, casa::Array<double>& out);
    };

  }
}
#endif
