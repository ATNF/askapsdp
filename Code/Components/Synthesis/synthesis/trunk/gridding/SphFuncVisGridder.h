/// @file
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
				
				/// @brief static method to create gridder
			    /// @details Each gridder should have a static factory method, which is
			    /// able to create a particular type of the gridder and initialise it with
			    /// the parameters taken form the given parset. It is assumed that the 
			    /// method receives a subset of parameters where the gridder name is already
			    /// taken out. 
			    /// @param[in] parset input parset file
			    /// @return a shared pointer to the gridder instance
			    static IVisGridder::ShPtr createGridder(const LOFAR::ParameterSet& parset);
				

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
