/// @file
///
/// MEDataSource: Allow access to a source of visibility data, probably
/// either a MeasurementSet or a stream.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef IEQDATASOURCE_H_
#define IEQDATASOURCE_H_

#include <string>

using std::string;

#include "METableDataAccessor.h"

namespace conrad {
class MEDataSource
{
public:
	/// Construct a data source
	MEDataSource() {};
	
	virtual ~MEDataSource();
	
	/// Initialize iteration with accessor
	/// @arg selection TaQL selection string
	void init(const string& selection);
	
	/// Initialize iteration with accessor
	void init();
	
	/// Return the data accessor
	MEDataAccessor& ida();
	
	/// Are there any more data?
	bool next() const;
	
protected:
private:
	METableDataAccessor itsIda;
};
}
#endif /*IEQDATASOURCE_H_*/
