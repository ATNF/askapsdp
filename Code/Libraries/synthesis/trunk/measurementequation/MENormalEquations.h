/// @file
///
/// MENormalEquations: Hold the normal equations for parameters
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef MENORMALEQUATIONS_H_
#define MENORMALEQUATIONS_H_

namespace conrad
{
namespace synthesis
{

class MENormalEquations
{
public:
	MENormalEquations();
	virtual ~MENormalEquations();
	void merge(const MENormalEquations& other);
	void reset();
};

}
}
#endif /*MENORMALEQUATIONS_H_*/
