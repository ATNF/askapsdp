/// @file FFTWrapper.h
///
/// FFTWrapper: This class provides limited wrapper (adapter) functionality for the FFTW library
///
/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Rudolph van der Merwe <rudolph@ska.ac.za>
///

#ifndef FFTWRAPPER_H_
#define FFTWRAPPER_H_

#include <casa/Arrays/Vector.h>

namespace conrad
{
    namespace synthesis
    {

        void fft(casa::Vector<casa::DComplex>& vec, const bool forward);

        void fft(casa::Vector<casa::Complex>& vec, const bool forward);

        /// FFT first two axes only
  			void fft2d(casa::Array<casa::Complex>& arr, const bool forward);
  			void fft2d(casa::Array<casa::DComplex>& arr, const bool forward);

    }
}
#endif
