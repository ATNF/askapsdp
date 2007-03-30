#include "MEParamsTable.h"

namespace conrad
{

MEParamsTable::MEParamsTable()
{
}

MEParamsTable::~MEParamsTable()
{
}

bool MEParamsTable::getParameters(MEParams& ip, const MEDomain& domain) const
{
	return false;
};

bool MEParamsTable::setParameters(const MEParams& ip, const MEDomain& domain) {
	return false;
}

bool MEParamsTable::getParameters(MEImageParams& ip, const MEDomain& domain) const
{
	return false;
};

bool MEParamsTable::setParameters(const MEImageParams& ip, const MEDomain& domain) {
	return false;
}


}
