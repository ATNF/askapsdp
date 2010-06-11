/// @file
///
/// Provides a composite equation assembled of other equations
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
