#include "MEDataSource.h"

namespace conrad {

MEDataSource::~MEDataSource()
{
}
void MEDataSource::init() {
}

bool MEDataSource::next() const {
	return false;
}

MEDataAccessor& MEDataSource::ida() {
	return itsIda;
}

}
