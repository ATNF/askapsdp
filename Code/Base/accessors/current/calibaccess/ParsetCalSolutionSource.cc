/// @file
/// @brief Parset file-based implementation of the calibration solution source
/// @details This implementation is to be used with pre-existing code writing/reading
/// the parset directly and with a number of tests. It is just to convert the legacy code.
/// There is only one implementation of this class which is used for both reading and writing.
/// Main functionality is implemented in the corresponding ParsetCalSolutionAccessor class.
/// This class just creates an instance of the accessor and manages it.
///
/// @copyright (c) 2011 CSIRO
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
/// @author Max Voronkov <Maxim.Voronkov@csiro.au>

#include <calibaccess/ParsetCalSolutionSource.h>
#include <calibaccess/ParsetCalSolutionAccessor.h>

// logging stuff
#include <askap_accessors.h>
#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, ".calibaccess");

namespace askap {

namespace accessors {

/// @brief constructor
/// @details Creates solution source object for a given parset file
/// (whether it is for writing or reading depends on the actual methods
/// used).
/// @param[in] parset parset file name
ParsetCalSolutionSource::ParsetCalSolutionSource(const std::string &parset) :
   CalSolutionSourceStub(boost::shared_ptr<ParsetCalSolutionAccessor>(new ParsetCalSolutionAccessor(parset))) {}
  
} // accessors

} // namespace askap

