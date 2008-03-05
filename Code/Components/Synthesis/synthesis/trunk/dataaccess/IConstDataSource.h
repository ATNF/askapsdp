/// @file IConstDataSource.h
/// @brief Read-only access to a source of visibility data
/// @details IConstDataSource allows access to a source of visibility data,
/// probably either a MeasurementSet or a stream.
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef I_CONST_DATA_SOURCE_H
#define I_CONST_DATA_SOURCE_H

// boost includes
#include <boost/shared_ptr.hpp>

// CASA includes
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MFrequency.h>
#include <measures/Measures/MRadialVelocity.h>

// own includes
#include <dataaccess/IConstDataIterator.h>
#include <dataaccess/IDataSelector.h>
#include <dataaccess/IDataConverter.h>


namespace askap {

namespace synthesis {
	
/// short cut for shared pointer to IDataSelector
typedef boost::shared_ptr<IDataSelector> IDataSelectorPtr;

/// short cut for shared pointer to IDataConverter
typedef boost::shared_ptr<IDataConverter> IDataConverterPtr;

/// short cut for shared pointer to const IDataSelector
typedef boost::shared_ptr<IDataSelector const> IDataSelectorConstPtr;

/// short cut for shared pointer to const IDataConverter
typedef boost::shared_ptr<IDataConverter const> IDataConverterConstPtr;

/// @brief Read-only access to a source of visibility data
/// @details IConstDataSource allows access to a source of visibility data,
/// probably either a MeasurementSet or a stream.
/// @ingroup dataaccess_i
class IConstDataSource
{
public:
	
	/// an empty virtual destructor to make the compiler happy
	virtual ~IConstDataSource();

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
	virtual IDataConverterPtr createConverter() const = 0;

	/// get iterator over the whole dataset represented by this DataSource
	/// object. Default data conversion policies will be used, see
	/// IDataConverter.h for default values. Default implementation is
	/// via the most general createConstIterator(...) call, override it in 
	/// derived classes, if a (bit) higher performance is required
	///
	/// @return a shared pointer to ConstDataIterator object
	///
	/// The method acts as a factory by creating a new ConstDataIterator.
	/// The lifetime of this iterator is the same as the lifetime of
	/// the DataSource object. Therefore, it can be reused multiple times,
	/// if necessary. 
	virtual boost::shared_ptr<IConstDataIterator> 
		                  createConstIterator() const;

	/// get iterator over the whole dataset with explicitly specified
	/// conversion policies. Default implementation is via the most 
	/// general createConstIterator(...) call, override it in the derived
	/// classes, if a (bit) higer performance is required
	///
	/// @param[in] conv a shared pointer to the converter object defining
	///            reference frames and units to be used
	/// @return a shared pointer to ConstDataIterator object
	///
	/// The method acts as a factory by creating a new ConstDataIterator.
	/// The lifetime of this iterator is the same as the lifetime of
	/// the DataSource object. Therefore, it can be reused multiple times,
	/// if necessary. 
	virtual boost::shared_ptr<IConstDataIterator> createConstIterator(const
                    IDataConverterConstPtr &conv) const;
	
	
	/// this variant of createConstIterator is defined to force the type
	/// conversion between the non-const and const smart pointers 
	/// explicitly. Otherwise, method overloading doesn't work because
	/// the compiler tries to build a template for interconversion
	/// between IDataConverter and IDataSelector
	/// 
	/// This version just calls the appropriate virtual function and
	/// shouldn't add any overheads, provided the compiler can optimize
	/// inline methods properly
	inline boost::shared_ptr<IConstDataIterator> createConstIterator(const
		    IDataConverterPtr &conv) const { 
            return createConstIterator((const IDataConverterConstPtr&)conv); 
        }

	/// get iterator over a selected part of the dataset represented
	/// by this DataSource object. Default data conversion policies
	/// will be used, see IDataConverter.h for default values.
	/// The default implementation is via the most general 
	/// createConstIterator(...) call, override it in derived classes, 
	/// if a (bit) higher performance is required
	///
	/// @param[in] sel a shared pointer to the selector object defining 
	///            which subset of the data is used
	/// @return a shared pointer to DataIterator object
	///
	/// The method acts as a factory by creating a new DataIterator.
	/// The lifetime of this iterator is the same as the lifetime of
	/// the DataSource object. Therefore, it can be reused multiple times,
	/// if necessary. 
	virtual boost::shared_ptr<IConstDataIterator> createConstIterator(const
	           IDataSelectorConstPtr &sel) const;

	/// this variant of createConstIterator is defined to force the type
	/// conversion between the non-const and const smart pointers 
	/// explicitly. Otherwise, method overloading doesn't work because
	/// the compiler tries to build a template for interconversion
	/// between IDataConverter and IDataSelector
	/// 
	/// This version just calls the appropriate virtual function and
	/// shouldn't add any overheads, provided the compiler can optimize
	/// inline methods properly
	inline boost::shared_ptr<IConstDataIterator> createConstIterator(const
		    IDataSelectorPtr &sel) const { 
            return createConstIterator((const IDataSelectorConstPtr&)sel); 
        }

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
	           IDataSelectorConstPtr &sel, const
		   IDataConverterConstPtr &conv) const = 0;

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
	virtual IDataSelectorPtr createSelector() const = 0;
};

} // end of namespace synthesis
} // end of namespace askap
#endif // I_CONST_DATA_SOURCE_H
