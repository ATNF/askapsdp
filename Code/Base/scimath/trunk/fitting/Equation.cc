/// @file
///
/// Equations hold equations for the fitting classes
///
/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell tim.cornwel@csiro.au
///
#include <fitting/Params.h>
#include <fitting/Equation.h>

#include <stdexcept>

namespace conrad
{
  namespace scimath
  {

    Equation::Equation(const Equation& other)
    {
      operator=(other);
    }

    Equation& Equation::operator=(const Equation& other)
    {
      if(this!=&other)
      {
        itsParams=other.itsParams;
      }
    }

    Equation::Equation()
    {
    };

// Using specified parameters
    Equation::Equation(const Params& ip) : itsParams(ip.clone()) 
    {
    };

    Equation::~Equation(){};

// Access the parameters
    const Params& Equation::parameters() const {return *itsParams;};

// Set the parameters to new values
    void Equation::setParameters(const Params& ip) {itsParams=ip.clone();};

    Equation::ShPtr Equation::clone() const
    {
      return Equation::ShPtr(new Equation(*this));
    }

  }

}
