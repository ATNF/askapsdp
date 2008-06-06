/// @file DataAccessError.h
/// @brief Exception classes used at the data access layer
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

#ifndef DATA_ACCESS_ERROR_H
#define DATA_ACCESS_ERROR_H

/// std includes
#include <string>

/// own includes
#include <askap/AskapError.h>

namespace askap {

namespace synthesis {

/// @brief general exception class used in the data access layer
/// @ingroup dataaccess_i
struct DataAccessError : public AskapError
{
  /// constructor - pass the message to the base class
  ///
  /// @param message a string message explaining what happens
  ///
  explicit DataAccessError(const std::string& message);
};

/// @brief exception class indicating a logic error in the data access layer
struct DataAccessLogicError : public DataAccessError
{
  /// constructor - pass the message to the base class
  ///
  /// @param message a string message explaining what happens
  ///
  explicit DataAccessLogicError(const std::string& message);
};

} // namespace askap

} // namespace askap

#endif // #ifndef DATA_ACCESS_ERROR_H
