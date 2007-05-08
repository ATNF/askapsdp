#include <fitting/ParamsTable.h>
#include <fitting/Params.h>
#include <fitting/Domain.h>

namespace conrad
{
namespace synthesis
{

ParamsTable::ParamsTable()
{
}

ParamsTable::~ParamsTable()
{
}

bool ParamsTable::getParameters(Params& ip, const Domain& domain) const
{
	return false;
};

bool ParamsTable::setParameters(const Params& ip, const Domain& domain) {
	return false;
}
}
}
