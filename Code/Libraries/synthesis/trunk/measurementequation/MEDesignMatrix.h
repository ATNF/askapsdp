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
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>

#include <measurementequation/MEParams.h>

namespace conrad {
namespace synthesis {
	
class MENormalEquations;
	
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
	
	/// Add the derivative of the data with respect to the named parameter
	/// @param name Name of parameter
	/// @param deriv Derivative
	void addDerivative(const string& name, const casa::Array<double>& deriv);
	
	/// Add the residual constraint
	/// @param residual Residual vector
	/// @param weights Weight vector
	void addResidual(const casa::Vector<double>& residual, const casa::Vector<double>& weights);
	
	/// Reset to empty
	void reset();
	
	/// Return names of parameters
	vector<string> names() const;
	
	friend class MENormalEquations;
	
private:
	MEParams itsParams;
	// Design Matrix = number of parameters x number of data points
	// The number of dof of parameters can vary from parameter to parameter
	mutable std::map<string, casa::Array<double> > itsDesignMatrix;
	// Residual matrix = number of data points
	mutable casa::Vector<double> itsResiduals;
	mutable casa::Vector<double> itsWeights;
};

}
}

#endif 
