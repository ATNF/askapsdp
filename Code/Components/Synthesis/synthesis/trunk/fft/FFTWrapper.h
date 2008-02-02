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
      /// @brief 1-D inplace transform
      /// @param vec Complex vector
      /// @param forward Forward transform?
        void fft(casa::Vector<casa::DComplex>& vec, const bool forward);
        /// @brief 1-D inplace transform
        /// @param vec Complex vector
        /// @param forward Forward transform?
        void fft(casa::Vector<casa::Complex>& vec, const bool forward);

        /// @brief FFT first two axes only
        /// @param arr Complex array
        /// @param forward Forward transform?
	void fft2d(casa::Array<casa::Complex>& arr, const bool forward);

	/// @brief FFT first two axes only
        /// @param arr Complex array
        /// @param forward Forward transform?
	void fft2d(casa::Array<casa::DComplex>& arr, const bool forward);

    }
}
#endif
