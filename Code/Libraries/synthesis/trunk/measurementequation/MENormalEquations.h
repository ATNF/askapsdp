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
	MENormalEquations() {};
	
	/// Define the normal equations
	/// @param ip Parameters
	MENormalEquations(const MEParams& ip);
	
	/// Copy constructor
	MENormalEquations(const MENormalEquations& normeq);
	
	/// Construct the normal equations from the design matrix
	/// @param dm Design matrix
	MENormalEquations(const MEDesignMatrix& dm);
	
	/// Assignment operator
	MENormalEquations& operator=(const MENormalEquations& normeq);
	
	virtual ~MENormalEquations();
	
	/// Merge this design matrix with another - means that we just
	/// need to append on the data axis
	/// @param other Other design matrix
	void merge(const MENormalEquations& other);
	
	/// Reset to empty
	void reset();
private:
	std::map<string, std::map<string, casa::Array<double> > > itsConstraintMatrix;
	std::map<string, casa::Array<double> > itsDataVector;
};

}
}
#endif /*MENORMALEQUATIONS_H_*/
