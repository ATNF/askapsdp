/// @file
///
/// FFTWrapper: This class provides limited wrapper (adapter) functionality for the FFTW library
///
/// (c) 2007 ASKAP, All Rights Reserved.
/// @author Rudolph van der Merwe <rudolph@ska.ac.za>
///
#include <fft/FFTWrapper.h>

#include <askap/AskapTypes.h>
#include <askap/AskapError.h>
#include <complex>
#ifdef ASKAP_USE_FFTW
#include <fftw3.h>
#else 
#include <scimath/Mathematics/FFTServer.h>
#endif

#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/ArrayIter.h>

using namespace casa;

namespace askap
  {
    namespace synthesis
      {

        void fft(casa::Vector<casa::DComplex>& vec, const bool forward)
          {
#ifdef ASKAP_USE_FFTW
            const IPosition shape = vec.shape();
            const uInt nElements = shape[0];
            Bool deleteIt;
            DComplex *dataPtr = vec.getStorage(deleteIt);
            DComplex *tempPtr = dataPtr;
            fftw_plan p;
            p = fftw_plan_dft_1d(nElements, reinterpret_cast<fftw_complex*>(dataPtr), reinterpret_cast<fftw_complex*>(dataPtr),
                (forward) ? FFTW_FORWARD : FFTW_BACKWARD, FFTW_ESTIMATE);
            fftw_execute(p);
            if (!forward)
              {
                const DComplex scale = DComplex(1)/DComplex(nElements);
                for(int i=0; i<nElements; i++)
                  {
                    *tempPtr++ *= scale;
                  }
              }
            vec.putStorage(dataPtr, deleteIt);
            fftw_destroy_plan(p);
#else
            casa::FFTServer<double, casa::DComplex> ffts;
            bool ncForward(forward);
            ffts.fft(vec, ncForward);
#endif
          }

        void fft(casa::Vector<casa::Complex>& vec, const bool forward)
          {
#ifdef ASKAP_USE_FFTW
            const IPosition shape = vec.shape();
            const uInt nElements = shape[0];
            Bool deleteIt;
            Complex *dataPtr = vec.getStorage(deleteIt);
            Complex *tempPtr = dataPtr;
            fftwf_plan p;
            p = fftwf_plan_dft_1d(nElements, reinterpret_cast<fftwf_complex*>(dataPtr), reinterpret_cast<fftwf_complex*>(dataPtr),
                (forward) ? FFTW_FORWARD : FFTW_BACKWARD, FFTW_ESTIMATE);
            fftwf_execute(p);
            if (!forward)
              {
                const Complex scale = Complex(1)/Complex(nElements);
                for(int i=0; i<nElements; i++)
                  {
                    *tempPtr++ *= scale;
                  }
              }
            vec.putStorage(dataPtr, deleteIt);
            fftwf_destroy_plan(p);
#else
            casa::FFTServer<float, casa::Complex> ffts;
            bool ncForward(forward);
            ffts.fft(vec, ncForward);
#endif
          }

        void fft2d(casa::Array<casa::Complex>& arr, const bool forward)
          {
            /// Make an iterator that returns plane by plane
            casa::ArrayIterator<casa::Complex> it(arr, 2);
            while (!it.pastEnd())
              {
                casa::Matrix<casa::Complex> mat(it.array());
                ASKAPDEBUGASSERT(arr.shape()(1)>=0);
                for (uint iy=0; iy<uint(arr.shape()(1)); iy++)
                  {
                    casa::Vector<casa::Complex> vec(mat.column(iy));
                    fft(vec, forward);
                  }
                ASKAPDEBUGASSERT(arr.shape()(0)>=0);  
                for (uint ix=0; ix<uint(arr.shape()(0)); ix++)
                  {
                    casa::Vector<casa::Complex> vec(mat.row(ix));
                    fft(vec, forward);
                  }
                it.next();
              }
          }
        void fft2d(casa::Array<casa::DComplex>& arr, const bool forward)
          {
            /// Make an iterator that returns plane by plane
            casa::ArrayIterator<casa::DComplex> it(arr, 2);
            while (!it.pastEnd())
              {
                casa::Matrix<casa::DComplex> mat(it.array());
                ASKAPDEBUGASSERT(arr.shape()(1));
                for (uint iy=0; iy<uint(arr.shape()(1)); iy++)
                  {
                    casa::Vector<casa::DComplex> vec(mat.column(iy));
                    fft(vec, forward);
                  }
                ASKAPDEBUGASSERT(arr.shape()(0));
                for (uint ix=0; ix<uint(arr.shape()(0)); ix++)
                  {
                    casa::Vector<casa::DComplex> vec(mat.row(ix));
                    fft(vec, forward);
                  }
                it.next();
              }
          }
      }
  }

