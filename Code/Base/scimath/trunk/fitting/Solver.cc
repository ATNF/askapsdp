/// @file
///
/// Solves equations from normal equations
///
/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell tim.cornwel@csiro.au
///

#include <fitting/Solver.h>
#include <conrad/ConradError.h>

namespace conrad
{
  namespace scimath
  {

    Solver::Solver(const Params& ip) : itsParams(ip.clone())
    {
      NormalEquations ne(ip);
      itsNormalEquations=ne.clone();
    };

    Solver::~Solver()
    {
    }
    
    Solver::Solver(const Solver& other) {
    	operator=(other);
    }
    
    Solver& Solver::operator=(const Solver& other)
    {
    	if(this!=&other) 
    	{
    		itsParams=other.itsParams;
    		itsNormalEquations=other.itsNormalEquations;
    	}
    	return *this;
    }


    
    void Solver::setParameters(const Params& ip)
    {
      itsParams=ip.clone();
      NormalEquations ne(ip);
      itsNormalEquations=ne.clone();
    }
    
/// Return current values of params
    const Params& Solver::parameters() const
    {
    	CONRADCHECK(itsParams, "Params not defined in Solver");
      return *itsParams;
    };

    void Solver::copyNormalEquations(const Solver& other)
    {
    	CONRADCHECK(other.itsNormalEquations, "NormalEquations not defined in other Solver");
      itsNormalEquations=other.itsNormalEquations->clone();
    	CONRADCHECK(itsNormalEquations, "NormalEquations not defined in Solver after copy");
    }

    void Solver::addNormalEquations(const NormalEquations& normeq)
    {
    	CONRADCHECK(itsNormalEquations, "NormalEquations not defined in Solver prior to adding");
      itsNormalEquations->merge(normeq);
    }
    
    bool Solver::solveNormalEquations(conrad::scimath::Quality& q)
    {
     return false;
    }
    
    Solver::ShPtr Solver::clone() const
    {
      return Solver::ShPtr(new Solver(*this));
    }
    
    void Solver::init()
    {
    	CONRADCHECK(itsNormalEquations, "NormalEquations not defined in Solver before reset");
      itsNormalEquations->reset();
    	CONRADCHECK(itsNormalEquations, "NormalEquations not defined in Solver after reset");
    }
    
  }
}
