/// @file 
/// @brief Calculation of speroidal function 
/// @details Speroidal function is used for gridding to achieve good aliasing rejection. This class 
/// implements the algorithm of Aquino and Casta\~no (2002) to calculate prolate spheroidal function
/// and then scales it with (1-nu^2)^{-alpha/2) to obtain desired spheroidal function. The prolate
/// spheroidal function is calculated using rather brute force approach by decomposition into 
/// a series of spherical Legendre functions (more or less an extension of Legendre polynomials).
/// The main trick of the Aquino and Casta\~no (2002) method is to use right coordinates/normalisation of
/// the Legendre function prior to decomposition, which gives a symmetric tri-diagonal matrix whose 
/// eigenvalues and eigenvectors are identical to that of the generating differential equation. Without this,
/// some terms in the DE don't cancel and one has to deal with Jordan form of the matrix (other methods typically
/// extract eigenvalue from the matrix decomposition but use other recurrence relations to get eigenvectors).
/// Currently, we use a generic GSL method for symmetric matrices to solve eigenproblem. In principle, a special
/// method can be written as the matrix is tri-diagonal (but given that the size of the matrix, nterms x nterms, is
/// not big, it doesn't seem to be a priority now). Spherical Legendre functions are also calculated using GSL.
/// Double precision is used throughout this code.
///
///
/// @copyright (c) 2008 CSIRO
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
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#ifndef SCIMATH_UTILS_SPEROIDAL_FUNCTION_H
#define SCIMATH_UTILS_SPEROIDAL_FUNCTION_H

// casa includes
#include <casa/BasicSL/Constants.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>


namespace askap {

namespace scimath {

/// @brief Calculation of speroidal function 
/// @details Speroidal function is used for gridding to achieve good aliasing rejection. This class 
/// implements the algorithm of Aquino and Casta\~no (2002) to calculate prolate spheroidal function
/// and then scales it with (1-nu^2)^{-alpha/2) to obtain desired spheroidal function. The prolate
/// spheroidal function is calculated using rather brute force approach by decomposition into 
/// a series of spherical Legendre functions (more or less an extension of Legendre polynomials).
/// The main trick of the Aquino and Casta\~no (2002) method is to use right coordinates/normalisation of
/// the Legendre function prior to decomposition, which gives a symmetric tri-diagonal matrix whose 
/// eigenvalues and eigenvectors are identical to that of the generating differential equation. Without this,
/// some terms in the DE don't cancel and one has to deal with Jordan form of the matrix (other methods typically
/// extract eigenvalue from the matrix decomposition but use other recurrence relations to get eigenvectors).
/// Currently, we use a generic GSL method for symmetric matrices to solve eigenproblem. In principle, a special
/// method can be written as the matrix is tri-diagonal (but given that the size of the matrix, nterms x nterms, is
/// not big, it doesn't seem to be a priority now). Spherical Legendre functions are also calculated using GSL.
/// Double precision is used throughout this code.
/// @ingroup utils
struct SpheroidalFunction {
 
   /// @brief Constructor of the object
   /// @details Set parameters of the function required and precompute decomposition into spheroidal Legendre 
   /// function series with the given number of terms. The calculation is done via prolate spheroidal function.
   /// This class implements the relatively brute force approach of Aquino and Casta\~no (2002).
   /// @param[in] c parameter c of the spheroidal function (bandwidth or a measure of the support size in our case)
   /// @param[in] alpha parameter alpha of the spheroidal function (weighting exponent in our case)
   /// @param[in] nterms number of terms in the decomposition
   SpheroidalFunction(const double c, const double alpha, const casa::uInt nterms = 16);
   
   /// @brief copy constructor to deal with casa vector
   /// @param[in] other an instance to copy from
   SpheroidalFunction(const SpheroidalFunction &other);

   /// @brief value of the function for argument nu
   /// @details
   /// @param[in] nu argument of the function
   /// @return value of the function 
   double operator()(const double nu) const;
   
protected:

   /// @brief sum of the Legendre series
   /// @details This helper method sums Legendre series for the given coefficients and the origin
   /// @param[in] x abcissa 
   /// @param[in] m parameter m of the Legendre function (corresponding to the resulting Smn(c,eta))
   /// @note coefficients are taken from itsCoeffs, element index r is incremented by two, interpretation
   /// of r depends on itsREven flag, which is true if series starts from r=0, false if from r=1 (n-m of Smn 
   /// is even or odd). Note. that currently this class is only used to generate spheroidal functions
   /// psi_{alpha 0}, i.e. corresponding to a single eigenvector associated with the smallest eigenvalue, so
   /// n in Smn is always equal to m and itsREven is always true.
   double sumLegendreSeries(const double x, const int m) const;
   
   /// @brief fill matrix which has the same eigenvalues/vectors as the original problem
   /// @details See equation (20) in Aquino and Casta\~no (2002)
   /// @param[out] B matrix to fill (should already be sized to the required number of terms)
   /// @param[in] c bandwidth of the prolate spheroidal function
   /// @param[in] m parameter m of the prolate spheroidal function Smn(c,eta)
   /// @note This class is only used to generate spheroidal functions  psi_{alpha 0}, i.e. 
   /// those corresponding to a single eigenvector associated with the smallest eigenvalue, so
   /// n in Smn is always equal to m and itsREven is always true. In addition m = alpha. It is
   /// passed as an additional parameter for generality.  
   void fillHelperMatrix(casa::Matrix<double> &B, const double c, const int m) const;
  
   /// @brief coefficients in Legendre series
   /// @details This method solves eigenvalue problem and obtains eigenvector corresponding to
   /// the smallest eigenvalue (for function Smn(c,eta) this means n=0). Coefficients are in the same order
   /// as elements of matrix B, i.e. in steps of 2 starting from even or odd depending whether n-m is even or odd
   /// @param[in] B matrix to solve
   /// @return eigenvalue
   /// @note an exception is thrown if there is an error solving eigensystem. itsCoeffs vector is resized
   /// to match the input matrix and filled with coefficients for Legendre series 
   double fillLegendreCoeffs(const casa::Matrix<double> &B);
   
private:
   /// @brief coefficients of the Legendre function series
   casa::Vector<double> itsCoeffs;
   
   /// @brief true, if series starts from r=0, false if from r=1 (n-m of Smn is even or odd)
   bool itsREven;
   
   /// @brief parameter alpha of the spheroidal function (weighting exponent in our case)
   double itsAlpha;
   
   /// @brief series value at nu=0 (for normalisation)
   double itsSum0;
};

} // namespace scimath

} // namespace askap

#endif // #ifndef SCIMATH_UTILS_SPEROIDAL_FUNCTION_H


