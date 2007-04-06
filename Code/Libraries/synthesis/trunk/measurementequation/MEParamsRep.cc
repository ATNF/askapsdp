#include <measurementequation/MEParamsRep.tcc>
#include <measurementequation/MEParamsRep.h>

#include <measurementequation/MEimage.h>

#include <vector>

using std::vector;

namespace conrad {
namespace synthesis
{
	template class MEParamsRep<double>;
	template class MEParamsRep<MEImage>;
	
}
}