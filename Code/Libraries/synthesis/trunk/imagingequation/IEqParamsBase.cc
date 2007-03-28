#include "IEqParamsBase.tcc"

#include "IEqParam.h"
#include "IEqImageParam.h"

namespace conrad {
	template class IEqParamsBase<IEqParam>;
	template class IEqParamsBase<IEqImageParam>;
}