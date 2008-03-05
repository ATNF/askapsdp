/// @file
///
/// Provides a composite equation assembled of other equations
///
/// (c) 2007 ASKAP, All Rights Reserved.
/// @author Tim Cornwell tim.cornwel@csiro.au
///
#include <fitting/CompositeEquation.h>

#include <stdexcept>

namespace askap
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
        static_cast<Equation*>(this)->operator=(other);
        itsList=other.itsList;
      }
      return *this;
    }

    CompositeEquation::CompositeEquation() : Equation(Params()) {};

    CompositeEquation::~CompositeEquation(){};
    
    Params CompositeEquation::defaultParameters()
    {
      Params ip;
      return ip;
    }

    void CompositeEquation::predict() const
    {
      for (std::list<Equation::ShPtr>::const_iterator it=itsList.begin();
        it!=itsList.end();it++)
      {
        (*it)->predict();
      }
    }

    void CompositeEquation::calcEquations(INormalEquations& ne) const
    {
      for (std::list<Equation::ShPtr>::const_iterator it=itsList.begin();
        it!=itsList.end();it++)
      {
        (*it)->calcEquations(ne);
      }
    }

    void CompositeEquation::add(const Equation& eq)
    {
      rwParameters()->merge(eq.parameters());
      itsList.push_back(eq.clone());
    }

    Equation::ShPtr CompositeEquation::clone() const
    {
      return Equation::ShPtr(new CompositeEquation(*this));
    }
  }

}
