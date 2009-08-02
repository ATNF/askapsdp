/// @file
///
/// AProjectWStackVisGridder: Grids visibility data using the self-convolution of 
/// the antenna illumination pattern.
///
/// @copyright (c) 2007 CSIRO
/// Australia Telescope National Facility (ATNF)
/// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
/// PO Box 76, Epping NSW 1710, Australia
/// atnf-enquiries@csiro.au
///
/// This file is part of the ASKAP software distribution.
///
/// The ASKAP software distribution is free software: you can redistribute it
/// and/or modify it under the terms of the GNU General Public License as
/// published by the Free Software Foundation; either version 2 of the License,
/// or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
///
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef APROJECTWSTACKVISGRIDDER_H_
#define APROJECTWSTACKVISGRIDDER_H_

#include <gridding/WStackVisGridder.h>
#include <gridding/IBasicIllumination.h>
#include <dataaccess/IConstDataAccessor.h>
#include <gridding/UVPattern.h>

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
      /// @param limitSupport Upper limit of support
      /// @param maxFeeds Maximum number of feeds allowed
      /// @param maxFields Maximum number of fields allowed
      /// @param maxAnts Maximum number of antennas allowed
      /// @param pointingTol Pointing tolerance in radians
      /// @param paTol Parallactic angle tolerance in radians
      /// @param frequencyDependent Frequency dependent gridding?
      /// @param name Name of table to save convolution function into
      AProjectWStackVisGridder(const boost::shared_ptr<IBasicIllumination const> &illum,
          const double wmax, const int nwplanes, const int overSample,
          const int maxSupport, const int limitSupport,
			       const int maxFeeds=1, const int maxFields=1, const int maxAnts=36,
			       const double pointingTol=0.0001,
			       const double paTol=0.01,
			       const bool frequencyDependent=true, 
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

      /// @brief Initialise the gridding
      /// @param axes axes specifications
      /// @param shape Shape of output image: u,v,pol,chan
      /// @param dopsf Make the psf?
      virtual void initialiseGrid(const scimath::Axes& axes,
              const casa::IPosition& shape, const bool dopsf=true);
   
      /// @brief Initialise the degridding
      /// @param axes axes specifications
      /// @param image Input image: cube: u,v,pol,chan
      virtual void initialiseDegrid(const scimath::Axes& axes,
              const casa::Array<double>& image);

  protected:
      /// @brief initialise sum of weights
      /// @details We keep track the number of times each convolution function is used per
      /// channel and polarisation (sum of weights). This method is made virtual to be able
      /// to do gridder specific initialisation without overriding initialiseGrid.
      /// This method accepts no parameters as itsShape, itsNWPlanes, etc should have already
      /// been initialised by the time this method is called.
      virtual void initialiseSumOfWeights();
  
      /// @brief Initialise the indices
      /// @param[in] acc const accessor to work with
      virtual void initIndices(const IConstDataAccessor& acc);

      /// Index into convolution function
      /// @param row Row number
      /// @param pol Polarization id
      /// @param chan Channel number
      virtual int cIndex(int row, int pol, int chan);

      /// Initialize convolution function
      /// @param[in] acc const accessor to work with
      virtual void initConvolutionFunction(const IConstDataAccessor& acc);

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
      /// Maximum number of antennas
      int itsMaxAnts;
      /// Pointing tolerance in radians
      double itsPointingTolerance;
      /// @brief parallactic angle tolerance (in radians)
      /// @details If new angle is different from the one used to compute the cache for more
      /// than this value, the cache will be recomputed. Note, a negative value means to 
      /// always recalculate for asymmetric illumination patterns
      double itsParallacticAngleTolerance;
      /// Last field processed
      int itsLastField;
      /// Current field processed
      int itsCurrentField;
      /// Is the convolution function frequency dependent?
      bool itsFreqDep;
      /// Maximum support to test
      int itsMaxSupport;
      /// Upper limit of support
      int itsLimitSupport;
      /// Mapping from row, pol, and channel to planes of convolution function
      casa::Cube<int> itsCMap;
      /// Cube of slopes
      casa::Cube<double> itsSlopes;
      /// Has this feed and field been filled in?
      casa::Matrix<bool> itsDone;
      /// Pointing for this feed and field
      casa::Matrix<casa::MVDirection> itsPointings;
      /// @brief buffer in the uv-space
      /// @details It is used to compute convolution functions (buffer for illumination pattern)
      boost::shared_ptr<UVPattern> itsPattern;
      
      /// @brief true if the CF cache is valid
      /// @details The shape of the accessor and mapping may change from one iteration to another. 
      /// We check (inside initIndices) whether the indices (and therefore the cache of CFs) stay 
      /// the same and reset this flag if they are not. This flag is also reset when
      /// gridding or degridding are initialised.
      bool itsIndicesValid;
      
      // stats for CF cache rebuilds
      /// @brief number of iterations when CFs were generated
      /// @details This number is incremented for each accessor which leads to recomputation of the 
      /// CF cache. In the best case (CFs computed once and
      /// reused later) it should be equal to 1. In the worst case (CFs are
      /// recomputed for every iteration) this should be equal to the number iterations 
      /// (to be exact, the number of calls to initConvolutionFunctions).
      casa::uInt itsNumberOfCFGenerations;
      
      /// @brief total number of iterations
      /// @details This number is incremented each time a new accessor is passed to this gridder
      /// (i.e. counts every iteration). 
      casa::uInt itsNumberOfIterations;
      
      /// @brief number of iterations when CFs were generated due to parallactic angle change
      /// @details This number is incremented for each accessor which leads to recomputation of the
      /// CF cache due to a change in parallactic angle. This value is always less than 
      /// itsNumberOfCFGenerations.
      casa::uInt itsNumberOfCFGenerationsDueToPA;
      
      /// @brief parallactic angles for which the cache is valid
      /// @details This buffer is only used and filled if the illumination pattern is asymmetric.
      /// We currently don't account for the VLBI case with notably different parallactic angles.
      /// Therefore, only one angle is stored here. 
      casa::Vector<casa::Float> itsCFParallacticAngles;
      
    };

  }
}
#endif
