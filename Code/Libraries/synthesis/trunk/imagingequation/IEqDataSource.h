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

#include <string>

using std::string;

#include "IEqDataAccessor.h"

namespace conrad {
class IEqDataSource
{
public:
	/// Construct a data source with the specified name
	IEqDataSource(const string& name) : itsName(name) {};
	
	virtual ~IEqDataSource();
	
	/// Initialize iteration with accessor
	/// @arg selection TaQL selection string
	void init(const string& selection);
	
	/// Initialize iteration with accessor
	void init();
	
	/// Return the data accessor
	IEqDataAccessor& ida();
	
	/// Are there any more data?
	bool next() const;
	
protected:
	string itsName;
private:
	IEqDataAccessor itsIda;
};
}
#endif /*IEQDATASOURCE_H_*/
