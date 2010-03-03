///
/// TableVisGridder: Table-based visibility gridder. This is an incomplete
/// class and cannot be used directly. Classes may be derived from this
/// and the unimplemented methods provided. In some cases, it may be 
/// necessary or more efficient to override the provided methods as well.
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
#ifndef TABLEVISGRIDDER_H_
#define TABLEVISGRIDDER_H_

// own includes
#include <gridding/IVisGridder.h>
#include <dataaccess/IDataAccessor.h>
#include <gridding/FrequencyMapper.h>

// std includes
#include <string>

// casa includes
#include <casa/BasicSL/Complex.h>

namespace askap
{
  namespace synthesis
  {
      

    /// @brief Incomplete base class for table-based gridding of visibility data.
    ///
    /// TVG supports gridding of visibility data onto one of a number of grids
    /// using one of a number of gridding functions. After gridding the final
    /// grid may be assembled by summing the grids appropriately. The separate
    /// grids may be for separate pointings, or for separate W planes. The summation
    /// process may include Fourier transformation.
    ///
    /// The main work in derived classes is to provide the convolution function
    /// and stacking operations.
    ///
    /// The sensitivity will vary as a function of position within the image.
    /// This is calculated by direct evaluation.
    ///
    /// @ingroup gridding
    class TableVisGridder : virtual public IVisGridder
    {
  public:

      /// @brief Standard two dimensional gridding using a convolution function
      /// in a table
      TableVisGridder();
      /// @brief Standard two dimensional gridding using a convolution function
      /// in a table
      /// @param[in] overSample Oversampling (currently limited to <=1)
      /// @param[in] support Support of function
      /// @param[in] padding padding factor (default is 1, i.e. no padding)
      /// @param name Name of table to save convolution function into
      TableVisGridder(const int overSample, const int support,
          const int padding = 1, 
          const std::string& name=std::string(""));

      /// @brief copy constructor
      /// @details it is required to decouple arrays between the input object
      /// and the copy.
      /// @param[in] other input object
      TableVisGridder(const TableVisGridder &other);
      
      virtual ~TableVisGridder();

      /// @brief Save to a table (for debugging)
      /// @param name Name of table
      void save(const std::string& name);

      /// @brief Initialise the gridding
      /// @param axes axes specifications
      /// @param shape Shape of output image: u,v,pol,chan
      /// @param dopsf Make the psf?
      virtual void initialiseGrid(const scimath::Axes& axes,
          const casa::IPosition& shape, const bool dopsf=true);

      /// @brief Grid the visibility data.
      /// @param acc const data accessor to work with
      /// @note a non-const adapter is created behind the scene. If no on-the-fly visibility 
      /// correction is performed, this adapter is equivalent to the original const data accessor
      virtual void grid(IConstDataAccessor& acc);
      
      /// Form the final output image
      /// @param out Output double precision image or PSF
      virtual void finaliseGrid(casa::Array<double>& out);

      /// @brief Calculate weights image
      /// @details Form the sum of the convolution function squared, 
      /// multiplied by the weights for each different convolution 
      /// function. This is used in the evaluation of the position
      /// dependent sensitivity
      /// @param out Output double precision sum of weights images
      virtual void finaliseWeights(casa::Array<double>& out);

      /// @brief Initialise the degridding
      /// @param axes axes specifications
      /// @param image Input image: cube: u,v,pol,chan
      virtual void initialiseDegrid(const scimath::Axes& axes,
          const casa::Array<double>& image);

      /// @brief Make context-dependant changes to the gridder behaviour
      /// @param context context
      virtual void customiseForContext(casa::String context);
      
      /// @brief assign weights
      /// @param viswt shared pointer to visibility weights
      virtual void initVisWeights(IVisWeights::ShPtr viswt);
      
      /// @brief Degrid the visibility data.
      /// @param[in] acc non-const data accessor to work with  
      virtual void degrid(IDataAccessor& acc);

      /// @brief Finalise
      virtual void finaliseDegrid();

      /// @brief return padding factor
      /// @return current padding factor
      int inline paddingFactor() const { return itsPaddingFactor;}

      /// @brief set padding factor
      /// @param[in] padding new padding factor
      void inline setPaddingFactor(const int padding) { itsPaddingFactor = padding;}

      /// @brief set or reset flag forcing gridder to use all data for PSF
      /// @details Change itsUseAllDataForPSF
      /// @param[in] useAll new value of the flag
      void inline useAllDataForPSF(const bool useAll) { itsUseAllDataForPSF = useAll;} 

      /// @brief set the largest angular separation between the pointing centre and the image centre
      /// @details If the threshold is positive, it is interpreted as the largest allowed angular
      /// separation between the beam (feed in the accessor terminology) pointing centre and the
      /// image centre. We need this to allow imaging of a subset of data (i.e. smaller field of view)
      /// and reject all pointings located outside this smaller image. All accessor rows with
      /// pointingDir1 separated from the image centre by more than this threshold will be ignored.
      /// If the threshold is negative (default), no data rejection based on the pointing direction is done.
      /// The gridder is initialised by default with the negative threshold, i.e. all data are used by default.
      /// @param[in] threshold largest allowed angular separation in radians, use negative value to select all data
      void inline maxPointingSeparation(double threshold = -1.) { itsMaxPointingSeparation = threshold; }
      
  protected:
      /// @brief shape of the grid
      /// @details The could be a number of grids indexed though gIndex (for each row, polarisation and channel). However, all should
      /// have exactly the same shape. 
      /// @return the shape of grid owned by this gridder
      inline const casa::IPosition& shape() const { return itsShape;}


      /// @brief correct visibilities, if necessary
      /// @details This method is intended for on-the-fly correction of visibilities (i.e. 
      /// facet-based correction needed for LOFAR). This method does nothing in this class, but
      /// can be overridden in the derived classes to plug some effect in. The same method is 
      /// used for both gridding and degridding, with the forward parameter used to distinguish
      /// between these two operations. A non-const accessor has to be modified in situ, if a
      /// correction is required. A buffer for read-only visibilities is created on-demand when
      /// rwVisibility method of the accessor is called for the first time.
      /// @param[in] acc non-const accessor with the data to correct, leave it intact if no
      /// correction is required
      /// @param[in] forward true for degridding (image to vis) and false for gridding (vis to image)
      virtual void correctVisibilities(IDataAccessor &acc, bool forward);
  
      /// @brief initialise sum of weights
      /// @details We keep track the number of times each convolution function is used per
      /// channel and polarisation (sum of weights). This method is made virtual to be able
      /// to do gridder specific initialisation without overriding initialiseGrid.
      /// This method accepts no parameters as itsShape, itsNWPlanes, etc should have already
      /// been initialised by the time this method is called.
      virtual void initialiseSumOfWeights();
      
      /// @brief helper method to initialise frequency mapping
      /// @details Derived gridders may override initialiseGrid and initialiseDegrid. Howerver, 
      /// they still need to be able to initialise frequency axis mapping (between accessor channels
      /// and image cube), which is handled by a private member class. This method initialises the 
      /// mapper using the content of itsShape and itsAxes, which should be set prior to calling this
      /// method.
      void initialiseFreqMapping();
      
      /// @brief helper method to set up cell size
      /// @details Similar action is required to calculate uv-cell size for gridding and degridding.
      /// Moreover, derived gridders may override initialiseGrid and initialiseDegrid and we don't want
      /// to duplicate the code up there. This method calculates uv-cell size for both ra and dec axes
      /// using coordinate information provided. This method also assigns passed axes parameter to itsAxes.
      /// @param[in] axes coordinate system (ra and dec axes are used).
      void initialiseCellSize(const scimath::Axes& axes);
      
      /// @brief gridder configured to calculate PSF?
      /// @details
      /// @return true if this gridder is configured to calculate PSF, false otherwise
      bool inline isPSFGridder() const { return itsDopsf; }
      
      /// @brief configure gridder to calculate PSF or residuals
      /// @details This method is expected to be called from overridden initialiseGrid method
      /// @param[in] dopsf if true, the gridder is configured to calculate PSF, otherwise
      /// a normal residual grid is calculated.
      void inline configureForPSF(bool dopsf) { itsDopsf = dopsf;}
        
      /// @brief obtain the centre of the image
      /// @details This method extracts RA and DEC axes from itsAxes and
      /// forms a direction measure corresponding to the middle of each axis.
      /// @return direction measure corresponding to the image centre
      casa::MVDirection getImageCentre() const;
      
      /// @brief obtain the tangent point
      /// @details For faceting all images should be constructed for the same tangent
      /// point. This method extracts the tangent point (reference position) from the
      /// coordinate system.
      /// @return direction measure corresponding to the tangent point
      casa::MVDirection getTangentPoint() const;
      
      // data members should be made private in the future!

      /// Axes definition for image
      askap::scimath::Axes itsAxes;

      /// Shape of image
      casa::IPosition itsShape;

      /// Cell sizes in wavelengths
      casa::Vector<double> itsUVCellSize;

      /// @brief Sum of weights (axes are index, pol, chan) 
      casa::Cube<double> itsSumWeights;

      /// @brief Convolution function
      /// The convolution function is stored as a vector of arrays so that we can
      /// use any of a number of functions. The index is calculated by cIndex.
      std::vector<casa::Matrix<casa::Complex> > itsConvFunc;

      /// Return the index into the convolution function for a given
      /// row, polarisation, and channel
      /// @param row Row of accessor
      /// @param pol Polarisation
      /// @param chan Channel
      virtual int cIndex(int row, int pol, int chan);

      /// Return the index into the grid for a given
      /// row and channel
      /// @param row Row of accessor
      /// @param pol Polarisation
      /// @param chan Channel
      virtual int gIndex(int row, int pol, int chan);

      /// @brief Initialize the convolution function - this is the key function to override.
      /// @param[in] acc const accessor to work with
      virtual void initConvolutionFunction(const IConstDataAccessor& acc) = 0;

      /// @brief Initialise the indices
      /// @param[in] acc const accessor to work with
      virtual void initIndices(const IConstDataAccessor& acc) = 0;

      /// @brief Correct for gridding convolution function
      /// @param image image to be corrected
      virtual void correctConvolution(casa::Array<double>& image) = 0;

      /// @brief Conversion helper function
      /// @details Copies in to out expanding double into complex values and
      /// padding appropriately if necessary (padding is more than 1)
      /// @param[out] out complex output array
      /// @param[in] in double input array
      /// @param[in] padding padding factor
      static void toComplex(casa::Array<casa::Complex>& out, const casa::Array<double>& in, 
                     const int padding = 1);

      /// @brief Conversion helper function
      /// @details Copies real part of in into double array and
      /// extracting an inner rectangle if necessary (padding is more than 1)
      /// @param[out] out real output array
      /// @param[in] in complex input array      
      /// @param[in] padding padding factor
      static void toDouble(casa::Array<double>& out, const casa::Array<casa::Complex>& in,
                    const int padding = 1);

      /// @brief a helper method to initialize gridding of the PSF
      /// @details The PSF is calculated using the data for a
      /// representative field/feed only. By default, the first encountered
      /// feed/field is chosen. If the same gridder is reused for another
      /// sequence of data points a new representative feed/field have to be
      /// found. This is done by resetting the cache in initialiseGrid. However,
      /// the latter method can be overridden in the derived classes. To avoid
      /// a duplication of the code, this helper method resets the representative
      /// feed/field cache. It is called from initialiseGrid.
      void initRepresentativeFieldAndFeed();
      
      /// @brief set up itsStokes using the information from itsAxes and itsShape
      void initStokes();
      
      /// @brief obtain stokes for each plane of the current grid
      /// @details The output of this method has a meaning only after initialiseGrid or
      /// initialiseDegrid has been called.
      inline const casa::Vector<casa::Stokes::StokesTypes>& getStokes() const {return itsStokes;}

      /// Support of convolution function
      int itsSupport;
      /// Oversampling of convolution function
      int itsOverSample;
      /// Name of table to save to
      std::string itsName;

      /// Is the model empty? Used to shortcut degridding
      bool itsModelIsEmpty;

      /// The grid is stored as a cube as well so we can index into that as well.
      std::vector<casa::Array<casa::Complex> > itsGrid;
            
  private:
      /// @brief polarisation frame for the grid
      /// @details Assumed to be the same for all elements of itsGrid vector.
      /// This field is filled in initialiseGrid or initialiseDegrid using the Axes 
      /// object.
      casa::Vector<casa::Stokes::StokesTypes> itsStokes;
  
      /// Number of samples gridded
      double itsSamplesGridded;
      /// Number of samples degridded
      double itsSamplesDegridded;
      /// Number of flagged visibility vectors (all pols.)
      double itsVectorsFlagged;
      /// Number of grid cells gridded
      double itsNumberGridded;
      /// Number of grid cells degridded
      double itsNumberDegridded;
      /// Time for Coordinates
      double itsTimeCoordinates;
      /// Time for convolution functions
      double itsTimeConvFunctions;
      /// Time for gridding
      double itsTimeGridded;
      /// Time for degridding
      double itsTimeDegridded;

      /// @brief is this gridder a PSF gridder?
      bool itsDopsf;
      
      /// @brief internal padding factor, 1 by default
      int itsPaddingFactor;
  
      /// Generic grid/degrid - this is the heart of this framework. It should never
      /// need to be overridden
      /// @param[in] acc non-const data accessor to work with.  
      /// @param[in] forward true for the model to visibility transform (degridding),
      /// false for the visibility to dirty image transform (gridding)
      /// @note We have to pass a non-const accessor because this method can either 
      /// write or read. A bit better re-structuring of the code can help to deal with
      /// constness properly.
      void generic(IDataAccessor& acc, bool forward);

      /// Visibility Weights
      IVisWeights::ShPtr itsVisWeight;

      /// @brief true if no visibilities have been gridded since the last initialize
      /// @details For PSF calculations we need to take just the first feed and 
      /// field (it is an approximation that they all considered the same). To be
      /// able to extend this check over multiple calls of the generic routine this
      /// flag is used. It is set to true in initialise and then reset to false when
      /// the first visibility is gridded. 
      bool itsFirstGriddedVis;

      /// @brief an index of the feed, which provides data for the PSF calculations
      /// @details This data member is initialized when the first visibility is gridded,
      /// only this feed is used to calculate the PSF.
      casa::uInt itsFeedUsedForPSF;

      /// @brief pointing direction, which provides data for the PSF calculations
      /// @details This data member is initialized when the first visibility is gridded,
      /// only this field is used to calculate the PSF
      casa::MVDirection itsPointingUsedForPSF;
      
      /// @brief use all data for PSF calculation
      /// @details By default we use just a representative feed and field to calculate PSF. 
      /// For research purposes we need an option which allows to take all available data into
      /// account. This is a flag showing that itsFeedUsedForPSF and itsPointingUsedForPSF are ignored.
      /// Default value is false.
      bool itsUseAllDataForPSF;     
      
      /// @brief mapping class between image planes and accessor channels
      /// @details Correspondence between planes of the image cube and accessor channels may be
      /// non-trivial. This class takes care of the mapping.
      FrequencyMapper itsFreqMapper;
      
      /// @brief largest angular separation between the pointing centre and the image centre
      /// @details If the value is positive, it is interpreted as the largest allowed angular
      /// separation between the beam (feed in the accessor terminology) pointing centre and the
      /// image centre. It is intended to allow imaging of a subset of data (i.e. smaller field of view)
      /// and reject all pointings located outside this smaller image. All accessor rows with
      /// pointingDir1 separated from the image centre by more than this threshold are ignored.
      /// If the value is negative, no data rejection based on the pointing direction is done.
      /// Values are in radians.
      double itsMaxPointingSeparation;
      
      /// @brief number of rows rejected due to itsMaxPointingSeparation
      /// @details Accumulated to get proper statistics/debugging info.
      long itsRowsRejectedDueToMaxPointingSeparation;
    };
  }
}
#endif
