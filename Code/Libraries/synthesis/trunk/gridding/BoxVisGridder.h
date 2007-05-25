/// @file
///
/// BoxVisGridder: Box-based visibility gridder. 
///
/// This supports gridders with a table loopkup.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef BOXVISGRIDDER_H_
#define BOXVISGRIDDER_H_

#include <gridding/TableVisGridder.h>

namespace conrad
{
namespace synthesis
{

class BoxVisGridder : public TableVisGridder
{
public:
	
	// Standard two dimensional gridding
	BoxVisGridder(IDataSharedIter& idi);
	
	virtual ~BoxVisGridder();
    
protected:
    int cOffset(int, int);
	
private:
	void initConvolutionFunction();
};

}
}
#endif
