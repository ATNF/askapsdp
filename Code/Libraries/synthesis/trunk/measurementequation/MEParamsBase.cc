#include "MEParamsBase.tcc"

#include "MEParam.h"
#include "MEImageParam.h"

namespace conrad {
	template class MEParamsBase<MEParam>;
	template class MEParamsBase<MEImageParam>;
}