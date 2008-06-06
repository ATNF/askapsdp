/// @file
/// @brief Basic exception for master/worker related errors.
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
/// @author Ger van Diepen (diepen AT astron nl)
///
//# $Id$

#ifndef ASKAP_MWCOMMON_MWERROR_H
#define ASKAP_MWCOMMON_MWERROR_H

#include <askap/AskapError.h>

namespace askap { namespace cp {

  /// @ingroup mwcommon
  /// @brief Basic exception for master/worker related errors.

  /// This class defines the basic MW exception.
  /// Only this basic exception is defined so far. In the future, some more 
  /// fine-grained exceptions might be derived from it.

  class MWError: public askap::AskapError
  {
  public:
    /// Create the exception object with the given message.
    explicit MWError (const std::string& message);

    virtual ~MWError() throw();
  };


}} /// end namespaces

#endif
