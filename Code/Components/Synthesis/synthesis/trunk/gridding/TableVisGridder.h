/// @file
///
/// TableVisGridder: Table-based visibility gridder. This is an incomplete
/// class and cannot be used directly. Classes may be derived from this
/// and the unimplemented methods provided. In some cases, it may be 
/// necessary or more efficient to override the provided methods as well.
///
/// The main work in derived classes is to provide the convolution function.
///
/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef TABLEVISGRIDDER_H_
#define TABLEVISGRIDDER_H_

#include <gridding/IVisGridder.h>

namespace conrad
{
  namespace synthesis
  {

    /// Incomplete base class for table-based gridding of visibility data.
    ///
    /// @todo Implement factory for gridders
    class TableVisGridder : public IVisGridder
    {
      public:

/// @brief Standard two dimensional gridding using a convolution function
/// in a table
        TableVisGridder();

        virtual ~TableVisGridder();

/// @brief Grid the visibility data onto the grid using multifrequency
/// synthesis. Note that the weights allow complete flexibility
/// @param idi DataIterator
/// @param axes axes specifications
/// @param grid Output grid: cube: u,v,pol
/// @param weights Output weights: vector: pol
        virtual void reverse(IDataSharedIter& idi,
          const conrad::scimath::Axes& axes,
          casa::Cube<casa::Complex>& grid,
          casa::Vector<double>& weights);

/// @brief Grid the spectral visibility data onto the grid
/// Note that the weights allow complete flexibility
/// @param idi DataIterator
/// @param axes axes specifications
/// @param grid Output grid: cube: u,v,chan,pol
/// @param weights Output weights: vector: pol
        virtual void reverse(IDataSharedIter& idi,
          const conrad::scimath::Axes& axes,
          casa::Array<casa::Complex>& grid,
          casa::Matrix<double>& weights);

/// @brief Estimate visibility data from the grid using multifrequency
/// synthesis.
/// @param idi DataIterator
/// @param axes axes specifications
/// @param grid Input grid: cube: u,v,pol
        virtual void forward(IDataSharedIter& idi,
          const conrad::scimath::Axes& axes,
          const casa::Cube<casa::Complex>& grid);

/// @brief Estimate spectral visibility data from the grid
/// @param idi DataIterator
/// @param axes axes specifications
/// @param grid Output weights: cube of same shape as visibility
        virtual void forward(IDataSharedIter& idi,
          const conrad::scimath::Axes& axes,
          const casa::Array<casa::Complex>& grid);

/// @brief Correct for gridding convolution function
/// @param axes axes specifications
/// @param image image to be corrected
        virtual void correctConvolution(const conrad::scimath::Axes& axes,
          casa::Cube<double>& image) = 0;

/// @brief Apply gridding convolution function
/// @param axes axes specifications
/// @param image image to be corrected
        virtual void applyConvolution(const conrad::scimath::Axes& axes,
          casa::Cube<double>& image) = 0;

      protected:

        /// @brief Convolution function
        ///
        /// The convolution function is stored as a cube so that we can use the third axes
        /// for data dependent variations e.g. w projection. The function cOffset can be
        /// used to generate this offset.
        casa::Cube<float> itsC;
        
        /// Return the offset into the convolution function for a given
        /// row and channel
        virtual int cOffset(int, int)=0;

        /// Support of convolution function
        int itsSupport;
        /// Oversampling of convolution function
        int itsOverSample;
        /// Size of convolution function on first two axes (square)
        int itsCSize;
        /// Center of convolution functio
        int itsCCenter;

/// @brief Are the convolution functions constant in meters?
///
/// If !itsInM these functions assume that the convolution function
/// is specified in wavelengths. This is not always the case e.g. for antenna illumination
/// pattern gridding. In that case, set itsInM to true.
        bool itsInM;

/// Initialize the convolution function - this is the key function to override.
/// This should also setup cOffset to get the correct value of the offset
/// fo every row and channel.
///
/// @param idi Data iterator
/// @param cellsize cellsize in wavelengths
/// @param shape grid shape
        virtual void initConvolutionFunction(IDataSharedIter& idi, 
          const casa::Vector<double>& cellsize,
          const casa::IPosition& shape)=0;

/// Find the cellsize from the image shape and axis definitions
/// @param cellsize cellsize in wavelengths
/// @param shape grid shape
/// @param axes Axes definition
        void findCellsize(casa::Vector<double>& cellsize, const casa::IPosition& shape,
          const conrad::scimath::Axes& axes);

/// Functions to do the real work. We may need to override these for derived classes so we
/// make them virtual and protected.

/// Visibility to image for a cube (MFS)
/// @param uvw UVW in meters
/// @param visibility Visibility
/// @param visweight Visibility weight
/// @param freq Frequency
/// @param cellsize Cellsize in wavelengths
/// @param grid Grid for data
/// @param sumwt Total summed weight per polarization 
        virtual void genericReverse(const casa::Vector<casa::RigidVector<double, 3> >& uvw,
          const casa::Cube<casa::Complex>& visibility,
          const casa::Cube<float>& visweight,
          const casa::Vector<double>& freq,
          const casa::Vector<double>& cellsize,
          casa::Cube<casa::Complex>& grid,
          casa::Vector<double>& sumwt);


/// Image to visibility for a cube (MFS))
/// @param uvw UVW in meters
/// @param visibility Visibility
/// @param visweight Visibility weight
/// @param freq Frequency
/// @param cellsize Cellsize in wavelengths
/// @param grid Grid for data
        virtual void genericForward(const casa::Vector<casa::RigidVector<double, 3> >& uvw,
          casa::Cube<casa::Complex>& visibility,
          casa::Cube<float>& visweight,
          const casa::Vector<double>& freq,
          const casa::Vector<double>& cellsize,
          const casa::Cube<casa::Complex>& grid);

/// Visibility to image for an array (spectral line)
/// @param uvw UVW in meters
/// @param visibility Visibility
/// @param visweight Visibility weight
/// @param freq Frequency
/// @param cellsize Cellsize in wavelengths
/// @param grid Grid for data
/// @param sumwt Total summed weight per polarization and channel
        virtual void genericReverse(const casa::Vector<casa::RigidVector<double, 3> >& uvw,
          const casa::Cube<casa::Complex>& visibility,
          const casa::Cube<float>& visweight,
          const casa::Vector<double>& freq,
          const casa::Vector<double>& cellsize,
          casa::Array<casa::Complex>& grid,
          casa::Matrix<double>& sumwt);


/// Image to visibility for an array (spectral line)
/// @param uvw UVW in meters
/// @param visibility Visibility
/// @param visweight Visibility weight
/// @param freq Frequency
/// @param cellsize Cellsize in wavelengths
/// @param grid Grid for data
        void genericForward(const casa::Vector<casa::RigidVector<double, 3> >& uvw,
          casa::Cube<casa::Complex>& visibility,
          casa::Cube<float>& visweight,
          const casa::Vector<double>& freq,
          const casa::Vector<double>& cellsize,
          const casa::Array<casa::Complex>& grid);
          ///
        /// Round to nearest integer
        /// @param x Value to be rounded
        static int nint(double x) {
          return x>0? int(x+0.5) : int(x-0.5);
        }

    };

  }
}
#endif
