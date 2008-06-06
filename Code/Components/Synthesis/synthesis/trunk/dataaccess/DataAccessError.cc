/// @file DataAccessError.cc
/// @brief define exception classes used at the data access layer
/// @details this file contains implementations of the exception classes,
/// which can be thrown from the data access layer
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

/// own includes
#include <dataaccess/DataAccessError.h>

using namespace askap;
using namespace askap::synthesis;

/// constructor - pass the message to the base class
///
/// @param message a string message explaining what happens
///
DataAccessError::DataAccessError(const std::string& message) :
     AskapError(message) {}

/// constructor - pass the message to the base class
///
/// @param message a string message explaining what happens
///
DataAccessLogicError::DataAccessLogicError(const std::string& message) :
     DataAccessError(message) {}
