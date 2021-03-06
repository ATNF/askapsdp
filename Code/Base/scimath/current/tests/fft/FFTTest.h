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

#include <cstdlib>
#include <fftw3.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <fft/FFTWrapper.h>

#include <askap/AskapError.h>
#include <cppunit/extensions/HelperMacros.h>

#include <stdexcept>

#define FFT true
#define IFFT false

enum MetricNames {NRMSE, NMSE, RMSE, MSE};

using namespace casa;

//------------------------------------------------------------------------
static double myRand(double low, double high)
{
    double range = (high - low);
    return low + range * ((double)rand())/((double)RAND_MAX);     
}

//---------------------------------------------------------------------------------------------
template <typename T>
static double calc_error(const casa::Array<T>& A, const casa::Array<T>& B, MetricNames metric)
{
    casa::IPosition aShape = A.shape();
    casa::IPosition bShape = B.shape();
    double totalError = 0.0;
    for(int i=0; i < aShape.product(); i++){
        casa::IPosition iPos = casa::toIPositionInArray(i, aShape);
        switch (metric) {
            case MSE:
            case RMSE:
                totalError += pow(abs(A(iPos)-B(iPos)),2);
                break;
            case NMSE:
            case NRMSE:
                totalError += pow(abs(A(iPos)-B(iPos)) / abs(B(iPos)),2);
                break;
            default:
                ASKAPTHROW(askap::AskapError, "Action is not defined for metric "<<metric);
        }
    }

    totalError /= (double)aShape.product();
    switch (metric){
        case RMSE:
        case NRMSE:
            totalError = sqrt(totalError);
            break;
        default:
            ASKAPTHROW(askap::AskapError, "Action is not defined for metric "<<metric);
    }
    return totalError;
}

//---------------------------------------------------------------------------------------------
template <typename T>
static bool test_for_equality(const casa::Array<T>& A, const casa::Array<T>& B, MetricNames metric, 
                       const double diffP, double& error)
{
    casa::IPosition aShape = A.shape();
    casa::IPosition bShape = B.shape();
    if (aShape != bShape) return false; 
    error = calc_error(A, B, metric);
    return ((error > diffP) ? false : true);
}

//---------------------------------------------------------------------------------------------
template <typename T>
static bool forward_backward_test(const int N, casa::Matrix<T>& mat, MetricNames metric, const double diffP)
{    
    bool returnVal;
    double diff = 0.0;
       
    casa::IPosition shape = mat.shape();
       
    for(int c=0; c < shape[0]; c++){
        for(int r = 0; r < shape[1]; r++){
            mat(r,c) = T(myRand(-0.5,0.5), myRand(-0.5,0.5));
        } 
    }
    casa::Matrix<T> original = mat.copy();    
                
    // Forward FFT
    for(int c=0; c<N; c++){
        casa::Vector<T> y = mat.column(c);
        askap::scimath::fft(y, FFT);                    
    }
    for(int r=0; r<N; r++){
        casa::Vector<T> y = mat.row(r);
        askap::scimath::fft(y, FFT);                            
    }
    
    returnVal = !(test_for_equality(mat, original, metric, diffP, diff));
    
    diff = 0.0;
    // Inverse FFT
    for(int c=0; c<N; c++){
        casa::Vector<T> y = mat.column(c);
        askap::scimath::fft(y, IFFT);                    
    }
    for(int r=0; r<N; r++){
        casa::Vector<T> y = mat.row(r);
        askap::scimath::fft(y, IFFT);                            
    }

    returnVal = returnVal && test_for_equality(mat, original, metric, diffP, diff);

    return returnVal;
}

//===============================================================================================

namespace askap
{
  namespace scimath
  {

    class FFTTest : public CppUnit::TestFixture
    {

      CPPUNIT_TEST_SUITE(FFTTest);
      CPPUNIT_TEST(testForwardBackwardSinglePrecision);
      CPPUNIT_TEST(testForwardBackwardDoublePrecision);      
      CPPUNIT_TEST_SUITE_END();

      private:
        
        int M;
        std::vector<int> dataLength;
        double sp_precision;
        double dp_precision;
     
      public:
          
        void setUp()
        {
            M = 11;
            sp_precision = 5e-6;
            dp_precision = 5e-12;            
            srand(0);
            dataLength = std::vector<int>(M); 
            // The maximum data length N*N is set here. We use powers of two from 2 to 2048 for N.
            for(size_t i=0; i < dataLength.size(); i++){
                dataLength[i]=(int)pow(2,i+1);
            }            
        }

        void testForwardBackwardSinglePrecision()
        {
            for(size_t i=0; i < dataLength.size(); i++){
                const size_t N = dataLength[i];
                cout << endl << "  Single precision : NMRSE error threshold = " << sp_precision 
                     << " : Testing if  ifft(fft(X)) = X : 2D NxN fft : N = " << N;
                casa::Matrix<casa::Complex> sp_mat(N, N, 1);
                CPPUNIT_ASSERT(forward_backward_test(N, sp_mat, NRMSE, sp_precision) == true);
            }
        }
        void testForwardBackwardDoublePrecision()
        {
            for(size_t i=0; i<dataLength.size(); i++){
                const size_t N = dataLength[i];            
                cout << endl << "  Double precision : NMRSE error threshold = " << dp_precision
                     << " : Testing if  ifft(fft(X)) = X : 2D NxN fft : N = " << N;
                casa::Matrix<casa::DComplex> dp_mat(N, N, 1);
                CPPUNIT_ASSERT(forward_backward_test(N, dp_mat, NRMSE, dp_precision) == true);
            }
        }
        
    };
    
  }
 }

        
