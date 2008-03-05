/// @file
///
/// Equations hold equations for the fitting classes
///
/// (c) 2007 ASKAP, All Rights Reserved.
/// @author Tim Cornwell tim.cornwel@csiro.au
///
#include <fitting/Params.h>
#include <fitting/Equation.h>

#include <stdexcept>

namespace askap
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
        rwParameters()=other.itsParams;
      }
      return *this;
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
    void Equation::setParameters(const Params& ip) 
    {
       rwParameters()=ip.clone();
    }

    /// @brief non-const reference to paramters
    /// @details Due to caching, derived classes may need to know when
    /// the parameters of the equation have been updated. To track all
    /// updates, itsParams is made private. All changes to parameters are
    /// done via this method (including setParameters exposed to the user).
    Params::ShPtr& Equation::rwParameters() throw() 
    {
       return itsParams;
    }

  }

}
