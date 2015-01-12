/// @file 
/// @brief interface describing context of the parallel step
/// @details The context describes parallel specifics for 
/// a particular processing step or its part. This includes
/// methods to obtain actual communicators to send data around,
/// work domain iterators and various application specific information.
/// We rely on polymorphic behaviour to substitute objects with adapters
/// when necessary (e.g. when work domain is split between a number of
/// workers).
///
/// For more information see ASKAPSDP-1605 issue. This class plays the
/// role of the IComms class in the original design presentation attached
/// to the jira ticket.
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

// own includes
#include "application/IContext.h"

namespace askap {

namespace askapparallel {
	
/// @brief an empty virtual destructor to make the compiler happy
IContext::~IContext() {}

/// @brief obtain global communicator
/// @details
/// Global communicator can be used to broadcast across all available ranks.
/// It is probably a good idea to try using named communicators as much 
/// as we can for these type of operations. 
/// @return shared pointer to communicator object
/// @note The result is guaranteed to be a non-zero pointer. 
/// An exception is thrown if a communicator with given name
/// does not exist.
boost::shared_ptr<IComms> IContext::globalComm() const 
           { return getComm("global"); }

/// @brief obtain local communicator
/// @details
/// Local communicator can be used to broadcast within ranks allocated
/// to a particular multi-rank processing step.
/// @return shared pointer to communicator object
/// @note The result is guaranteed to be a non-zero pointer. 
/// An exception is thrown if a communicator with given name
/// does not exist.
boost::shared_ptr<IComms> IContext::localComm() const 
          { return getComm("local"); }
  
} // end of namespace askapparallel
} // end of namespace askap

