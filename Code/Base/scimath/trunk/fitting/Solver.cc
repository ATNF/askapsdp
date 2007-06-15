#include <fitting/Solver.h>

namespace conrad
{
  namespace scimath
  {

    Solver::Solver(const Params& ip) : itsParams(ip), itsNormalEquations(ip)
    {
    };

    Solver::~Solver()
    {
    }
    
    void Solver::setParameters(const Params& ip)
    {
      itsParams=ip;
    }
/// Return current values of params
    const Params& Solver::parameters() const
    {
      return itsParams;
    };

/// Return current values of params
    Params& Solver::parameters()
    {
      return itsParams;
    };

    void Solver::addNormalEquations(const NormalEquations& normeq)
    {
      itsNormalEquations.merge(normeq);
    }
  }
}
