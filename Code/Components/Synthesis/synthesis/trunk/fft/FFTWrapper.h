/// @file FFTWrapper.h
///
/// FFTWrapper: This class provides limited wrapper (adapter) functionality for the FFTW library
///
/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Rudolph van der Merwe <rudolph@ska.ac.za>
///

#ifndef FFTWRAPPER_H_
#define FFTWRAPPER_H_

#include <complex>
#include <fftw3.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>

using namespace casa;

namespace conrad
{
    namespace fftw
    {

        template <typename T>
        void fft(Vector<T>& vec, const Bool forward=True);
        
        template <>
        void fft(Vector<DComplex>& vec, const Bool forward)
        {
            const IPosition shape = vec.shape();
            const uInt nElements = shape[0];
            Bool deleteIt;
            DComplex *dataPtr = vec.getStorage(deleteIt);
            DComplex *tempPtr = dataPtr;
            fftw_plan p;
            p = fftw_plan_dft_1d(nElements, reinterpret_cast<fftw_complex*>(dataPtr), reinterpret_cast<fftw_complex*>(dataPtr), 
                                 (forward) ? FFTW_FORWARD : FFTW_BACKWARD, FFTW_ESTIMATE);                           
            fftw_execute(p); 
            if (!forward) {
                const DComplex scale = DComplex(1)/DComplex(nElements);
                for(int i=0; i<nElements; i++){
                    *tempPtr++ *= scale;
                }
            }
            vec.putStorage(dataPtr, deleteIt);    
            fftw_destroy_plan(p);
        }
        
        template <>
        void fft(Vector<Complex>& vec, const Bool forward)
        {
            const IPosition shape = vec.shape();
            const uInt nElements = shape[0];
            Bool deleteIt;
            Complex *dataPtr = vec.getStorage(deleteIt);
            Complex *tempPtr = dataPtr;
            fftwf_plan p;
            p = fftwf_plan_dft_1d(nElements, reinterpret_cast<fftwf_complex*>(dataPtr), reinterpret_cast<fftwf_complex*>(dataPtr), 
                                 (forward) ? FFTW_FORWARD : FFTW_BACKWARD, FFTW_ESTIMATE);                           
            fftwf_execute(p); 
            if (!forward) {
                const Complex scale = Complex(1)/Complex(nElements);
                for(int i=0; i<nElements; i++){
                    *tempPtr++ *= scale;
                }
            }
            vec.putStorage(dataPtr, deleteIt);    
            fftwf_destroy_plan(p);
        }        
        
    }
}
#endif
