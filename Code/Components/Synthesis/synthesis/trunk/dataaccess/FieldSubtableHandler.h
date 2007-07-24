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

#ifndef FIELD_SUBTABLE_HANDLER_H
#define FIELD_SUBTABLE_HANDLER_H

// casa includes
#include <tables/Tables/Table.h>
#include <tables/Tables/TableIter.h>
#include <casa/Quanta/Quantum.h>

// own includes
#include <dataaccess/IFieldSubtableHandler.h>
#include <dataaccess/TimeDependentSubtable.h>
#include <dataaccess/TableHolder.h>

namespace conrad {

namespace synthesis {


/// @brief A handler of the FIELD subtable
/// @details This class derived provides access to
/// the content of the FIELD subtable (which provides delay, phase and
/// reference centres for each time). The POINTING table gives the actual 
/// pointing of the antennae. Although this implementation caches the values
/// for the last requested time range, it reads the data on-demand. This is 
/// a difference from subtable handler classes, whose name starts from Mem...
/// The latter classes read all the subtable into memory in the constructor and
/// later return cached values. 
/// @note The class has not been properly tested with time-dependent FIELD table
/// @ingroup dataaccess_tab
struct FieldSubtableHandler : virtual public IFieldSubtableHandler,
                              virtual protected TableHolder,
                              virtual protected TimeDependentSubtable {

  /// @brief construct the object
  /// @details 
  /// @param[in] ms a table object, which has a field subtable defined
  /// (i.e. this method accepts a main ms table).
  explicit FieldSubtableHandler(const casa::Table &ms); 
  
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

  /// @brief check whether the field changed for a given time
  /// @details The users of this class can do relatively heavy calculations
  /// depending on the field position on the sky. It is, therefore, practical
  /// to assist caching by providing a method to test whether the cache is
  /// still valid or not for a new time. Use this method instead of testing
  /// whether directions are close enough as it can make use the information
  /// stored in the subtable. The method always returns true before the 
  /// first access to the data.
  /// @param[in] time a full epoch of interest (the subtable can have multiple
  /// pointings.
  /// @return true if the field information have been changed
  virtual bool newField(const casa::MEpoch &time) const;

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
  
  /// @brief flag showing that no data has been obtained yet via this class
  /// @details It is necessary that newField always returns true before 
  /// the first getReferenceDir call. Otherwise, caching of the derived
  /// information can be complicated. With this flag the meaning of newField
  /// method is to test whether the field is new since the last access to
  /// its parameters.
  mutable bool itsNeverAccessedFlag;
};


} // namespace synthesis

} // namespace conrad

#endif // #ifndef FIELD_SUBTABLE_HANDLER_H

