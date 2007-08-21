#include <iostream>
#include <fftw3.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <FFTWrapper.h>

#define FFT True
#define IFFT False

#define PRECISION 1e-10
typedef casa::DComplex MyComplex;

using namespace std;

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

template <typename T>
Bool test_for_equality(const casa::Array<T>& A, const casa::Array<T>& B, const T diffP = PRECISION)
{
    T diff;
    casa::IPosition aShape = A.shape();
    casa::IPosition bShape = B.shape();
    if (aShape != bShape) return False;    
    for(int i=0; i < aShape.product(); i++){
        casa::IPosition iPos = casa::toIPositionInArray(i, aShape);
        diff = abs((A(iPos)-B(iPos))/(T(0.5)*(A(iPos)+B(iPos))));
        if (diff > diffP) {
            //cout << endl << "Max error percentage = " << diff << endl <<endl;
            return False;
        }
    }
    return True;
}

int main ()
{
    
    cout << endl << "FFTW Test Program" << endl << endl;
    
    int N = 8;
    
    casa::Matrix<MyComplex> mat(N, N, 1);

    for(int c=0; c < N; c++){
        for(int r = 0; r < N; r++){
            mat(r,c) = MyComplex(rand(), rand());
        } 
    }
    casa::Matrix<MyComplex> original = mat.copy();    
    
    // cout << "mat == mat ? " << test_for_equality(mat, mat) << endl << endl;
    // cout << "mat == original ? " << test_for_equality(mat, original) << endl << endl;
        
    // cout << endl << "Original input matrix" << endl << "----------" << endl;        
    // print_array(mat);
    

    // Forward FFT
    for(int c=0; c<N; c++){
        casa::Vector<MyComplex> y = mat.column(c);
        conrad::fftw::fft(y, FFT);                    
    }
    for(int r=0; r<N; r++){
        casa::Vector<MyComplex> y = mat.row(r);
        conrad::fftw::fft(y, FFT);                            
    }
    
    if (test_for_equality(mat, original)) {
        cout << "Problems! fft(X) == X" << endl;
    }; 
    
    // Inverse FFT
    for(int c=0; c<N; c++){
        casa::Vector<MyComplex> y = mat.column(c);
        conrad::fftw::fft(y, IFFT);                    
    }
    for(int r=0; r<N; r++){
        casa::Vector<MyComplex> y = mat.row(r);
        conrad::fftw::fft(y, IFFT);                            
    }
                
    if (test_for_equality(mat, original)) {
        cout << "Success! : ifft(fft(X)) == X   ";
    } else {
        cout << "Error!   : ifft(fft(X)) != X   ";
    }
    cout << "(precision = " << PRECISION << ")" << endl;
    cout << endl << endl;    
    
    //delete[] x;
    
    return 0;
}
