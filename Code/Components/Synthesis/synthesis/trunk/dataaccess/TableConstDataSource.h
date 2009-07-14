/// @file TableConstDataSource.h
/// @brief Implementation of IConstDataSource in the table-based case
/// @details
/// TableConstDataSource: Allow read-only access to the data stored in the
/// measurement set. This class implements IConstDataSource interface.
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

#ifndef TABLE_CONST_DATA_SOURCE_H
#define TABLE_CONST_DATA_SOURCE_H

// boost includes
#include <boost/shared_ptr.hpp>

// casa includes
#include <tables/Tables/Table.h>

// own includes
#include <dataaccess/IConstDataSource.h>
#include <dataaccess/TableInfoAccessor.h>

// std includes
#include <string>

namespace askap {

namespace synthesis {

/// @brief Implementation of IConstDataSource in the table-based case
/// @details
/// TableConstDataSource: Allow read-only access to the data stored in the
/// measurement set. This class implements IConstDataSource interface.
/// @ingroup dataaccess_tab
class TableConstDataSource : virtual public IConstDataSource,
                             virtual protected TableInfoAccessor
{
public:
  /// @brief construct a read-only data source object
  /// @details All iterators obtained from this object will be read-only
  /// iterators.
  /// @param[in] fname file name of the measurement set to use
  /// @param[in] dataColumn a name of the data column used by default
  ///                       (default is DATA)
  explicit TableConstDataSource(const std::string &fname, 
                                const std::string &dataColumn = "DATA");
  
  /// create a converter object corresponding to this type of the
  /// DataSource. The user can change converting policies (units,
  /// reference frames) by appropriate calls to this converter object
  /// and pass it back to createConstIterator(...). The data returned by
  /// the iteratsr will automatically be in the requested frame/units
  ///
  /// @return a shared pointer to a new DataConverter object
  ///
  /// The method acts as a factory by creating a new DataConverter.
  /// The lifetime of this converter is the same as the lifetime of the
  /// DataSource object. Therefore, it can be reused multiple times,
  /// if necessary. However, the behavior of iterators created
  /// with a particular DataConverter is undefined, if you change
  /// the DataConverter after the creation of an iterator, unless you
  /// call init() of the iterator (and start a new iteration loop).
  virtual IDataConverterPtr createConverter() const;
  
  /// get iterator over a selected part of the dataset represented
  /// by this DataSource object with an explicitly specified conversion
  /// policy. This is the most general createConstIterator(...) call, 
  /// which is used as a default implementation for all less general 
  /// cases (although they can be overriden in the derived classes, if it 
  /// will be necessary because of the performance issues)
  ///
  /// @param[in] sel a shared pointer to the selector object defining 
  ///            which subset of the data is used
  /// @param[in] conv a shared pointer to the converter object defining
  ///            reference frames and units to be used
  /// @return a shared pointer to DataIterator object
  ///
  /// The method acts as a factory by creating a new DataIterator.
  /// The lifetime of this iterator is the same as the lifetime of
  /// the DataSource object. Therefore, it can be reused multiple times,
  /// if necessary. Call init() to rewind the iterator.
  virtual boost::shared_ptr<IConstDataIterator> createConstIterator(const
             IDataSelectorConstPtr &sel,
             const IDataConverterConstPtr &conv) const;
  
  // we need this to get access to the overloaded syntax in the base class 
  using IConstDataSource::createConstIterator;
 
  /// create a selector object corresponding to this type of the
  /// DataSource
  ///
  /// @return a shared pointer to the DataSelector corresponding to
  /// this type of DataSource. DataSource acts as a factory and
  /// creates a selector object of the appropriate type
  ///
  /// This method acts as a factory by creating a new DataSelector
  /// appropriate to the given DataSource. The lifetime of the
  /// DataSelector is the same as the lifetime of the DataSource 
  /// object. Therefore, it can be reused multiple times,
  /// if necessary. However, the behavior of iterators already obtained
  /// with this DataSelector is undefined, if one changes the selection
  /// unless the init method is called for the iterator (and the new
  /// iteration loop is started).
  virtual IDataSelectorPtr createSelector() const;
  
  /// @breif configure caching of the uvw-machines
  /// @details A number of uvw machines can be cached at the same time. This can
  /// result in a significant performance improvement in the mosaicing case. By default
  /// only single machine is cached and this method should be called to change it. 
  /// All subsequent iterators will be created with the parameters set in this method until
  /// it is called again. Call this method without parameters to revert to default settings.
  /// @note This method is a feature of this implementation and is not available via the 
  /// general interface (intentionally)
  /// @param[in] cacheSize a number of uvw machines in the cache (default is 1)
  /// @param[in] tolerance pointing direction tolerance in radians, exceeding which leads 
  /// to initialisation of a new UVW Machine
  void configureUVWMachineCache(size_t cacheSize = 1, double tolerance = 1e-6);
  
protected:
  /// construct a part of the read only object for use in the
  /// derived classes
  TableConstDataSource();
  
  /// @brief UVW machine cache size
  /// @return size of the uvw machine cache
  inline size_t uvwMachineCacheSize() const {return itsUVWCacheSize;}
  
  /// @brief direction tolerance used for UVW machine cache
  /// @return direction tolerance used for UVW machine cache (in radians)
  inline double uvwMachineCacheTolerance() const {return itsUVWCacheTolerance;}   
  
private:
  /// @brief a number of uvw machines in the cache (default is 1)
  /// @details To speed up mosaicing it is possible to cache any number of uvw machines
  /// as it takes time to setup the transformation which depends on the phase centre. 
  /// A change to this parameter applies to all iterators created afterwards. 
  size_t itsUVWCacheSize;
  
  /// @brief pointing direction tolerance in radians (for uvw machine cache)
  /// @details Exceeding this tolerance leads to initialisation of a new UVW Machine in the cache
  double itsUVWCacheTolerance;
};
 
} // namespace synthesis

} // namespace askap

#endif // #ifndef TABLE_CONST_DATA_SOURCE_H
