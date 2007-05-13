/// @file
///
/// DesignMatrix: Hold the design matrix for parameters
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef SCIMATHDESIGNMATRIX_H
#define SCIMATHDESIGNMATRIX_H

#include <map>
#include <vector>

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>

#include <fitting/Params.h>

namespace conrad {
namespace scimath {
	
class NormalEquations;
	
typedef std::vector<casa::Matrix<casa::Double> > DMAMatrix;
typedef std::vector<casa::Vector<casa::Double> > DMBVector;
typedef std::vector<casa::Vector<casa::Double> > DMWeight;

class DesignMatrix
{
public:

	DesignMatrix() {};
	
	/// Define a design matrix
	/// @param ip Parameters
	DesignMatrix(const Params& ip);
	
	/// Copy constructor
	DesignMatrix(const DesignMatrix& dm);
	
	/// Assignment operator
	DesignMatrix& operator=(const DesignMatrix& dm);
	
	virtual ~DesignMatrix();
	
	/// Merge this design matrix with another - means that we just
	/// need to append on the data axis
	/// @param other Other design matrix
	void merge(const DesignMatrix& other);
	
	/// Add the derivative of the data with respect to dof of the named parameter
	/// @param name Name of parameter
	/// @param deriv Derivative
	void addDerivative(const string& name, const casa::Matrix<casa::Double>& deriv);
	
	/// Add the residual constraint
	/// @param residual Residual vector
	/// @param weight Weight vector
	void addResidual(const casa::Vector<casa::Double>& residual, const casa::Vector<double>& weight);
	
	/// Reset to empty
	void reset();

	/// Return the specified parameters
	const Params& parameters() const;
	Params& parameters();
		
	/// Return the list of named design matrix terms
	DMAMatrix derivative(const string& name) const;

	/// Return list of the residual vectors
	DMBVector residual() const;

	/// Return lists of the weight vector
	DMWeight weight() const;

	/// Return names of parameters
	vector<string> names() const;
	
	/// Return value of fit
	double fit() const;
	
	// Return number of data constraints
	uint nData() const;
	
	// Return number of parameters
	uint nParameters() const;
	
	friend class NormalEquations;
	
private:
	Params itsParams;
	// Design Matrix = number of parameters x number of dof/parameter x number of data points
	// The number of dof of parameters can vary from parameter to parameter
	mutable std::map<string, DMAMatrix > itsAMatrix;
	// Residual matrix = number of data points
	mutable DMBVector itsBVector;
	mutable DMWeight itsWeight;
};

}
}

#endif 
