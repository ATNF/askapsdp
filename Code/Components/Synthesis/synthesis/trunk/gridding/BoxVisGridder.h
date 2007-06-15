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
	BoxVisGridder();
	
	virtual ~BoxVisGridder();
    
protected:
    virtual int cOffset(int, int);
    virtual void initConvolutionFunction(IDataSharedIter& idi, const casa::Vector<double>& cellSize,
        const casa::IPosition& shape);
};

}
}
#endif
