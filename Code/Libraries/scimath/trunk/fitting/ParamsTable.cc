#include <fitting/ParamsTable.h>
#include <fitting/Params.h>

namespace conrad
{
namespace scimath
{

ParamsTable::ParamsTable()
{
}

ParamsTable::~ParamsTable()
{
}

bool ParamsTable::getParameters(Params& ip) const
{
	return false;
};

bool ParamsTable::setParameters(const Params& ip) {
	return false;
}

}
}
