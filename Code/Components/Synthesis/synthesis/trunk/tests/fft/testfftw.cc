#include <iostream>
#include <ctime> 
#include <cstdlib>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <fft/FFTWrapper.h>

using namespace casa;

#define FFT true
#define IFFT false

const int M = 10;

const double sp_precision = 1e-6;
const double dp_precision = 1e-12;

using namespace std;

//---------------------------------------------------------------------------------------------

double myRand(double low, double high)
{
    double range = (high - low);
    return low + range * ((double)rand())/((double)RAND_MAX);     
}

//---------------------------------------------------------------------------------------------

template <typename T>
void print_array(const casa::Array<T>& a)
{
    casa::IPosition aShape = a.shape();
    for(int i=0; i < aShape.product(); i++){
        casa::IPosition iPos = casa::toIPositionInArray(i, aShape);
        cout << iPos << " : " << a(iPos) << endl;
    }
    cout << endl;    
}

//---------------------------------------------------------------------------------------------

enum MetricNames {NRMSE, NMSE, RMSE, MSE};

template <typename T>
double calc_error(const casa::Array<T>& A, const casa::Array<T>& B, MetricNames metric)
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
        }
                
    }
    
    totalError /= (double)aShape.product();

    switch (metric){
        case RMSE:
        case NRMSE:
            totalError = sqrt(totalError);
            break;
    }

    return totalError;
}

//---------------------------------------------------------------------------------------------
template <typename T>
Bool test_for_equality(const casa::Array<T>& A, const casa::Array<T>& B, MetricNames metric, 
                       const double diffP, double& error)
{
    casa::IPosition aShape = A.shape();
    casa::IPosition bShape = B.shape();

    if (aShape != bShape) return False; 

    error = calc_error(A, B, metric);

    return ((error > diffP) ? False : True);
   
}

//---------------------------------------------------------------------------------------------

template <typename T>
int forward_backward_test(const int N, casa::Matrix<T>& mat, const double diffP)
{
    
    double diff = 0.0;
    
    for(int c=0; c < N; c++){
        for(int r = 0; r < N; r++){
            mat(r,c) = T(myRand(-0.5,0.5), myRand(-0.5,0.5));
        } 
    }
    casa::Matrix<T> original = mat.copy();    
    
    //conrad::printContainer(cout, original); cout << endl << endl;
        
    // Forward FFT
    for(int c=0; c<N; c++){
        casa::Vector<T> y = mat.column(c);
        conrad::synthesis::fft(y, FFT);                    
    }
    for(int r=0; r<N; r++){
        casa::Vector<T> y = mat.row(r);
        conrad::synthesis::fft(y, FFT);                            
    }
    
    if (test_for_equality(mat, original, NRMSE, diffP, diff)) {
        cout << "Problems! fft(X) == X . Diff percentage = " << diff << endl;
        return 1;
    }; 
    
    diff = 0.0;
    
    // Inverse FFT
    for(int c=0; c<N; c++){
        casa::Vector<T> y = mat.column(c);
        conrad::synthesis::fft(y, IFFT);                    
    }
    for(int r=0; r<N; r++){
        casa::Vector<T> y = mat.row(r);
        conrad::synthesis::fft(y, IFFT);                            
    }

    //conrad::printContainer(cout, mat); cout << endl << endl;
                
    if (test_for_equality(mat, original, NRMSE, diffP, diff)) {
        cout << "Success! : ifft(fft(X)) == X ";
        return 0;
    } else {
        cout << "Error!   : ifft(fft(X)) != X . Diff percentage = " << diff << endl;
        return 1;
    }
}

//===================================================================================================

int main ()
{
    
    srand((unsigned)time(0)); 
    
    int problems;
    std::vector<int> dataLength(M);
    for(int i=0; i < dataLength.size(); i++){
        dataLength[i]=pow(2,i);
    }
    int N;
    
    cout << endl << "FFTW Test Program";
    cout << endl << "=================" << endl << endl;
    
    for(int i = 0; i < dataLength.size(); ++i)
    {
        N = dataLength[i];
        
        cout << "------------------------------------------------------" << endl;
        cout << "Data size = (NxN) : N = " << N << endl << endl;

        // First test for single precision
        
        cout << "Testing for single precisions : Error threshold = " << sp_precision << " : ";
        casa::Matrix<casa::Complex> sp_mat(N, N, 1);
        problems = forward_backward_test(N, sp_mat, sp_precision);
        cout << endl;
    
        // Now test for double precision
    
        cout << "Testing for double precisions : Error threshold = " << dp_precision << " : ";
        casa::Matrix<casa::DComplex> dp_mat(N, N, 1);
        problems += forward_backward_test(N, dp_mat, dp_precision);
        cout << endl;

    }
    
    return problems;
}
