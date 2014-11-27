/// @file 
/// @brief Communicator interface
/// @details New parallel framework relies on a polymorphic communicator
/// class to perform operations appropriate for the context.
/// This abstract base class represents a generic interface
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

#ifndef ASKAP_ASKAPPARALLEL_I_COMMS_H
#define ASKAP_ASKAPPARALLEL_I_COMMS_H

// boost includes
#include <boost/shared_ptr.hpp>

// own includes


namespace askap {

namespace askapparallel {
	
/// @brief Communicator interface
/// @details New parallel framework relies on a polymorphic communicator
/// class to perform operations appropriate for the context.
/// This abstract base class represents a generic interface
class IComms
{
public:
	
  /// an empty virtual destructor to make the compiler happy
  virtual ~IComms();

  // usual communication methods for broadcast, point2point, etc
};

} // end of namespace askapparallel
} // end of namespace askap
#endif // ASKAP_ASKAPPARALLEL_I_COMMS_H

