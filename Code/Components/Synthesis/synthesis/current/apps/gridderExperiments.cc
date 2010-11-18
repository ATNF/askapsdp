
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

class testGridder : public SphFuncVisGridder
{
public:
      testGridder() {
          std::cout<<"Test gridder, used for debugging"<<std::endl;
          casa::Matrix<casa::DComplex> B(5,5);
          fillMatrixB(B,1.5,15);
          std::cout<<B<<std::endl;
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

