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
	/// Define a design matrix
	/// @param ip Parameters
	MEDesignMatrix(const MEParams& ip);
	
	virtual ~MEDesignMatrix();
	
	/// Merge this design matrix with another - means that we just
	/// need to append on the data access
	/// @param other Other design matrix
	void merge(const MEDesignMatrix& other);
	void reset();
};

}
}

#endif /*MENORMALEQUATIONS_H_*/
