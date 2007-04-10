/// @file
///
/// MEDesignMatrix: represent a set of parameters for an imaging equation.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef MEDESIGNMATRIX_H_
#define MEDESIGNMATRIX_H_

#include <measurementequation/MEDesignMatrixRep.h>
#include <measurementequation/MEImage.h>
#include <measurementequation/MEParams.h>

namespace conrad {
namespace synthesis
{

class MERegularDesignMatrix : public MEDesignMatrixRep<double> {
public:
	MERegularDesignMatrix(const MERegularParams& ip) {};
};


}
}

#endif
