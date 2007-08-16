/// @file
///
/// BoxVisGridder: Box-based visibility gridder.
///
/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef BOXVISGRIDDER_H_
#define BOXVISGRIDDER_H_

#include <gridding/TableVisGridder.h>

namespace conrad
{
	namespace synthesis
	{
		/// @brief Minimal box-car convolution (aka nearest neighbour) gridder.
		///
		/// @details It doesn't work well but it is fast and simple.
		/// @ingroup gridding
		class BoxVisGridder : public TableVisGridder
		{
			public:

				// Standard two dimensional box gridding
				BoxVisGridder();

				virtual ~BoxVisGridder();

				/// @brief Correct for gridding convolution function
				/// @param axes axes specifications
				/// @param image image to be corrected
				virtual void correctConvolution(const scimath::Axes& axes,
				    casa::Cube<double>& image)
				{
				}

				/// @brief Apply gridding convolution function
				/// @param axes axes specifications
				/// @param image image to be corrected
				virtual void applyConvolution(const scimath::Axes& axes,
				    casa::Cube<double>& image)
				{
				}

			protected:
				/// Offset into convolution function
				/// @param row Row number
				/// @param chan Channel number
				virtual int cOffset(int row, int chan);
				/// Initialize convolution function
				/// @param idi Data access iterator
				/// @param axes axes specifications
				/// @param uvw Input uvw (may be rotated so we cannot use the iterator version)
				/// @param cellSize Cell size in wavelengths
				/// @param shape Shape of grid
				virtual void initConvolutionFunction(IDataSharedIter& idi,
				    const conrad::scimath::Axes& axes,
				    casa::Vector<casa::RigidVector<double, 3> >& uvw,
				    const casa::Vector<double>& cellSize, const casa::IPosition& shape);
		};

	}
}
#endif
