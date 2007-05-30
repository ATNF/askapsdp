/// @file
///
/// CompositeEquation: Represent a parametrized equation. 
///
/// This is a base class. See PolynomialCompositeEquation for an example
/// of how to derive.
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
    CompositeEquation(const Params& ip);
    
    CompositeEquation(const CompositeEquation& other);
    
    CompositeEquation& operator=(const CompositeEquation& other);
    
    virtual ~CompositeEquation();

    virtual void predict();
    
    virtual void calcEquations(NormalEquations& ne);
    
    virtual void add(Equation& eq);
    
    virtual void remove(Equation& eq);
    
protected:
    std::list<Equation*> itsList;
};

}
}
#endif





