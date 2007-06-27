/// @file
/// @brief CompositeEquation: Represent composite equations. 
///
/// CompositeEquation: Represent composite equations. This uses
/// the composite pattern to allow a set of equations to be
/// assembled and used the same as a single equation.
///
/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef SCIMATHCOMPOSITEEQUATION_H
#define SCIMATHCOMPOSITEEQUATION_H

#include <fitting/Equation.h>

#include <list>

namespace conrad
{

  namespace scimath
  {
    /// @brief A composite of Equations
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
        virtual void predict();

/// Calculate the normal equations for the given data and parameters
/// @param ne Normal equations to be filled
        virtual void calcEquations(NormalEquations& ne);

/// Clone this
        virtual CompositeEquation::ShPtr clone();

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
