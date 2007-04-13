/// @file
///
/// MEDesignMatrix: Hold the design matrix for parameters
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef MEDESIGNMATRIX_H
#define MEDESIGNMATRIX_H

#include <map>

#include <casa/aips.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>

#include <measurementequation/MEParams.h>

namespace conrad
{
	namespace synthesis
{
	
class MEDesignMatrix
{
public:

	MEDesignMatrix() {};
	
	/// Define a design matrix
	/// @param ip Parameters
	MEDesignMatrix(const MEParams& ip);
	
	/// Copy constructor
	MEDesignMatrix(const MEDesignMatrix& dm);
	
	/// Assignment operator
	MEDesignMatrix& operator=(const MEDesignMatrix& dm);
	
	virtual ~MEDesignMatrix();
	
	/// Merge this design matrix with another - means that we just
	/// need to append on the data axis
	/// @param other Other design matrix
	void merge(const MEDesignMatrix& other);
	
	/// Add the derivative with respect to the named parameter
	/// @param name Name of parameter
	/// @param deriv Derivative
	void addDerivative(const string& name, const casa::Vector<double>& deriv);
	
	/// Reset to empty
	void reset();
private:
	uint itsDataLength;
	std::map<string, casa::Vector<double> > itsDesignMatrix;
	std::map<string, casa::Vector<double> >::iterator itsIter;
};

}
}

#endif 
