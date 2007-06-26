#include <fitting/Quality.h>

namespace conrad
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
    }

  }
}
