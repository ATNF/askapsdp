/// @file
///
/// MEDesignMatrix: Hold the design matrix for parameters
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef MEDESIGNMATRIXREP_H
#define MEDESIGNMATRIXREP_H

#include <map>
using std::map;

#include <casa/aips.h>
#include <casa/Arrays/Matrix.h>

#include <measurementequation/MEParamsRep.h>

namespace conrad
{
	namespace synthesis
{
	
template <class T>
class MEDesignMatrixRep
{
public:

	MEDesignMatrixRep() {};
	
	/// Define a design matrix
	/// @param ip Parameters
	MEDesignMatrixRep(const MEParamsRep<T>& ip);
	
	/// Copy constructor
	MEDesignMatrixRep(const MEDesignMatrixRep& dm);
	
	/// Assignment operator
	MEDesignMatrixRep& operator=(const MEDesignMatrixRep& dm);
	
	virtual ~MEDesignMatrixRep();
	
	/// Merge this design matrix with another - means that we just
	/// need to append on the data axis
	/// @param other Other design matrix
	void merge(const MEDesignMatrixRep& other);
	
	/// Reset to empty
	void reset();
private:
	map<string, int> itsIndices;
	casa::Matrix<T> itsDesignMatrix;
};

}
}

#endif 
