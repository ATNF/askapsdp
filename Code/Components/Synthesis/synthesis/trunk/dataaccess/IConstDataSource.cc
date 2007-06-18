/// @file IConstDataSource.cc
/// @brief Access to a source of visibility data
/// @details IConstDataSource allows access to a source of visibility data,
/// probably either a MeasurementSet or a stream.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#include <dataaccess/IConstDataSource.h>

namespace conrad {

namespace synthesis {

// an empty virtual destructor to make the compiler happy
IConstDataSource::~IConstDataSource()
{
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
boost::shared_ptr<IConstDataIterator> 
    IConstDataSource::createConstIterator(const
            IDataSelectorConstPtr &sel) const {
    // create a new default converter just for this new iterator
    return createConstIterator(sel,createConverter());
}

/// get iterator over the whole dataset represented by this DataSource
/// object. Default data conversion policies will be used, see
/// IDataConverter.h for default values. Default implementation is
/// via the most general createConstIterator(...) call, override it in
/// derived classes, if a (bit) higher performance is required
///
/// @return a shared pointer to ConstDataIterator object
///
/// The method acts as a factory by creating a new DataIterator.
/// The lifetime of this iterator is the same as the lifetime of
/// the DataSource object. Therefore, it can be reused multiple times,
/// if necessary.
boost::shared_ptr<IConstDataIterator> 
    IConstDataSource::createConstIterator() const {
    // create new default selector and converter just for this new iterator
    return createConstIterator(createSelector(),createConverter());
}

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
boost::shared_ptr<IConstDataIterator> 
    IConstDataSource::createConstIterator(const
	IDataConverterConstPtr &conv) const {
    return createConstIterator(createSelector(),conv);
}

} // end of namespace synthesis

} // end of namespace conrad
