/// @file Equation.h
///
/// Equation: Represent a parametrized equation. The equation
/// can be used to calculate predicted values (predict) and
/// to evaluate the normal equations. The data are to be
/// passed in via the (derived class) constructor. Hence
/// the arguments should not be declared as const.
///
/// This is a base class. See PolynomialEquation for an example
/// of how to derive, and CompositeEquation for how to assemble
/// composite equations. This fitting framework has been designed
/// for synthesis calibration and imaging using the master/worker
/// framework but is also appropriate for general use.
///
/// Implementors of derived classes are encouraged to use shared
/// pointers rather than copies. CASA::Arrays can be used as is
/// since they are by reference.
///
/// Here's a (longwinded) example of how to use this framework
/// for fitting a polynomial equation.
/// @code
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
///    GenericNormalEquations normeq;
///    poly.calcEquations(normeq);
///
///    LinearSolver solver(ip);
///    solver.addNormalEquations(normeq);
///    Quality q;
///    solver.setAlgorithm("SVD");
///    solver.solveNormalEquations(q);
/// @endcode
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
/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef SCIMATHEQUATION_H
#define SCIMATHEQUATION_H

#include <fitting/Params.h>
#include <fitting/INormalEquations.h>

#include <boost/shared_ptr.hpp>

namespace conrad
{

  namespace scimath
  {
    /// Representa parameterised equation
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
        
/// Destructor
        virtual ~Equation();

/// Access the parameters (const)
        const Params& parameters() const;

/// Set the parameters to new values
/// @param ip Parameters
        virtual void setParameters(const Params& ip);

/// Predict the data from the parameters.
        virtual void predict() {} ;

/// Calculate the normal equations for the given data and parameters
/// @param ne Normal equations to be filled
        virtual void calcEquations(INormalEquations& ne) {};

/// Shared pointer definition
        typedef boost::shared_ptr<Equation> ShPtr;

/// Clone this into a shared pointer
/// @return shared pointer to a copy
        virtual Equation::ShPtr clone() const = 0;
    protected:
      /// @brief non-const reference to paramters
      /// @details Due to caching, derived classes may need to know when
      /// the parameters of the equation have been updated. To track all
      /// updates, itsParams is made private. All changes to parameters are
      /// done via this method (including setParameters exposed to the user).
      virtual Params::ShPtr& rwParameters() throw();
      
    private:
      /// Parameters
        Params::ShPtr itsParams;
    };

  }
}
#endif
