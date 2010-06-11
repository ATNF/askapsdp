/// @file
/// @brief A method to set up converters and selectors from parset file
/// @details Parameters are currently passed around using parset files.
/// The methods declared in this file set up converters and selectors
/// from the ParameterSet object. This is probably a temporary solution.
/// This code can eventually become a part of some class (e.g. a DataSource
/// which returns selectors and converters with the defaults alread
/// applied according to the parset file).
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
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef ME_PARSET_INTERFACE_H
#define ME_PARSET_INTERFACE_H

#include <vector>
#include <string>

#include <casa/aips.h>
#include <casa/Quanta.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MEpoch.h>

// own includes
#include <fitting/Params.h>
#include <fitting/Solver.h>
#include <Common/ParameterSet.h>

namespace askap
{

	namespace synthesis
	{


		/// @brief set up solver according to the parset file
		/// @param[in] solver Pointer to solver to be created
		/// @param[in] parset a parset object to read the parameters from
		/// @ingroup measurementequation
		void operator<<(askap::scimath::Solver::ShPtr& solver, const LOFAR::ParameterSet &parset);

		/// @brief Conversion functions for parset entries
		class MEParsetInterface
		{
			public:
				/// @brief Interpret string as an integer
				/// @param s String to be interpreted
			static int asInteger(const std::string& s);
			
			/// @brief Interpret string as a quantity
			/// @param s String to be interpreted
			static casa::Quantity asQuantity(const std::string& s);

			/// @brief Interpret string as an MEpoch
			/// @param epoch String to be interpreted
			static casa::MEpoch asMEpoch(const std::vector<std::string>& epoch);

			/// @brief Interpret string as an MDirection
			/// @param direction String to be interpreted 
			static casa::MDirection asMDirection(const std::vector<std::string>& direction);

			/// @brief Interpret string as an MPosition
			/// @param position String to be interpreted 
			static casa::MPosition asMPosition(const std::vector<std::string>& position);
		};

	} // namespace synthesis

} // namespace askap


#endif
