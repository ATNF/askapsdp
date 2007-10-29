/// @file
///
/// Solves equations from normal equations
///
/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell tim.cornwel@csiro.au
///

#include <fitting/Solver.h>

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
    
    void Solver::setParameters(const Params& ip)
    {
      itsParams=ip.clone();
    }
/// Return current values of params
    const Params& Solver::parameters() const
    {
      return *itsParams;
    };

    void Solver::copyNormalEquations(const Solver& other)
    {
      itsNormalEquations=other.itsNormalEquations->clone();
    }

    void Solver::addNormalEquations(const NormalEquations& normeq)
    {
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
      itsNormalEquations->reset();
    }
    
  }
}
