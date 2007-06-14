#include <fitting/CompositeEquation.h>

#include <stdexcept>

namespace conrad
{
  namespace scimath
  {

    CompositeEquation::CompositeEquation(const CompositeEquation& other)
    {
      operator=(other);
    }

    CompositeEquation& CompositeEquation::operator=(const CompositeEquation& other)
    {
      if(this!=&other)
      {
        itsParams=other.itsParams;
        itsDefaultParams=other.itsDefaultParams;
        itsList=other.itsList;
      }
    }

/// Using specified parameters
    CompositeEquation::CompositeEquation(const Params& ip) : Equation(ip) {};

    CompositeEquation::~CompositeEquation(){};

    void CompositeEquation::predict()
    {
      for (std::list<Equation::ShPtr>::iterator it=itsList.begin();
        it!=itsList.end();it++)
      {
        (*it)->predict();
      }
    }

    void CompositeEquation::calcEquations(NormalEquations& ne)
    {
      for (std::list<Equation::ShPtr>::iterator it=itsList.begin();
        it!=itsList.end();it++)
      {
        (*it)->calcEquations(ne);
      }
    }

    void CompositeEquation::add(Equation& eq)
    {
      itsList.push_back(eq.clone());
    }

    Equation::ShPtr CompositeEquation::clone()
    {
      return Equation::ShPtr(new CompositeEquation(*this));
    }

  }

}
