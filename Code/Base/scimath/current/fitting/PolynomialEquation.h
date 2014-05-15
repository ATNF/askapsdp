/// @file
///
/// PolynomialEquation: Represent a polynomial equation.
/// The parameters of the polynomial are supplied via the
/// Params class. The data and arguments are also part
/// of the constructor.
///
/// This is mostly a demonstration quality class - it needs
/// to be optimized for real use.
///
/// @todo Use axes if present to normalize before fitting
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
#ifndef SCIMATHPOLYEQUATION_H
#define SCIMATHPOLYEQUATION_H

#include <fitting/GenericEquation.h>

namespace askap
{

  namespace scimath
  {
    /// Represent a polynomial of arbitrary degree
    /// @ingroup fitting
    class PolynomialEquation : public GenericEquation
    {
      public:

/// Return a default set of parameters
        static Params defaultParameters();

/// Constructor for real use: Note that the arguments are NOT
/// const - the values are updated in place.
/// @param ip Coefficients of polynomial stored with names poly.*
/// @param data Data constraints
/// @param weights Weights for data
/// @param arguments Arguments for the polynomial
/// @param model Model (to be calculated)
        PolynomialEquation(const Params& ip, casa::Vector<double>& data,
          casa::Vector<double>& weights, casa::Vector<double>& arguments,
          casa::Vector<double>& model);

/// Constructor using default parameters: Note that the arguments are NOT
/// const - the values are updated in place.
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
        virtual void predict() const;

/// Calculate the normal equations
/// @param ne Normal equations
        virtual void calcGenericEquations(GenericNormalEquations& ne) const;

/// Clone this
        virtual Equation::ShPtr clone() const;

      protected:
        /// Initialize
        void init();
        /// Calculate the polynomial
        /// @param arguments Arguments to the equation i.e. set of values x
        /// @param parameters Parameters to the equation
        /// @param values Returned values i.e. set of f(x;P) for all x
        static void calcPoly(const casa::Vector<double>& arguments, 
          const casa::Vector<double>& parameters,
          casa::Vector<double>& values);

        /// Calculate the polynomial derivatives
        /// @param arguments Arguments to the equation i.e. set of values x
        /// @param parameters Parameters to the equation
        /// @param valueDerivs Returned derivatives i.e. set of df(x;P)/dP for all x and P
        static void calcPolyDeriv(const casa::Vector<double>& arguments, const casa::Vector<double>& parameters,
          casa::Matrix<double>& valueDerivs);
        /// Data
        casa::Vector<double> itsData;
        /// Weights
        casa::Vector<double> itsWeights;
        /// Arguments
        casa::Vector<double> itsArguments;
        /// Model
        mutable casa::Vector<double> itsModel;
    };

  }
}
#endif
