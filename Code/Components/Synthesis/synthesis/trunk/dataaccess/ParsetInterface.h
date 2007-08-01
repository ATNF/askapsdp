/// @file
/// @brief A method to set up converters and selectors from parset file
/// @details Parameters are currently passed around using parset files.
/// The methods declared in this file set up converters and selectors
/// from the ParameterSet object. This is probably a temporary solution.
/// This code can eventually become a part of some class (e.g. a DataSource
/// which returns selectors and converters with the defaults alread
/// applied according to the parset file).
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef PARSET_INTERFACE_H
#define PARSET_INTERFACE_H

// own includes
#include <dataaccess/IDataConverter.h>
#include <dataaccess/IDataSelector.h>
#include <APS/ParameterSet.h>

// boost includes
#include <boost/shared_ptr.hpp>

namespace conrad {

namespace synthesis {

/// @brief set selections according to the given parset object
/// @details
/// @param[in] sel a shared pointer to the converter to be updated
/// @param[in] parset a parset object to read the parameters from
/// @ingroup dataaccess_hlp
void operator<<(const boost::shared_ptr<IDataSelector> &sel,
                          const LOFAR::ACC::APS::ParameterSet &parset);

} // namespace synthesis

} // namespace conrad


#endif // #ifndef PARSET_INTERFACE_H
