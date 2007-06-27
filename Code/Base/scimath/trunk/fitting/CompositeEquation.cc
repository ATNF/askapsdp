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
        itsList=other.itsList;
      }
    }

    CompositeEquation::CompositeEquation() : Equation(Params()) {};

    CompositeEquation::~CompositeEquation(){};
    
    Params CompositeEquation::defaultParameters()
    {
      Params ip;
      return ip;
    }

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

    void CompositeEquation::add(const Equation& eq)
    {
      itsParams->merge(eq.parameters());
      itsList.push_back(eq.clone());
    }

    Equation::ShPtr CompositeEquation::clone()
    {
      return Equation::ShPtr(new CompositeEquation(*this));
    }
  }

}
