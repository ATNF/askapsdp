/// @file
///
/// Solves equations from normal equations
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

#include <fitting/Solver.h>
#include <askap/AskapError.h>

namespace askap
{
  namespace scimath
  {

    /// @brief default constructor
    Solver::Solver() {};
    
    Solver::Solver(const Solver& other) : Solveable(other) {
        if (other.itsNormalEquations) {
            itsNormalEquations = other.itsNormalEquations->clone();
        }
    }
    
    /// @brief assignment operator
    /// @param[in] other solver to take the data from
    /// @return reference to itself
    Solver& Solver::operator=(const Solver& other)  {
        Solveable::operator=(other);
        if (other.itsNormalEquations) {
            itsNormalEquations = other.itsNormalEquations->clone();
        }    
        return *this;
    }
    
    /// @return a reference to normal equations object
    const INormalEquations& Solver::normalEquations() const
    {
      ASKAPDEBUGASSERT(itsNormalEquations);
      return *itsNormalEquations;
    }
    
    /// @brief reset normal equations
    void Solver::resetNormalEquations() const
    {
      if (itsNormalEquations) {
          itsNormalEquations->reset();   
      }
    }
    
    
    void Solver::copyNormalEquations(const Solver& other)
    {
      ASKAPCHECK(other.itsNormalEquations, "NormalEquations not defined in other Solver");
      itsNormalEquations=other.normalEquations().clone();
      ASKAPCHECK(itsNormalEquations, "NormalEquations not defined in Solver after copy");
    }

    void Solver::addNormalEquations(const INormalEquations& normeq)
    { 
      if (itsNormalEquations) {
          itsNormalEquations->merge(normeq);
      } else { 
          itsNormalEquations = normeq.clone();
      }
    }
    
    /// @brief solve for parameters
    /// The solution is constructed from the normal equations and given
    /// parameters are updated
    /// @param[in] params parameters to be updated 
    /// @param[in] q Quality of solution
    bool Solver::solveNormalEquations(Params &params, Quality& q)
    {
     return false;
    }
    
    Solver::ShPtr Solver::clone() const
    {
      return Solver::ShPtr(new Solver(*this));
    }
    
    void Solver::init()
    {
      if (itsNormalEquations) {
         itsNormalEquations->reset();
       	 ASKAPCHECK(itsNormalEquations, "NormalEquations not defined in Solver after reset");
      }
    }
    
  }
}
