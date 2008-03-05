/// @file
///
/// Solves equations from normal equations
///
/// (c) 2007 ASKAP, All Rights Reserved.
/// @author Tim Cornwell tim.cornwel@csiro.au
///

#include <fitting/Solver.h>
#include <askap/AskapError.h>

namespace askap
{
  namespace scimath
  {

    Solver::Solver(const Params& ip) : itsParams(ip.clone())
    {
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

    /// @return a reference to normal equations object
    const INormalEquations& Solver::normalEquations() const
    {
      ASKAPDEBUGASSERT(itsNormalEquations);
      return *itsNormalEquations;
    }
    
    /// @brief reset normal equations
    void Solver::resetNormalEquations() const
    {
      if (itsNormalEquations) {
          itsNormalEquations->reset();   
      }
    }
    
    
    void Solver::setParameters(const Params& ip)
    {
      itsParams=ip.clone();
    }
    
/// Return current values of params
    const Params& Solver::parameters() const
    {
    	ASKAPCHECK(itsParams, "Params not defined in Solver");
      return *itsParams;
    };

    void Solver::copyNormalEquations(const Solver& other)
    {
    	ASKAPCHECK(other.itsNormalEquations, "NormalEquations not defined in other Solver");
      itsNormalEquations=other.itsNormalEquations->clone();
    	ASKAPCHECK(itsNormalEquations, "NormalEquations not defined in Solver after copy");
    }

    void Solver::addNormalEquations(const INormalEquations& normeq)
    { 
      if (itsNormalEquations) {
          itsNormalEquations->merge(normeq);
      } else { 
          itsNormalEquations = normeq.clone();
      }
    }
    
    bool Solver::solveNormalEquations(askap::scimath::Quality& q)
    {
     return false;
    }
    
    Solver::ShPtr Solver::clone() const
    {
      return Solver::ShPtr(new Solver(*this));
    }
    
    void Solver::init()
    {
      if (itsNormalEquations) {
         itsNormalEquations->reset();
       	 ASKAPCHECK(itsNormalEquations, "NormalEquations not defined in Solver after reset");
      }
    }
    
  }
}
