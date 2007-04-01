#include "MEParamsRep.tcc"
#include "MEParamsRep.h"

#include "MEimage.h"

#include <vector>

using std::vector;

namespace conrad {
	template class MEParamsRep<double>;
	template class MEParamsRep<MEImage>;
	
}