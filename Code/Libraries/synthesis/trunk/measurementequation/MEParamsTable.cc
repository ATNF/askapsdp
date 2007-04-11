#include <measurementequation/MEParamsTable.h>

namespace conrad
{
namespace synthesis
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

bool MEParamsTable::getParameters(MERegularParams& ip, const MEDomain& domain) const
{
	return false;
};

bool MEParamsTable::getParameters(MEImageParams& ip, const MEDomain& domain) const
{
	return false;
};

bool MEParamsTable::setParameters(const MEParams& ip, const MEDomain& domain) {
	return false;
}
bool MEParamsTable::setParameters(const MERegularParams& ip, const MEDomain& domain) {
	return false;
}
bool MEParamsTable::setParameters(const MEImageParams& ip, const MEDomain& domain) {
	return false;
}
}
}
