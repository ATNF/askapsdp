/// @file
///
/// IEqDataSource: Allow access to a source of visibility data, probably
/// either a MeasurementSet or a stream.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef IEQDATASOURCE_H_
#define IEQDATASOURCE_H_

#include <casa/BasicSL/String.h>

#include "IEqDataAccessor.h"

namespace conrad {
class IEqDataSource
{
public:
	/// Construct a data source with the specified name
	IEqDataSource(casa::String name) : itsName(name) {};
	
	virtual ~IEqDataSource();
	
	/// Initialize iteration with accessor
	/// @arg selection TaQL selection string
	void init(casa::String& selection);
	
	/// Return the data accessor
	IEqDataAccessor& ida();
	
	/// Is there any more?
	bool next();
	
protected:
	casa::String itsName;
};
}
#endif /*IEQDATASOURCE_H_*/
