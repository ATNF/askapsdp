#ifndef IEQDATASOURCE_H_
#define IEQDATASOURCE_H_

#include <casa/BasicSL/String.h>

#include "IEqDataAccessor.h"

namespace conrad {
class IEqDataSource
{
public:
	IEqDataSource(casa::String name) : itsName(name) {};
	virtual ~IEqDataSource();
	
	IEqDataAccessor& ida();
	void init();
	bool next();
	
protected:
	casa::String itsName;
};
}
#endif /*IEQDATASOURCE_H_*/
