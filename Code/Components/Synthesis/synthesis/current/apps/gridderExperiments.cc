
// casa
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MEpoch.h>

// System includes
#include <stdexcept>
#include <iostream>
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <CommandLineParser.h>

#include <gridding/SphFuncVisGridder.h>
#include <dataaccess/DataAccessorStub.h>
#include <fitting/Axes.h>

ASKAP_LOGGER(logger, ".test");

using namespace askap;
using namespace askap::synthesis;
//using namespace askap::scimath;

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_eigen.h>

inline casa::DComplex getComplex(const gsl_complex gc) {
     return casa::DComplex(GSL_REAL(gc), GSL_IMAG(gc));
}


class testGridder : public SphFuncVisGridder
{
public:
      testGridder() {
          std::cout<<"Test gridder, used for debugging"<<std::endl;
          casa::Matrix<casa::DComplex> B(5,5);
          const double c = casa::C::pi*6/2.;
          fillMatrixB(B,c,15);
          std::cout<<B<<std::endl;
          
          casa::Vector<casa::DComplex> V(B.nrow());
          casa::DComplex eVal = optimumEigenVector(B,V);
          std::cout<<"eigen value "<<eVal<<" vector: "<<V<<std::endl;
          
          casa::Vector<casa::DComplex> P(V.nelements(),casa::DComplex(0.,0.));
          for (size_t i = 0; i<P.nelements(); ++i) {
               P[i] = -eVal*V[i];
               for (size_t k = 0; k<V.nelements(); ++k) {
                    P[i] += B(i,k)*V(k);
               }
          }
          
          std::cout<<P<<std::endl;
          
          casa::Vector<casa::DComplex> vals(6.);
          calcValsAtRegularGrid(vals, V, eVal, false);
                    
          std::cout<<vals<<std::endl;
          
          std::ofstream os ("cf.dat");
          for (size_t i=0; i<vals.nelements(); ++i) {
               const double x = double(i)*casa::C::pi/c;
               const double sfval = (abs(x)<1 ? real(vals[i])/sqrt(1.-x*x) : 0.);
               os<<x<<" "<<sfval<<" "<<grdsf(x)<<std::endl;
          }
          
          casa::Vector<double> coeffs(6,0.);
          calcBesselCoeffs(c,1,coeffs);
      }
      
      /// @brief lth derivative of kth Legendre polynomial at 1.0
      /// @details Calculate the value of the lth derivative of the Legendre
      /// polynomial at 1.0 using recursive formula. It might be possible to
      /// join several loops together and speed the algorithm up a bit, but
      /// we will worry about the optimisation later (if we see that it is 
      /// useful).
      /// @param[in] l order of the derivative
      /// @param[in] k order of the polynomial
      /// @return value of the derivative at 1.0
      static double derivativeOfLegendrePolynomial(casa::uInt l, casa::uInt k) {
         if (l > k) {
             return 0.;
         }
         // initialise with 0-order derivative
         double res = sqrt(double(2*k+1)/2.);
         for (size_t order = 0; order < l; ++order) {
              res *= (double(k*(k+1)) - double(order*(order+1)))/ double(2*(order+1));
         }
         return res;
      }
      
      /// @brief calculate values at the regular grid
      /// @details The spheroidal function is approximated as a series with coefficients
      /// which are the values at regular grid points pi*N/c. This method fills a vector
      /// with such values spanning N from 0 to size-1. The formulas are slightly different
      /// for odd and even order of spherodial functions.
      /// @param[in] vals values vector to fill, should already be sized to the required size
      /// @param[in] eVec eigen vector in Legendre space
      /// @param[in] eVal eigen value corresponding to eVec
      /// @param[in] isOdd if true, the calculated function is assumed to be of the odd order
      static void calcValsAtRegularGrid(casa::Vector<casa::DComplex> &vals, 
             const casa::Vector<casa::DComplex> &eVec, const casa::DComplex &eVal, const bool isOdd) 
      {
         ASKAPASSERT(vals.nelements()>1);
         ASKAPASSERT(eVec.nelements()>1);
         ASKAPASSERT(casa::abs(eVal) != 0.);
         // the value at 0 is calculated through direct series expansion
         vals.set(0.);
         // all odd Legendre polynomials are anti-symmetric, so will be the spheroidal function
         if (!isOdd) {
             double p0 = 1; // The value of Legendre polynomial at x=0
             for (size_t order=0; 2*order<eVec.nelements(); ++order) {
                  vals[0] += eVec[order] * p0;
                  p0 *= -double(order+1)/double(order+2);
             }
         }
         // now filling the values
         for (size_t N=1; N<vals.nelements(); ++N) {
              for (size_t k = (isOdd ? 1 : 0); k<eVec.nelements(); k+=2) {
                   // Ink is the coefficient in the eigenvector space
                   // see formula (49) in Karoui & Moumni
                   // for the function of an odd order, the value is pure imaginary
                   // so we store just the imaginary part 
                   double Ink = 0;
                   for (size_t l=1; l < k/2; ++l) {
                        if (isOdd) {
                            Ink += negateForOdd(l+1)/pow(casa::C::pi*double(N),2*l+1)*
                                   derivativeOfLegendrePolynomial(2*l,k);
                        } else {
                            Ink += negateForOdd(l+1)/pow(casa::C::pi*double(N),2*l)*
                                   derivativeOfLegendrePolynomial(2*l-1,k);
                        }
                   }
                   Ink *= 2*negateForOdd(N);
                   vals[N] += eVec[k] * Ink * (isOdd ? casa::DComplex(0.,1.) : casa::DComplex(1.,0.));
              }
              // all function values for N>0 should be divided by the eigenvalue
              vals[N] /= eVal;
         }             
      }
      
      /// @brief Bessel series expansion coefficients
      /// @details This is a helper method to compute series coefficients for decomposition
      /// of a given spheroidal functions via Bessel functions
      /// @param[in] c parameter c of the spheroidal function (bandwidth or a measure of the support size in our case)
      /// @param[in] alpha parameter alpha of the spheroidal function (weighting exponent in our case)
      /// @param[in] coeffs vector to fill with the coefficients (must already be resized to a 
      ///                   required number of coefficients)
      /// @param[in] mSize optional matrix size for the dependent eigenproblem, 0 means the miminal size sufficient to
      ///            produce coeffs.nelements() coefficient. Positive number should not be below coeffs.nelements().
      static void calcBesselCoeffs(const double c, const double alpha, const casa::Vector<double> &coeffs,
                                   const casa::uInt mSize = 0)
      {
        ASKAPASSERT(coeffs.nelements()>1);
        const casa::uInt matrSize = (mSize == 0 ? coeffs.nelements() : mSize);
        ASKAPCHECK(matrSize >= coeffs.nelements(), "Requested matrix size of "<<matrSize<<
                   " should not be less than the number of requested coefficients ("<<coeffs.nelements()<<")");
        ASKAPCHECK(2*alpha != -3.,"Implemented formulas don't work for alpha = -1.5");
        const double cSquared = c*c;
        // buffers
        casa::Vector<double> bufA(2*matrSize+1,0.);                   
        casa::Vector<double> bufB(2*matrSize+1,0.);
        casa::Vector<double> bufC(2*matrSize+1,0.);
        casa::Vector<double> diag(matrSize,0.);
        casa::Vector<double> sdiag2(matrSize-1,0.);
        
        // fill the buffers
        bufB[0] = cSquared / (2*alpha+3);
        bufC[0] = cSquared * (2*alpha+2) / (2*alpha+3);
        for (casa::uInt k = 2; k <= 2*matrSize; k+=2) {
             // k is a 1-based index into buffers
             bufA[k] = cSquared * double(k*(k-1))/(2*alpha+2*k-1)/(2*alpha+2*k+1);
             bufB[k] = cSquared * (double(k)*(2*alpha+k+1)+(2*alpha-1+2*k*k+2*k*(2*alpha+1)))/
                                  (2*alpha+2*k-1) / (2*alpha+2*k+3);
             bufC[k] = cSquared * (2*alpha+k+1) * (2*alpha+k+2) / (2*alpha+2*k+1) / (2*alpha+2*k+3);
             // recursion relation for matrix coefficients
             ASKAPDEBUGASSERT(k>=2);
             diag[k/2-1] = bufB[k-2];
             // sub-diagonal has one less element, exclude the last one
             if (k<2*matrSize) {
                 sdiag2[k/2-1] = bufA[k] * bufC[k-2];
             }
        }
        std::cout<<"diag="<<diag<<std::endl;
        std::cout<<"sdiag2="<<sdiag2<<std::endl;
        std::cout<<"ev="<<smallestEigenValue(diag,sdiag2)<<std::endl;
      }
      
      /// @brief smallest eigenvalue of a symmetric tridiagonal matrix
      /// @details This helper method finds the smallest eigenvalue of a symmetric tridiagonal
      /// matrix.
      /// @param[in] diag main diagonal of the matrix
      /// @param[in] sdiag2 squares of the subdiagonal of the matrix
      /// @return smallest eigenvalue
      static double smallestEigenValue(const casa::Vector<double> &diag, const casa::Vector<double> &sdiag2) 
      {
         ASKAPASSERT(diag.nelements() == sdiag2.nelements() + 1);
         ASKAPASSERT(diag.nelements() > 1);
         
         gsl_matrix *A = gsl_matrix_alloc(diag.nelements(),diag.nelements());
         gsl_matrix_set_zero(A);         
         gsl_eigen_symm_workspace *work = gsl_eigen_symm_alloc(diag.nelements());
         gsl_vector *eVal = gsl_vector_alloc(diag.nelements());
         
         // fill the matrix (a bit of an overkill, but it is faster to reuse existing code
         // than to write something for tridiagonal matrix)
         for (casa::uInt elem = 0; elem<diag.nelements(); ++elem) {
              gsl_matrix_set(A, elem, elem, diag[elem]);
              gsl_matrix_set(A, elem, elem, diag[elem]);
              if (elem + 1 < diag.nelements()) {
                  ASKAPASSERT(sdiag2[elem]>=0.);
                  gsl_matrix_set(A, elem, elem+1, sqrt(abs(sdiag2[elem])));              
                  gsl_matrix_set(A, elem+1, elem, sqrt(abs(sdiag2[elem])));              
              }
         }
         const int status = gsl_eigen_symm(A,eVal, work);
         double result = -1.;
         if (status == GSL_SUCCESS) {
             for (casa::uInt elem = 0; elem<diag.nelements(); ++elem) {
                  const double val = gsl_vector_get(eVal,elem);
                  if ((elem == 0) || (val<result)) {
                      result = val;
                  }
             }
         }

         gsl_matrix_free(A);         
         gsl_eigen_symm_free(work);
         gsl_vector_free(eVal);
         
         ASKAPCHECK(status == GSL_SUCCESS, "Error solving eigenproblem for symmetric tridiagonal matrix, status="<<status);
         return result;
      }
            
      /// @brief do eigen decomposition, get optimum eigen vector/value
      /// @details Solve for eigenvalues and eigen vectors of the helper matrix,
      /// Find the largest by absolute value and extract appropriate eigenvector
      /// @param[in] B matrix to decompose      
      /// @param[out] V optimum eigenvector (will be resized to B.nrow())
      /// @return largest eigenvalue (by absolute value)
      static casa::DComplex optimumEigenVector(const casa::Matrix<casa::DComplex> &B, casa::Vector<casa::DComplex> &V)
      {
         ASKAPASSERT(B.nrow() == B.ncolumn());
         ASKAPASSERT(B.nrow() > 1);
         gsl_matrix *A = gsl_matrix_alloc(B.nrow()*2,B.ncolumn()*2);
         gsl_vector_complex *eVal = gsl_vector_complex_alloc(B.nrow()*2);
         gsl_eigen_nonsymmv_workspace *work = gsl_eigen_nonsymmv_alloc(B.nrow()*2);
         gsl_matrix_complex *eVec = gsl_matrix_complex_alloc(B.nrow()*2,B.ncolumn()*2);
         
         for (size_t row=0; row<B.nrow(); ++row) {
              for (size_t col=0; col<B.ncolumn(); ++col) {
                   const double reB = casa::real(B(row,col));
                   const double imB = casa::imag(B(row,col));
                   gsl_matrix_set(A, row*2, col*2, reB);
                   gsl_matrix_set(A, row*2+1, col*2+1, reB);
                   gsl_matrix_set(A, row*2, col*2+1, -imB);
                   gsl_matrix_set(A, row*2+1, col*2, imB);                   
              }
         }
         const int status = gsl_eigen_nonsymmv(A,eVal,eVec, work);
         
         casa::Complex peakVal(0.,0.);
         if (status == 0) {
             // eigenproblem solved successfully
             size_t peakIndex = 0;
             
             // search for peak eigenvalue
             for (size_t el=0; el<B.nrow()*2; ++el) {
                  casa::DComplex val = getComplex(gsl_vector_complex_get(eVal,el));              
                  std::cout<<"el="<<el<<" "<<val<<std::endl;
                  if ((el == 0) || (casa::abs(val) > casa::abs(peakVal))) {
                      peakIndex = el;
                      peakVal = val;
                  }
             }
             
             /*
             peakIndex = 7;
             peakVal = getComplex(gsl_vector_complex_get(eVal,peakIndex));              
             */
             
             std::cout<<"peak Index="<<peakIndex<<" peakValue="<<peakVal<<std::endl;
             // extract the appropriate eigenvector
             V.resize(B.nrow());
             for (size_t i=0; i<B.nrow(); ++i) {
                  V[i] = getComplex(gsl_matrix_complex_get(eVec,  2*i,peakIndex))+casa::DComplex(0.,1.)*
                             getComplex(gsl_matrix_complex_get(eVec, 2*i+1,peakIndex));                             
             }
         }
         gsl_matrix_complex_free(eVec);
         gsl_eigen_nonsymmv_free(work);
         gsl_vector_complex_free(eVal);
         gsl_matrix_free(A);
         ASKAPCHECK(status == 0, "Eigen problem solution has failed in optimumEigenVector");
         return peakVal;
      }
      
      /// @brief helper method to evaluate (-1)^l
      /// @param[in] l integer power index
      /// @return 1 if l is even, -1 otherwise
      static inline int negateForOdd(const casa::uInt l) { return (l%2 == 0) ? 1 : -1; } 
      
      /// @brief fill matrix B which has the same eigenvalues as the original problem
      /// @details See equation (8) in Karoui & Moumni (2008)
      /// @param[in] B matrix to fill (should already be sized)
      /// @param[in] c bandwidth of the prolate spheroidal function
      /// @param[in] nterms number of terms in the series expansion
      static void fillMatrixB(casa::Matrix<casa::DComplex> &B, const double c, const int nterms = 15) {
          ASKAPASSERT(B.nrow() == B.ncolumn());
          ASKAPASSERT(B.nrow() > 1);
          ASKAPASSERT(nterms>1);
          // supplementary matrix is used to hold moments (row is the moment number, starting from 0),
          // of the normalised Legendre polynomials (column is the order of the polynomial, starting from 0)
          // Note, the matrix is rectangular. The last row is not used to fill
          // the matrix B, but is required to construct other elements of this supplementary matrix through
          // the recursion formula.
          casa::Matrix<double> moments(nterms+1,B.nrow(),0.);
          ASKAPASSERT(moments.ncolumn() >= 2);
          // fill the first two columns
          for (casa::uInt l=0; l<moments.nrow(); ++l) {
               moments(l,0) = (1. + negateForOdd(l))/(sqrt(2.)*(l + 1));
               moments(l,1) = sqrt(1.5)*(1. + negateForOdd(l+1))/(l + 2);               
          }
          // now fill other columns, if any
          for (casa::uInt k=1; k + 1 < moments.ncolumn(); ++k) {
               for (casa::uInt l=0; l + 1 < moments.nrow(); ++l) {
                    moments(l,k+1) = sqrt(double((2*k+1)*(2*k+3))/(k+1)/(k+1)) * moments(l+1,k) -
                          double(k)/(k+1)*sqrt(double(2*k+3)/(2*int(k)-1)) * moments(l, k-1);
               } 
          }
          
          // now fill the matrix B (approximation of the matrix for the Helmoltz equation operator in the 
          // Legendre basis)
          double coeff = 1.; // (c^l / l!)
          B.set(casa::DComplex(0.,0.));
          for (int l = 0; l<nterms; ++l) {
               if (l != 0) {
                   coeff *= c / l;
               }
               // i^l goes in sequence 1,i,-1,-i,1,...
               const casa::DComplex iPwrl = casa::DComplex((l % 4 > 1) ? -1. : 1.) * 
                             ((l % 2 == 1) ? casa::DComplex(0.,1.) : casa::DComplex(1.,0.));
               // fill the actual elements of the matrix              
               for (casa::uInt row = 0; row<B.nrow(); ++row) {
                    for (casa::uInt col = 0; col<B.ncolumn(); ++col) {
                         B(row,col) += iPwrl * coeff * moments(l,row) * moments(l, col); 
                    }
               }
          }
      }
};

// Main function
int main(int argc, const char** argv)
{

  try {
  
        // Put everything in scope to ensure that all destructors are called
        // before the final message
        {
            cmdlineparser::Parser parser; // a command line parser
            testGridder gridder;
            
            const double cellSize=10*casa::C::arcsec;

            casa::Matrix<double> xform(2,2,0.);
            xform.diagonal().set(1.);
               
            scimath::Axes axes;
            axes.addDirectionAxis(casa::DirectionCoordinate(casa::MDirection::J2000, 
                      casa::Projection(casa::Projection::SIN), 0.,0.,cellSize,cellSize,xform,256.,256.));
        
            DataAccessorStub acc(true);
            
            const casa::IPosition shape(4,256,256,1,1);
            gridder.initialiseGrid(axes,shape,false);
            gridder.grid(acc);
            casa::Array<double> grid;
            gridder.finaliseGrid(grid);        
        }
    } catch (const cmdlineparser::XParser &ex) {
        ASKAPLOG_FATAL_STR(logger, "Command line parser error, wrong arguments " << argv[0]);
        std::cerr << "Usage: " << argv[0] << " [-inputs parsetFile]"
                      << std::endl;
    } catch (const askap::AskapError& x) {
        ASKAPLOG_FATAL_STR(logger, "Askap error in " << argv[0] << ": " << x.what());
        std::cerr << "Askap error in " << argv[0] << ": " << x.what()
                      << std::endl;
        exit(1);
    } catch (const std::exception& x) {
        ASKAPLOG_FATAL_STR(logger, "Unexpected exception in " << argv[0] << ": " << x.what());
        std::cerr << "Unexpected exception in " << argv[0] << ": " << x.what()
                      << std::endl;
        exit(1);
    }

    return 0;
}

