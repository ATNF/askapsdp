#include "IEqDataSource.h"

namespace conrad {

IEqDataSource::~IEqDataSource()
{
}
void IEqDataSource::init() {
}

bool IEqDataSource::next() const {
	return false;
}

IEqDataAccessor& IEqDataSource::ida() {
	return itsIda;
}

}
