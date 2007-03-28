#include "IEqParamBase.tcc"

#include "IEqParam.h"
#include "IEqImage.h"

namespace conrad {
	template class IEqParamBase<double>;
	template class IEqParamBase<IEqImage>;
}