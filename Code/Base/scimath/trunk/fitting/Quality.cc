/// @file
///
/// Captures quality of a solution from a Solver
///
/// (c) 2007 ASKAP, All Rights Reserved.
/// @author Tim Cornwell tim.cornwel@csiro.au
///

#include <fitting/Quality.h>

namespace askap
{
  namespace scimath
  {

    Quality::Quality() : itsCond(0.0), itsRank(0), itsDOF(0), itsInfo("")
    {
    }

    Quality::~Quality()
    {
    }

    std::ostream& operator<<(std::ostream& os, const Quality& q)
    {
      if(q.info()!="") {
        os << "Solution : " << q.info();
      }
      if(q.DOF()>0)
      {
        os << " : degrees of freedom " << q.DOF();
      }
      if(q.rank()>0)
      {
        os << ", rank = " << q.rank();
      }
      if(q.cond()>0.0)
      {
        os << ", condition number = " << q.cond();
      }
      return os;
    }

  }
}
