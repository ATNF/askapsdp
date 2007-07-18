/// @file
/// @brief A handler of the FIELD subtable
/// @details This class derived provides access to
/// the content of the FIELD subtable (which provides delay, phase and
/// reference centres for each time). The POINTING table gives the actual 
/// pointing of the antennae. Although this implementation caches the values
/// for the last requested time range, it reads the data on-demand. This is 
/// a difference from subtable handler classes, whose name starts from Mem...
/// The latter classes read all the subtable into memory in the constructor and
/// later return cached values. 
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef TABLE_FIELD_HOLDER_H
#define TABLE_FIELD_HOLDER_H

// casa includes
#include <tables/Tables/Table.h>
#include <tables/Tables/TableIter.h>
#include <casa/Quanta/Quantum.h>

// own includes
#include <dataaccess/ITableFieldHolder.h>
#include <dataaccess/TimeDependentSubtable.h>
#include <dataaccess/TableHolder.h>

namespace conrad {

namespace synthesis {


/// @brief An interface to FIELD subtable
/// @details A class derived from this interface provides access to
/// the content of the FIELD subtable (which provides delay, phase and
/// reference centres for each time). The POINTING table gives the actual 
/// pointing of the antennae. Although this implementation caches the values
/// for the last requested time range, it reads the data on-demand. This is 
/// a difference from subtable handler classes, whose name starts from Mem...
/// The latter classes read all the subtable into memory in the constructor and
/// later return cached values.
/// @note The class has not been properly tested with time-dependent FIELD table
/// @ingroup dataaccess_tab
struct TableFieldHolder : virtual public ITableFieldHolder,
                          virtual protected TableHolder,
                          virtual protected TimeDependentSubtable {

  /// @brief construct the object
  /// @details 
  /// @param[in] ms a table object, which has a field subtable defined
  /// (i.e. this method accepts a main ms table).
  explicit TableFieldHolder(const casa::Table &ms); 
  
  /// @brief obtain the reference direction for a given time.
  /// @details It is not clear at the moment whether this subtable is
  /// useful in the multi-beam case because each physical feed corresponds to
  /// its own phase- and delay tracking centre. It is assumed at the moment
  /// that the reference direction can be used as the dish pointing direction
  /// in the absence of the POINTING subtable. It is not clear what this
  /// direction should be in the case of scanning.
  /// @param[in] time a full epoch of interest (the subtable can have multiple
  /// pointings.
  /// @return a reference to direction measure
  virtual const casa::MDirection& getReferenceDir(const casa::MEpoch &time) 
                                                  const;
protected:
  /// read the data if cache is outdated
  /// @param[in] time a full epoch of interest (field table can have many
  /// pointing and therefore can be time-dependent)
  void fillCacheOnDemand(const casa::MEpoch &time) const;
  
  /// read the current iteration and populate cache
  void fillCacheWithCurrentIteration() const;
  
private:
  
  /// iterator over the FIELD subtable
  mutable casa::TableIterator itsIterator;
   
  /// start time of the time range, for which the cache is valid
  /// Time independent table has a very wide time range. Values are
  /// stored as Doubles in the native frame/units of the FIELD table
  mutable casa::Double itsCachedStartTime;
  
  /// stop time of the time range, for which the cache is valid
  /// see itsCachedStartTime for more details.
  mutable casa::Double itsCachedStopTime;
  
  /// cache of the reference direction
  mutable casa::MDirection itsReferenceDir;
   
};


} // namespace synthesis

} // namespace conrad

#endif // #ifndef TABLE_FIELD_HOLDER_H
