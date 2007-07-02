/// @file
/// @brief Interface to a manager of a table and derived information
/// @details This interface is a pure structural item, which doesn't
/// introduce any new members or implement any methods. It joins two
/// branches in the interface class tree: ITableHolder and
/// ISubtableInfoHolder. The derived TableManager class implements
/// both interfaces and provides a access to the Table itself and
/// cached derived information. See its description for more details on 
/// how this part of the class tree is organized.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef I_TABLE_MANAGER_H
#define I_TABLE_MANAGER_H

// own includes
#include <dataaccess/ISubtableInfoHolder.h>
#include <dataaccess/ITableHolder.h>

namespace conrad {

namespace synthesis {

/// @brief Interface to a manager of a table and derived information
/// @details This interface is a pure structural item, which doesn't
/// introduce any new members or implement any methods. It joins two
/// branches in the interface class tree: ITableHolder and
/// ISubtableInfoHolder. The derived TableManager class implements
/// both interfaces and provides a access to the Table itself and
/// cached derived information. See its description for more details on 
/// how this part of the class tree is organized.
/// @ingroup dataaccess
struct ITableManager : virtual public ITableHolder,
                       virtual public ISubtableInfoHolder
{  
};

} // namespace synthesis

} // namespace conrad

#endif // #define TABLE_MANAGER_H
