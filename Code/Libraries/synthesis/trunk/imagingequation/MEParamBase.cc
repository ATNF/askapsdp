#include "MEParamBase.tcc"

#include "MEParam.h"
#include "MEImage.h"

namespace conrad {
	template class MEParamBase<double>;
	template class MEParamBase<MEImage>;
}