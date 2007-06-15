/// @file
///
/// PolynomialEquation: Represent a polynomial equation.
/// The parameters of the polynomial are supplied via the
/// Params class. The data and arguments are also part
/// of the constructor.
///
/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef SCIMATHPOLYEQUATION_H
#define SCIMATHPOLYEQUATION_H

#include <fitting/Equation.h>
#include <fitting/NormalEquations.h>

namespace conrad
{

  namespace scimath
  {
    /// Represent a polynomial of arbitrary degree
    class PolynomialEquation : public Equation
    {
      public:

/// Constructor for real use
/// @param ip Coefficients of polynomial stored with names poly.*
/// @param data Data constraints
/// @param weights Weights for data
/// @param arguments Arguments for the polynomial
/// @param model Model (to be calculated)
        PolynomialEquation(const Params& ip, casa::Vector<double>& data,
          casa::Vector<double>& weights, casa::Vector<double>& arguments,
          casa::Vector<double>& model);

/// Constructor using default parameters
/// @param data Data constraints
/// @param weights Weights for data
/// @param arguments Arguments for the polynomial
/// @param model Model (to be calculated)
        PolynomialEquation(casa::Vector<double>& data,
          casa::Vector<double>& weights, casa::Vector<double>& arguments,
          casa::Vector<double>& model);

/// Copy constructor
        PolynomialEquation(const PolynomialEquation& other);

/// Assignment operator
        PolynomialEquation& operator=(const PolynomialEquation& other);

/// Destructor
        virtual ~PolynomialEquation();

/// Predict the model data
        virtual void predict();

/// Calculate the normal equations
/// @param ne Normal equations
        virtual void calcEquations(NormalEquations& ne);

/// Clone this
        virtual Equation::ShPtr clone();

      protected:
        /// Initialize
        void init();
        /// Calculate the polynomial
        /// @param arguments Arguments to the equation i.e. set of values x
        /// @param parameters Parameters to the equation
        /// @param values Returned values i.e. set of f(x;P) for all x
        void calcPoly(const casa::Vector<double>& arguments, 
          const casa::Vector<double>& parameters,
          casa::Vector<double>& values);

        /// Calculate the polynomial derivatives
        /// @param arguments Arguments to the equation i.e. set of values x
        /// @param parameters Parameters to the equation
        /// @param valueDerivs Returned derivatives i.e. set of df(x;P)/dP for all x and P
        void calcPolyDeriv(const casa::Vector<double>& arguments, const casa::Vector<double>& parameters,
          casa::Matrix<double>& valueDerivs);
        /// Data
        casa::Vector<double> itsData;
        /// Weights
        casa::Vector<double> itsWeights;
        /// Arguments
        casa::Vector<double> itsArguments;
        /// Model
        casa::Vector<double> itsModel;
    };

  }
}
#endif
