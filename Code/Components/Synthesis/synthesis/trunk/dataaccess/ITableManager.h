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
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef I_TABLE_MANAGER_H
#define I_TABLE_MANAGER_H

// own includes
#include <dataaccess/ISubtableInfoHolder.h>
#include <dataaccess/ITableHolder.h>
#include <dataaccess/IMiscTableInfoHolder.h>

namespace conrad {

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

} // namespace conrad

#endif // #define TABLE_MANAGER_H
