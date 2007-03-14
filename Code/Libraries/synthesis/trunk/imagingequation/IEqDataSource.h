#ifndef IEQDATASOURCE_H_
#define IEQDATASOURCE_H_

#include "IEqDataAccessor.h"

namespace conrad {
class IEqDataSource
{
public:
	IEqDataSource();
	virtual ~IEqDataSource();
	
	IEqDataAccessor& ida();
	bool next();
	
};
}
#endif /*IEQDATASOURCE_H_*/
