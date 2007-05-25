/// @file
///
/// SphFuncVisGridder: SphFunc-based visibility gridder. 
///
/// This supports gridders with a table loopkup.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef SPHVISGRIDDER_H_
#define SPHVISGRIDDER_H_

#include <gridding/TableVisGridder.h>

namespace conrad
{
namespace synthesis
{

class SphFuncVisGridder : public TableVisGridder
{
public:
	
	// Standard two dimensional gridding
	SphFuncVisGridder(IDataSharedIter& idi);
	
	virtual ~SphFuncVisGridder();
    
protected:
    int cOffset(int, int);
	
private:
	void initConvolutionFunction();
    double grdsf(double nu);
};

}
}
#endif
