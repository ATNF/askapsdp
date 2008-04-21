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
#include <dataaccess/IConstDataAccessor.h>

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

      /// Degrid the visibility data.
      /// @param idi DataIterator
      virtual void degrid(IDataSharedIter& idi);

      /// @brief Finalise
      virtual void finaliseDegrid();

  protected:

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
      /// @param idi Data iterator
      virtual void initConvolutionFunction(IDataSharedIter& idi) = 0;

      /// @brief Initialise the indices
      virtual void initIndices(IDataSharedIter& idi) = 0;

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
      void generic(IDataSharedIter& idi, bool forward);

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

    };
  }
}
#endif
