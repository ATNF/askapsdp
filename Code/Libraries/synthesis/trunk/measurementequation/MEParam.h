/// @file
///
/// MERealParam: represent a parameter for imaging equation. A parameter
/// can be a single real number. The first two derivatives are also included.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///

#ifndef MEPARAM_H
#define MEPARAM_H

#include "MEParamBase.h"

namespace conrad {
	
class MEParam : public MEParamBase<double> {
public:
	MEParam() {};
};

};

#endif





