#include "IEqParamsTable.h"

namespace conrad
{

IEqParamsTable::IEqParamsTable()
{
}

IEqParamsTable::~IEqParamsTable()
{
}

IEqParams IEqParamsTable::getParameters(const IEqParams& ip, const IEqDomain& domain) const {
	return IEqParams(ip);
}

bool IEqParamsTable::setParameters(const IEqParams& ip, const IEqDomain& domain) {
	return true;
}


}
