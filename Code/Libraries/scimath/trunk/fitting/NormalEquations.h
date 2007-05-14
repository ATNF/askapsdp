/// @file
///
/// NormalEquations: Hold the normal equations for parameters
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef SCIMATHNORMALEQUATIONS_H_
#define SCIMATHNORMALEQUATIONS_H_

#include <fitting/Params.h>

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>

namespace conrad
{
namespace scimath
{
	
class DesignMatrix;
	
class NormalEquations
{
public:

	// Enumerate the types of approximations used in holding the
	// normal equations for non-scalar parameters
	enum Approximation {
		COMPLETE=0,  // All cross terms (inter and intra) are retained
		DIAGONAL_COMPLETE, // No inter-parameter are retained
		DIAGONAL_SLICE, // No inter-parameter cross terms and single plane (PSF) intra-parameter
		DIAGONAL_DIAGONAL // Only diagonal terms are kept
	};
	
	NormalEquations() {};
	
	/// Define the normal equations
	/// @param ip Parameters
	NormalEquations(const Params& ip);
	
	/// Copy constructor
	NormalEquations(const NormalEquations& normeq);

    /// Assignment operator
    NormalEquations& operator=(const NormalEquations& normeq);
    
    virtual ~NormalEquations();
    
	/// Return the specified parameters
	const Params& parameters() const;
	Params& parameters();
		
    /// Construct the normal equations from the design matrix
    /// @param dm Design matrix
    /// @param approx Type of approximation to be used
    NormalEquations(const DesignMatrix& dm, const NormalEquations::Approximation approx);
    
    /// Add the design matrix to the normal equations
    /// @param dm Design matrix
    /// @param approx Type of approximation to be used
    void add(const DesignMatrix& dm, const NormalEquations::Approximation approx);
    
	// Set the approximation
	void setApproximation(const NormalEquations::Approximation approx);
	
	/// Merge these normal equations with another - means that we just add
	/// @param other Other normal equations
	void merge(const NormalEquations& other);
	
	/// Return normal equations
	std::map<string, std::map<string, casa::Matrix<double> > > normalMatrix() const;
	
	/// Return data vector
	std::map<string, casa::Vector<double> > dataVector() const;
	
	/// Reset to empty
	void reset();
private:
	Params itsParams;
	NormalEquations::Approximation itsApprox;
	// Note that this is a very flexible format - it allows any of the
	// enumerated approximations to be used
	mutable std::map<string, std::map<string, casa::Matrix<double> > > itsNormalMatrix;
	mutable std::map<string, casa::Vector<double> > itsDataVector;
};

}
}
#endif /*NORMALEQUATIONS_H_*/
