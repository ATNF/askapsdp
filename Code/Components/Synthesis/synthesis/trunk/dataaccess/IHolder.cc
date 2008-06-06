/// @file
/// @brief A base class for classes holding something
/// @details This class is a structural item used as a base class for
/// interfaces to various data holders (i.e. table holder, derived
/// information holder, subtable data holder, etc). There is no need
/// for any methods here. It just has an empty virtual destructor to
/// avoid specifying it for a number of the next level classes.
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

#include <dataaccess/IHolder.h>

namespace askap {

namespace synthesis {

/// void virtual destructor to keep the compiler happy
IHolder::~IHolder()
{
}

} // namespace synthesis

} // namespace askap
