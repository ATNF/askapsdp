/// @file
/// @brief CompositeEquation: Represent composite equations. 
///
/// CompositeEquation: Represent composite equations. This uses
/// the composite pattern to allow a set of equations to be
/// assembled and used the same as a single equation.
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
#ifndef SCIMATHCOMPOSITEEQUATION_H
#define SCIMATHCOMPOSITEEQUATION_H

#include <fitting/Equation.h>

#include <list>

namespace askap
{

  namespace scimath
  {
    /// @brief A composite of Equations
    /// @ingroup fitting
    class CompositeEquation : public Equation
    {
      public:
/// Constructor
        CompositeEquation();

/// Copy constructor
        CompositeEquation(const CompositeEquation& other);

/// Assignment operator
        CompositeEquation& operator=(const CompositeEquation& other);

        virtual ~CompositeEquation();

/// Return a default set of parameters - this doesn't make much sense
/// for a static of a composite but it doesn't matter
        static Params defaultParameters();

/// Predict the data from the parameters. This changes the internal state.
        virtual void predict() const;

/// Calculate the normal equations for the given data and parameters
/// @param ne Normal equations to be filled
        virtual void calcEquations(INormalEquations& ne) const;

/// Clone this
        virtual CompositeEquation::ShPtr clone() const;

/// @brief Add an equation to the composite
///
/// This function is specific to the Composite
/// @param eq equation to be added
        virtual void add(const Equation& eq);

      private:
      /// List of shared pointers to Equations
        std::list<Equation::ShPtr> itsList;
    };

  }
}
#endif
