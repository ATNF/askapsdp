/// @file
///
/// IEqParam: represent a parameter for imaging equation. A parameter
/// can be a single real number. The first two derivatives are also included.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///

#ifndef IEQPARAM_H
#define IEQPARAM_H

#include "IEqParamBase.h"

namespace conrad {
	
class IEqParam : public IEqParamBase<double> {
public:
	IEqParam() {};
};

};

#endif





