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
	// normal equations
	enum Type {
		COMPLETE=0,  // All cross terms (inter and intra) are retained
		DIAGONAL,    // No inter-parameter or intra-parameter cross terms are retained
		DIAGONAL_PSF // No inter-parameter cross terms and single plane (PSF) intra-parameter
	};
	
	MENormalEquations() {};
	
	/// Define the normal equations
	/// @param ip Parameters
	MENormalEquations(const MEParams& ip);
	
	/// Copy constructor
	MENormalEquations(const MENormalEquations& normeq);
	
	/// Construct the normal equations from the design matrix
	/// @param dm Design matrix
	/// @param type Type of approximation to be used
	MENormalEquations(const MEDesignMatrix& dm, const MENormalEquations::Type type);
	
	/// Assignment operator
	MENormalEquations& operator=(const MENormalEquations& normeq);
	
	virtual ~MENormalEquations();
	
	// Set the type
	void setType(const MENormalEquations::Type type);
	
	/// Merge this design matrix with another - means that we just
	/// need to append on the data axis
	/// @param other Other design matrix
	void merge(const MENormalEquations& other);
	
	/// Reset to empty
	void reset();
private:
	MENormalEquations::Type itsType;
	// Note that this is a very flexible format - it allows any of the
	// enumerated approximations to be used
	std::map<string, std::map<string, casa::Array<double> > > itsConstraintMatrix;
	std::map<string, casa::Array<double> > itsDataVector;
};

}
}
#endif /*MENORMALEQUATIONS_H_*/
