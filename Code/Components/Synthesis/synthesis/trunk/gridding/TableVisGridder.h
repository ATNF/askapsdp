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

// std includes
#include <string>

// casa includes
#include <casa/BasicSL/Complex.h>

namespace askap
{
  namespace synthesis
  {
  
    /// @brief a helper method to print directions nicely
    /// @details By default an instance of casa::MVDirection is printed
    /// as 3 direction cosines. It is not very convenient. This method
    /// allows to print it in a more log-reader-friendly way. We can move this
    /// method to a higher level if (when) it becomes necessary in other places.
    /// I (MV) didn't move it do askap just because it would introduce a
    /// dependency on casacore. Although scimath may be a better place for this
    /// method, which would solve the problem.
    /// @param[in] dir MVDirection object to print
    /// @return a string containing a nice representation of the direction
    std::string printDirection(const casa::MVDirection &dir);
    

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
    class TableVisGridder : public IVisGridder
    {
  public:

      /// @brief Standard two dimensional gridding using a convolution function
      /// in a table
      TableVisGridder();
      /// @brief Standard two dimensional gridding using a convolution function
      /// in a table
      /// @param overSample Oversampling (currently limited to <=1)
      /// @param support Support of function
      /// @param name Name of table to save convolution function into
      TableVisGridder(const int overSample, const int support,
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

      /// Grid the visibility data.
      /// @param idi DataIterator
      virtual void grid(IDataSharedIter& idi);

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
      
      /// Degrid the visibility data.
      /// @param idi DataIterator
      virtual void degrid(IDataSharedIter& idi);

      /// @brief Finalise
      virtual void finaliseDegrid();

  protected:
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
      /// point. We currently pass its coordinates in two specialised axes RA-TANGENT and
      /// DEC-TANGENT. If these axes are present a proper MVDirection quantity is 
      /// extracted and returned, otherwise this method does the same as getImageCentre()
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

      /// Support of convolution function
      int itsSupport;
      /// Oversampling of convolution function
      int itsOverSample;
      /// Size of convolution function on first two axes (square)
      int itsCSize;
      /// Center of convolution function
      int itsCCenter;
      /// Name of table to save to
      std::string itsName;

      /// Is the model empty? Used to shortcut degridding
      bool itsModelIsEmpty;

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
      /// Time for gridding
      double itsTimeGridded;
      /// Time for degridding
      double itsTimeDegridded;

      /// The grid is stored as a cube as well so we can index into that as well.
      std::vector<casa::Array<casa::Complex> > itsGrid;
      
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

      /// Conversion helper function
      static void toComplex(casa::Array<casa::Complex>& out,
          const casa::Array<double>& in);

      /// Conversion helper function
      static void toDouble(casa::Array<double>& out,
          const casa::Array<casa::Complex>& in);

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
      
  private:
      /// @brief is this gridder a PSF gridder?
      bool itsDopsf;
  
      /// Generic grid/degrid - this is the heart of this framework. It should never
      /// need to be overridden
      /// @param[in] acc non-const data accessor to work with.  
      /// @param[in] forward true for the model to visibility transform (degridding),
      /// false for the visibility to dirty image transform (gridding)
      /// @note We have to pass a non-const accessor because this method can either 
      /// write or read. A bit better re-structuring of the code can help to deal with
      /// constness properly.
      void generic(IDataAccessor& acc, bool forward);

      /// Find the cellsize from the image shape and axis definitions
      /// @param cellsize cellsize in wavelengths
      void findCellsize(casa::Vector<double>& cellsize);

      /// @brief Find the change in delay required
      /// @details
      /// @param[in] acc data accessor to take the input data from
      /// @param[out] outUVW Rotated uvw
      /// @param[out] delay Delay change (m)
      /// @note output vectors are resized to match the accessor's number of rows
      void rotateUVW(const IConstDataAccessor& acc,
          casa::Vector<casa::RigidVector<double, 3> >& outUVW,
          casa::Vector<double>& delay) const;

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
    };
  }
}
#endif
