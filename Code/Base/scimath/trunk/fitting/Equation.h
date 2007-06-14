/// @file Equation.h
///
/// Equation: Represent a parametrized equation. The equation
/// can be used to calculate predicted values (predict) and
/// to evaluate the normal equations. The data are to be
/// passed in via the (derived class) constructor.
///
/// This is a base class. See PolynomialEquation for an example
/// of how to derive, and CompositeEquation for how to assemble
/// composite equations. This fitting framework has been designed
/// for synthesis calibration and imaging using the master/worker
/// framework but is also appropriate for general use.
///
/// Here's a (longwinded) example of how to use this framework
/// for fitting a polynomial equation.
///
///    casa::Vector<double> arguments(10);
///    casa::Vector<double> data(10);
///    casa::Vector<double> weights(10);
///    casa::Vector<double> model(10);
///
///    for (uint i=0;i<arguments.size();i++) {
///        arguments[i]=i;
///    }
///    data.set(0.0);
///    weights.set(1.0);
///    model.set(0.0);
///
///    Params ip;
///    casa::Vector<double> quadratic(3);
///    quadratic(0)=1;
///    quadratic(1)=2;
///    quadratic(2)=3;
///    ip.add("poly", quadratic);
///
///    PolynomialEquation poly(ip, data, weights, arguments, model);
///    poly.predict();
///    quadratic.set(0.0);
///    ip.update("poly", quadratic);
///
///    NormalEquations normeq(ip);
///    poly.calcEquations(normeq);
///
///    LinearSolver solver(ip);
///    solver.addNormalEquations(normeq);
///    Quality q;
///    solver.setAlgorithm("SVD");
///    solver.solveNormalEquations(q);
///
/// The class PolynomialEquation holds the C++ code responsible for
/// calculating values and derivatives of the specific polynomial.
///
/// The overall concept of this set of classes is similar to that
/// of the MeqTree package from ASTRON with a couple of notable
/// exceptions:
///   - For large number of parameters, one has the option of calculating
/// and keeping only subsections of the full normal equations
///   - The equations are hard-coded to specific high level mathematical
/// relationships rather than being composed from a tree.
///
/// These two changes are needed to allow imaging.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef SCIMATHEQUATION_H
#define SCIMATHEQUATION_H

#include <fitting/Params.h>
#include <fitting/NormalEquations.h>

#include <boost/shared_ptr.hpp>

namespace conrad
{

  namespace scimath
  {

    class Equation
    {
      public:

/// Construct using default parameters
        Equation();

/// Construct using specified parameters
        explicit Equation(const Params& ip);

/// Copy constructor
        Equation(const Equation& other);

/// Assignment operator
        Equation& operator=(const Equation& other);

        virtual ~Equation();

/// Access the parameters
        const Params& parameters() const;
        Params& parameters();

/// Set the parameters to new values
/// @param ip Parameters
        virtual void setParameters(const Params& ip);

/// Check if a set of parameters is complete for this equation
/// i.e. are all the required parameters present
/// @param ip Parameters
        virtual bool complete(const Params& ip);

/// Return a default set of parameters
/// @param ip Parameters
        const Params& defaultParameters() const;
        Params& defaultParameters();

/// Predict the data from the parameters. This changes the internal state.
        virtual void predict() {};

/// Calculate the normal equations for the given data and parameters
/// @param ne Normal equations to be filled
        virtual void calcEquations(NormalEquations& ne) {};

/// Shared pointer definition
        typedef boost::shared_ptr<Equation> ShPtr;

/// Clone this into a shared pointer
        virtual Equation::ShPtr clone();

      protected:
        Params itsParams;
        Params itsDefaultParams;
    };

  }
}
#endif
