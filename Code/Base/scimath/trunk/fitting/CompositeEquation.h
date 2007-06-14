/// @file
///
/// CompositeEquation: Represent composite equations. This uses
/// the composite pattern to allow a set of equations to be
/// assembled and used the same as a single equation.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
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

    class CompositeEquation : public Equation
    {
      public:
/// Constructor

/// Using specified parameters
/// @param ip Parameters
        CompositeEquation(const Params& ip);

/// Copy constructor
        CompositeEquation(const CompositeEquation& other);

/// Assignment operator
        CompositeEquation& operator=(const CompositeEquation& other);

        virtual ~CompositeEquation();

/// Predict the data from the parameters. This changes the internal state.
        virtual void predict();

/// Calculate the normal equations for the given data and parameters
/// @param ne Normal equations to be filled
        virtual void calcEquations(NormalEquations& ne);

/// Clone this
        virtual Equation::ShPtr clone();

/// These next function is specific to the Composite
/// Add an equation
/// @param eq equation to be added
        virtual void add(Equation& eq);

      protected:
        std::list<Equation::ShPtr> itsList;
    };

  }
}
#endif
