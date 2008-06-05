/// @file
///

/// (c) 2007 ASKAP, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef SPHVISGRIDDER_H_
#define SPHVISGRIDDER_H_

#include <gridding/TableVisGridder.h>
#include <dataaccess/IConstDataAccessor.h>

namespace askap
{
	namespace synthesis
	{

		/// @brief SphFuncVisGridder: Spheroidal function-based visibility gridder.
		/// @details The gridding function is a prolate spheroidal function identical to the
		/// one used in AIPS, AIPS++, and probably other packages. At some point
		/// we should revisit the tradeoffs since the choice to use this was made
		/// about twenty years ago and computers are quite different now.
		///
		/// The spheroidal function has m = 6, alpha = 1 using the rational
		/// approximations discussed by fred schwab in 'indirect imaging'.
		/// The gridding function is (1-nu**2)*grdsf(nu) where nu is the distance
		/// to the edge. the grid correction function is just 1/grdsf(nu) where nu
		/// is now the distance to the edge of the image.
		class SphFuncVisGridder : public TableVisGridder
		{
			public:

				/// @brief Standard two dimensional gridding
				SphFuncVisGridder();

				virtual ~SphFuncVisGridder();

				/// Clone a copy of this Gridder
				virtual IVisGridder::ShPtr clone();

			protected:
				/// @brief Initialize the convolution function
				/// @param[in] acc const data accessor to work with
				virtual void initConvolutionFunction(const IConstDataAccessor& acc);

				/// @brief Initialise the indices
				/// @param[in] acc const data accessor to work with
				virtual void initIndices(const IConstDataAccessor& acc);

				/// Correct for gridding convolution function
				/// @param image image to be corrected
				virtual void correctConvolution(casa::Array<double>& image);

				/// Calculate prolate spheroidal function
				/// @param nu Argument for spheroidal function
				double grdsf(double nu);

				/// Initialize lookup table for spheriodal function
				void initSphFunc();

		};

	}
}
#endif
