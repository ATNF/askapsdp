/// @file
/// @brief Interface to a manager of a table and derived information
/// @details This interface is a pure structural item, which doesn't
/// introduce any new members or implement any methods. It joins different
/// branches in the interface class tree: ITableHolder, IMiscTableInfoHolder and
/// ISubtableInfoHolder. The derived TableManager class implements
/// all interfaces and provides an access to the Table itself, additional
/// processing options and cached derived information. See its description 
/// for more details on how this part of the class tree is organized.
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

#ifndef I_TABLE_MANAGER_H
#define I_TABLE_MANAGER_H

// own includes
#include <dataaccess/ISubtableInfoHolder.h>
#include <dataaccess/ITableHolder.h>
#include <dataaccess/IMiscTableInfoHolder.h>

namespace askap {

namespace synthesis {

/// @brief Interface to a manager of a table and derived information
/// @details This interface is a pure structural item, which doesn't
/// introduce any new members or implement any methods. It joins different
/// branches in the interface class tree: ITableHolder, IMiscTableInfoHolder and
/// ISubtableInfoHolder. The derived TableManager class implements
/// all interfaces and provides an access to the Table itself, additional
/// processing options and cached derived information. See its description for 
/// more details on how this part of the class tree is organized.
/// @ingroup dataaccess_tm
struct ITableManager : virtual public ITableHolder,
                       virtual public ISubtableInfoHolder,
                       virtual public IMiscTableInfoHolder
{  
};

} // namespace synthesis

} // namespace askap

#endif // #define TABLE_MANAGER_H
