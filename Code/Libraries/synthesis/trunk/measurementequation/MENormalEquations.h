/// @file
///
/// MENormalEquations: Hold the normal equations for parameters
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef MENORMALEQUATIONS_H_
#define MENORMALEQUATIONS_H_

#include <measurementequation/MEParams.h>


namespace conrad
{
namespace synthesis
{
	
class MEDesignMatrix;
	
class MENormalEquations
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
	
	MENormalEquations() {};
	
	/// Define the normal equations
	/// @param ip Parameters
	MENormalEquations(const MEParams& ip);
	
	/// Copy constructor
	MENormalEquations(const MENormalEquations& normeq);

	/// Return the specified parameters
	const MEParams& parameters() const;
	MEParams& parameters();
		
	/// Construct the normal equations from the design matrix
	/// @param dm Design matrix
	/// @param approx Type of approximation to be used
	MENormalEquations(const MEDesignMatrix& dm, const MENormalEquations::Approximation approx);
	
	/// Assignment operator
	MENormalEquations& operator=(const MENormalEquations& normeq);
	
	virtual ~MENormalEquations();
	
	// Set the approximation
	void setApproximation(const MENormalEquations::Approximation approx);
	
	/// Merge this design matrix with another - means that we just
	/// need to append on the data axis
	/// @param other Other design matrix
	void merge(const MENormalEquations& other);
	
	/// Reset to empty
	void reset();
private:
	MEParams itsParams;
	MENormalEquations::Approximation itsApprox;
	// Note that this is a very flexible format - it allows any of the
	// enumerated approximations to be used
	std::map<string, std::map<string, casa::Array<double> > > itsConstraintMatrix;
	std::map<string, casa::Array<double> > itsDataVector;
};

}
}
#endif /*MENORMALEQUATIONS_H_*/
