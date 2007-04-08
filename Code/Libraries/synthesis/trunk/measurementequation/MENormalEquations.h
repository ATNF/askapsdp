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
	
class MEParams;

class MENormalEquations
{
public:
	MENormalEquations(const MEParams& ip);
	virtual ~MENormalEquations();
	virtual void merge(const MENormalEquations& other);
	virtual void reset();
};

}
}
#endif /*MENORMALEQUATIONS_H_*/
