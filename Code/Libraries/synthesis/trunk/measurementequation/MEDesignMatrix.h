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
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>

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
	
	/// Add the derivative of the data with respect to dof of the named parameter
	/// @param name Name of parameter
	/// @param deriv Derivative
	void addDerivative(const string& name, const casa::Matrix<casa::Complex>& deriv);
	
	/// Add the residual constraint
	/// @param residual Residual vector
	/// @param weight Weight vector
	void addResidual(const casa::Vector<casa::Complex>& residual, const casa::Vector<double>& weight);
	
	/// Reset to empty
	void reset();

	/// Return the specified parameters
	const MEParams& parameters() const;
	MEParams& parameters();
		
	/// Return the design matrix
	const std::map<string, casa::Matrix<casa::Complex> >& designMatrix() const;
	
	/// Return the named design matrix term	
	const casa::Matrix<casa::Complex>& derivative(const string& name) const;

	/// Return the residual vector
	const casa::Vector<casa::Complex>& residual() const;

	/// Return the weight vector
	const casa::Vector<double>& weight() const;

	/// Return names of parameters
	vector<string> names() const;
	
	/// Return value of fit
	double fit() const;
	
	friend class MENormalEquations;
	
private:
	MEParams itsParams;
	// Design Matrix = number of parameters x number of dof/parameter x number of data points
	// The number of dof of parameters can vary from parameter to parameter
	mutable std::map<string, casa::Matrix<casa::Complex> > itsDesignMatrix;
	// Residual matrix = number of data points
	mutable casa::Vector<casa::Complex> itsResidual;
	mutable casa::Vector<double> itsWeight;
};

}
}

#endif 
