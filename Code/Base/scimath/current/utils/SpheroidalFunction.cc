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

#include <utils/SpheroidalFunction.h>
#include <askap/AskapError.h>

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_eigen.h>
#include <gsl/gsl_sf.h>

using namespace askap;
using namespace askap::scimath;

/// @brief value of the function for argument nu
/// @details
/// @param[in] nu argument of the function
/// @return value of the function 
double SpheroidalFunction::operator()(const double nu) const
{
  if (casa::abs(nu)>=1.) {
      // Legendre series expansion doesn't allow to compute values for nu outside [-1,1] interval. However,
      // approximation is good enough (for a reasonable number of terms), so it is quite close to 0.
      return 0.;
  }
  
  // force normalisation to 1. at eta=0., functions corresponding to n=0 are even, so such normalisation
  // should not cause any problems
  const double res = sumLegendreSeries(nu,itsAlpha) / itsSum0;
  return res * pow(1.-nu*nu, -itsAlpha/2.);
}

/// @brief Constructor of the object
/// @details Set parameters of the function required and precompute decomposition into spheroidal Legendre 
/// function series with the given number of terms. The calculation is done via prolate spheroidal function.
/// This class implements the relatively brute force approach of Aquino and Casta\~no (2002).
/// @param[in] c parameter c of the spheroidal function (bandwidth or a measure of the support size in our case)
/// @param[in] alpha parameter alpha of the spheroidal function (weighting exponent in our case)
/// @param[in] nterms number of terms in the decomposition
SpheroidalFunction::SpheroidalFunction(const double c, const double alpha, const casa::uInt nterms) :
         itsREven(true), itsAlpha(alpha) 
{
  ASKAPCHECK(casa::abs(alpha-int(alpha))<1e-6, 
             "Current code supports only integer alpha due to GSL limitations, you have alpha="<<alpha);
  
  casa::Matrix<double> hlp(nterms,nterms,0.);
  
  fillHelperMatrix(hlp,c,alpha);
        
  fillLegendreCoeffs(hlp);

  itsSum0 = sumLegendreSeries(0., alpha);
}

/// @brief sum of the Legendre series
/// @details This helper method sums Legendre series for the given coefficients and the origin
/// @param[in] x abcissa 
/// @param[in] m parameter m of the Legendre function (corresponding to the resulting Smn(c,eta))
/// @note coefficients are taken from itsCoeffs, element index r is incremented by two, interpretation
/// of r depends on itsREven flag, which is true if series starts from r=0, false if from r=1 (n-m of Smn 
/// is even or odd). Note. that currently this class is only used to generate spheroidal functions
/// psi_{alpha 0}, i.e. corresponding to a single eigenvector associated with the smallest eigenvalue, so
/// n in Smn is always equal to m and itsREven is always true.
double SpheroidalFunction::sumLegendreSeries(const double x, const int m) const
{
   ASKAPASSERT(m>=0);
   ASKAPASSERT(itsCoeffs.nelements()>1);
   const int nOrders = int(itsCoeffs.nelements())*2 + (itsREven ? 0 : 1);
   double *vals = new double[nOrders+1];
           
   const int status = gsl_sf_legendre_sphPlm_array(nOrders + m, m, x, vals);
   double result = 0.;
   for (casa::uInt elem = 0; elem<itsCoeffs.nelements(); ++elem) {
        const int r = 2*elem + (itsREven ? 0 : 1);
        //const int l = r + m;
        ASKAPASSERT(r < nOrders + 1);
        result += itsCoeffs[elem]*vals[r];
   }
           
   delete[](vals);
   ASKAPCHECK(status == GSL_SUCCESS, "Error calculating associated Legendre functions, status="<<status);           
   return result;
}

/// @brief fill matrix which has the same eigenvalues/vectors as the original problem
/// @details See equation (20) in Aquino and Casta\~no (2002)
/// @param[out] B matrix to fill (should already be sized to the required number of terms)
/// @param[in] c bandwidth of the prolate spheroidal function
/// @param[in] m parameter m of the prolate spheroidal function Smn(c,eta)
/// @note This class is only used to generate spheroidal functions  psi_{alpha 0}, i.e. 
/// those corresponding to a single eigenvector associated with the smallest eigenvalue, so
/// n in Smn is always equal to m and itsREven is always true. In addition m = alpha. It is
/// passed as an additional parameter for generality.  
void SpheroidalFunction::fillHelperMatrix(casa::Matrix<double> &B, const double c, const int m) const
{
   ASKAPASSERT(B.nrow() == B.ncolumn());
   ASKAPASSERT(B.nrow() > 1);
   const double cSquared = c*c;
   for (casa::uInt row = 0; row<B.nrow(); ++row) {
        const int r = int(row)*2 + (itsREven ? 0 : 1);
        const int l = r + m; // order of Legendre function P_l^m
        B(row,row) = double(l*(l+1)) + cSquared*(double(2*l+3)*(l+m)*(l-m)+double(2*l-1)*(l+m+1)*(l-m+1)) /
                        (double(2*l+1)*(2*l-1)*(2*l+3));
        if (row>=1) {
            B(row,row-1) = cSquared/double(2*l-1)*sqrt(double(l+m)*(l+m-1)*(l-m)*(l-m-1)/(double(2*l+1)*(2*l-3)));
        }
        if (row+1<B.nrow()) {
            B(row,row+1) = cSquared/double(2*l+3)*sqrt(double(l+m+1)*(l+m+2)*(l-m+1)*(l-m+2)/
                             (double(2*l+1)*(2*l+5)));
        }
               
   }
}

/// @brief coefficients in Legendre series
/// @details This method solves eigenvalue problem and obtains eigenvector corresponding to
/// the smallest eigenvalue (for function Smn(c,eta) this means n=0). Coefficients are in the same order
/// as elements of matrix B, i.e. in steps of 2 starting from even or odd depending whether n-m is even or odd
/// @param[in] B matrix to solve
/// @return eigenvalue
/// @note an exception is thrown if there is an error solving eigensystem. itsCoeffs vector is resized
/// to match the input matrix and filled with coefficients for Legendre series 
double SpheroidalFunction::fillLegendreCoeffs(const casa::Matrix<double> &B)
{
    ASKAPASSERT(B.nrow() == B.ncolumn());
    itsCoeffs.resize(B.nrow());
         
    gsl_matrix *A = gsl_matrix_alloc(B.nrow(),B.nrow());
    gsl_matrix *eVec = gsl_matrix_alloc(B.nrow(),B.nrow());
    gsl_eigen_symmv_workspace *work = gsl_eigen_symmv_alloc(B.nrow());
    gsl_vector *eVal = gsl_vector_alloc(itsCoeffs.nelements());

    // fill the matrix (a bit of an overkill, but it is faster to reuse existing code
    // than to write something for tridiagonal matrix)
    for (casa::uInt row = 0; row<B.nrow(); ++row) {
         for (casa::uInt col = 0; col<B.ncolumn(); ++col) {
              gsl_matrix_set(A, row, col, B(row,col));
         }
    }
         
    const int status = gsl_eigen_symmv(A,eVal,eVec,work);
    double result = -1.;
    casa::uInt optIndex = 0;
    if (status == GSL_SUCCESS) {
        for (casa::uInt elem = 0; elem<itsCoeffs.nelements(); ++elem) {
             const double val = gsl_vector_get(eVal,elem);
             if ((elem == 0) || (val<result)) {
                  result = val;
                  optIndex = elem;
             }
        }
    }
         
    // extract the appropriate eigenvector
    for (size_t i=0; i<B.nrow(); ++i) {
         itsCoeffs[i] = gsl_matrix_get(eVec,i,optIndex);                             
    }         

    gsl_matrix_free(A);         
    gsl_matrix_free(eVec);         
    gsl_eigen_symmv_free(work);
    gsl_vector_free(eVal);
         
    ASKAPCHECK(status == GSL_SUCCESS, "Error solving eigenproblem in legendreCoeffs, status="<<status);
                 
    return result;         
}




