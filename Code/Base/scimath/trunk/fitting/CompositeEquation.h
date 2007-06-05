/// @file
///
/// CompositeEquation: Represent composite equations. This uses
/// the composite pattern to allow a set of equations to be
/// assembled and used the same as a single equation. 
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef SCIMATHCOMPOSITEEQUATION_H
#define SCIMATHCOMPOSITEEQUATION_H

#include <fitting/Equation.h>

#include <list>

namespace conrad { 

namespace scimath
{

class CompositeEquation : public Equation {
public:	
	/// Constructor
	/// Using default parameters
    CompositeEquation() : Equation() {};
    
    /// Using specified parameters
    /// @param ip Parameters
    CompositeEquation(const Params& ip);
    
    CompositeEquation(const CompositeEquation& other);
    
    CompositeEquation& operator=(const CompositeEquation& other);
    
    virtual ~CompositeEquation();
    
    virtual void predict();
    
    ///
    virtual void calcEquations(NormalEquations& ne);
    
    virtual void add(Equation& eq);
    
    virtual void remove(Equation& eq);
    
protected:
    std::list<Equation*> itsList;
};

}
}
#endif





