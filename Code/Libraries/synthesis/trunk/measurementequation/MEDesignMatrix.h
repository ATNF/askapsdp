/// @file
///
/// MEDesignMatrix: Hold the design matrix for parameters
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef MEDESIGNMATRIX_H
#define MEDESIGNMATRIX_H

namespace conrad
{
	namespace synthesis
{
	
class MEParams;

class MEDesignMatrix
{
public:
	MEDesignMatrix(const MEParams& ip);
	virtual ~MEDesignMatrix();
	void merge(const MEDesignMatrix& other);
	void reset();
};

}
}

#endif /*MENORMALEQUATIONS_H_*/
