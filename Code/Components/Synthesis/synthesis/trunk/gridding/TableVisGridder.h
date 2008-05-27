///
/// TableVisGridder: Table-based visibility gridder. This is an incomplete
/// class and cannot be used directly. Classes may be derived from this
/// and the unimplemented methods provided. In some cases, it may be 
/// necessary or more efficient to override the provided methods as well.
///
/// (c) 2007 ASKAP, All Rights Reserved.
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
      /// @param out Output double precision image
      virtual void finaliseGrid(casa::Array<double>& out);

      /// Form the final output image
      /// @param out Output double precision PSF
      virtual void finalisePSF(casa::Array<double>& out);

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
      /// @param xxx xxx
      virtual void customiseForContext(casa::String context);
      virtual void initVisWeights(IVisWeights::ShPtr viswt);
      
      /// Degrid the visibility data.
      /// @param idi DataIterator
      virtual void degrid(IDataSharedIter& idi);

      /// @brief Finalise
      virtual void finaliseDegrid();

  protected:
  
      /// @brief obtain the centre of the image
      /// @details This method extracts RA and DEC axes from itsAxes and
      /// forms a direction measure corresponding to the middle of each axis.
      /// @return direction measure corresponding to the image centre
      casa::MVDirection getImageCentre() const;
      
      // data members should be made private in the future!

      /// Axes definition for image
      askap::scimath::Axes itsAxes;

      /// Shape of image
      casa::IPosition itsShape;

      /// Do we want a PSF?
      bool itsDopsf;

      /// Cell sizes in wavelengths
      casa::Vector<double> itsUVCellSize;

      /// @brief Sum of weights (axes are index, pol, chan) 
      casa::Cube<casa::Complex> itsSumWeights;

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
      /// Grid for PSF
      std::vector<casa::Array<casa::Complex> > itsGridPSF;

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

  private:
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
