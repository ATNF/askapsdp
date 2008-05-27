/// @file
///
/// BoxVisGridder: Box-based visibility gridder.
///
/// (c) 2007 ASKAP, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef BOXVISGRIDDER_H_
#define BOXVISGRIDDER_H_

#include <gridding/TableVisGridder.h>
#include <dataaccess/IConstDataAccessor.h>

namespace askap
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

				/// Clone a copy of this Gridder
				virtual IVisGridder::ShPtr clone();

				virtual ~BoxVisGridder();

				/// @brief Initialise the indices
				/// @param[in] acc const data accessor to work with
				virtual void initIndices(const IConstDataAccessor& acc);

				/// @brief Correct for gridding convolution function
				/// @param image image to be corrected
				virtual void correctConvolution(casa::Array<double>& image);
				
			protected:
				/// Initialize convolution function
				/// @param idi Data access iterator
				virtual void initConvolutionFunction(IDataSharedIter& idi);
		};

	}
}
#endif
