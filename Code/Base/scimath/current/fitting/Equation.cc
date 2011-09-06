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
      if(this!=&other) {
        if (other.itsParams) {
            setParameters(*other.itsParams);
        } else {
            itsParams.reset();
            // call the virtual method just in case some derived class needs to do a cache update
            rwParameters();
        }
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
       if (rwParameters()) {
           // use assignment operator of Params class, i.e. 
           // copy will happen at itsParams side and shared pointer will
           // not change (we somewhat rely on this behavior in the calibration
           // code)
           *rwParameters() = ip;
       } else {
         // current parameters are empty, clone the input parameters and setup
         // shared pointer
         itsParams = ip.clone(); 
         // call the virtual method just in case some derived class needs to do a cache update
         rwParameters();
       }
    }

    /// @brief shared pointer to paramters
    /// @details Due to caching, derived classes may need to know when
    /// the parameters of the equation have been updated. To track all
    /// updates, itsParams is made private. All changes to parameters are
    /// done via this method (including setParameters exposed to the user).
    /// @note This method allows non-const manipulation of the parameters, but
    /// not the change of the actual shared pointer class. The latter is done only
    /// inside this class and not in derived classes.
    /// @return non-const reference to the shared pointer    
    const Params::ShPtr& Equation::rwParameters() const throw() 
    {
       return itsParams;
    }

    /// @brief reference the given parameter object
    /// @details Sometimes it is handy to have a number of equations sharing exactly
    /// the same parameters and use reference semantics. A call to this method allows
    /// to reference itsParams member of this class to any given shared pointer to
    /// scimath::Params. In particular, a shared pointer can be obtained using rwParameters
    /// method. Use with caution. There is some legacy code/design which keep track of 
    /// changes in parameters by overriding rwParameters. This methanism will not work 
    /// correctly if referencing is used. Ideally, we want to convert this approach to
    /// use change monitors together with scimath::Params class.
    /// @param[in] params shared pointer to scimath::Params class to adopt in this equation
    void Equation::reference(const Params::ShPtr &params)
    {
       itsParams = params;
       // call the virtual method just in case some derived class needs to do a cache update
       rwParameters();       
    }

  }

}
