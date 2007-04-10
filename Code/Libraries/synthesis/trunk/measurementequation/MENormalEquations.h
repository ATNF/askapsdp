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
	
class MERegularNormalEquations
{
public:
	MERegularNormalEquations(const MERegularParams& ip);
	virtual ~MERegularNormalEquations();
	virtual void merge(const MERegularNormalEquations& other);
	virtual void reset();
};

class MEImageNormalEquations
{
public:
	MEImageNormalEquations(const MEImageParams& ip);
	virtual ~MEImageNormalEquations();
	virtual void merge(const MEImageNormalEquations& other);
	virtual void reset();
};

}
}
#endif /*MENORMALEQUATIONS_H_*/
