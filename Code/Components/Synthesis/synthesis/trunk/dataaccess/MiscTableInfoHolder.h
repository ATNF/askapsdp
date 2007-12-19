/// @file 
/// @brief An implementation of the IMiscTableInfoHolder interface
/// @details The main idea of this class is to carry around 
/// additional information describing how the table is going to be
/// processed. The overall design is similar to ISubtableInfoHolder and
/// derived classes. Although this additional info can be assigned to 
/// either TableHolder or SubtableInfoHolder, making a separate tree of
/// classes seems to be a more structured approach.
/// Finally, having this miscelaneous information carried between classes 
/// the same way as the table itself and associated derived information,
/// allows to avoid creating multiple copies for data source and iterators
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef MISC_TABLE_INFO_HOLDER_H
#define MISC_TABLE_INFO_HOLDER_H

// own includes
#include <dataaccess/IMiscTableInfoHolder.h>

// std includes
#include <string>

namespace conrad {

namespace synthesis {

/// @brief An implementation of the IMiscTableInfoHolder interface
/// @details The main idea of this class is to carry around 
/// additional information describing how the table is going to be
/// processed. The overall design is similar to ISubtableInfoHolder and
/// derived classes. Although this additional info can be assigned to 
/// either TableHolder or SubtableInfoHolder, making a separate tree of
/// classes seems to be a more structured approach.
/// Finally, having this miscelaneous information carried between classes 
/// the same way as the table itself and associated derived information,
/// allows to avoid creating multiple copies for data source and iterators
/// @ingroup dataaccess_tm
struct MiscTableInfoHolder : virtual public IMiscTableInfoHolder 
{
  /// @brief construct a holder of miscellaneous table processing information
  /// @details The class just remembers the default column name passed in
  /// this method.
  /// @param[in] dataColumn the name of the data column used by default
  explicit MiscTableInfoHolder(const std::string &dataColumn);

  /// @brief obtain the name of the data column to use by default
  /// @details The code allows to read/write data not only from DATA
  /// column of the measurement set, but from any other suitable column
  /// as well. It is possible to change the name of the column via 
  /// selector (ITableDataSelector or derived classes only, as this is
  /// a table-specific operation. Hence a dynamic_cast may be required).
  /// An alternative is to change the default column name via data source
  /// constructor (it will be carried accross all required classes by 
  /// TableManager).
  /// @return a const reference to the default name of the data column 
  virtual const std::string& defaultDataColumnName() const throw();
private:
  /// @brief name of the data column used by default
  std::string itsDefaultDataColumnName;
};

} // namespace synthesis

} // namespace conrad

#endif // #ifndef MISC_TABLE_INFO_HOLDER_H
