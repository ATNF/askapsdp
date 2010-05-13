/// @file
///
/// Equations hold equations for the fitting classes
///
/// @copyright (c) 2007 CSIRO
/// Australia Telescope National Facility (ATNF)
/// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
/// PO Box 76, Epping NSW 1710, Australia
/// atnf-enquiries@csiro.au
///
/// This file is part of the ASKAP software distribution.
///
/// The ASKAP software distribution is free software: you can redistribute it
/// and/or modify it under the terms of the GNU General Public License as
/// published by the Free Software Foundation; either version 2 of the License,
/// or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
///
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#include <fitting/Params.h>
#include <fitting/Equation.h>
#include <askap/AskapError.h>

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
    const Params& Equation::parameters() const {
      ASKAPDEBUGASSERT(itsParams);
      return *itsParams;
    };

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
