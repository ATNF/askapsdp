/// @file FFTWrapper.cc
///
/// FFTWrapper: This class provides limited wrapper (adapter) functionality
/// for the FFTW library
///
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
/// @author Rudolph van der Merwe <rudolph@ska.ac.za>
/// @author Ben Humphreys <ben.humphreys@csiro.au>

// Include own header file first
#include "fft/FFTWrapper.h"

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "profile/AskapProfiler.h"
#include "casa/Arrays/Vector.h"
#include "casa/Arrays/Matrix.h"
#include "casa/Arrays/ArrayIter.h"
#include "fftw3.h"

#ifdef _OPENMP
// boost include
#include "boost/thread/mutex.hpp"
#endif

using namespace casa;

namespace askap {
    namespace scimath {

#ifdef _OPENMP
        /// @brief mutex to ensure thread safety in the calls to fft
        static boost::mutex fftWrapperMutex;
#endif

        /**
         * Scale the array by 1/N were N is the total number of elements in
         * the array
         */
        template<typename T>
        static inline void scaleResult(T* data, const size_t nElements)
        {
            const T scale = T(1) / T(nElements);
            for (size_t i = 0; i < nElements; i++) {
                data[i] *= scale;
            }
        }

        /*
         * Execute a Double-precision FFT plan.
         */
        static inline void fftExec(casa::Vector<casa::DComplex>& vec, const bool forward,
                            fftw_plan& p, fftw_complex* buf, size_t bufsz)
        {
            const size_t nElements = vec.nelements();
            ASKAPDEBUGASSERT(nElements == bufsz);

            Bool deleteIt;
            DComplex *dataPtr = vec.getStorage(deleteIt);

            // rotate input because the origin for FFTW is at 0, not n/2 (casa fft).
            std::rotate_copy(dataPtr, dataPtr + (nElements / 2), dataPtr + nElements,
                             reinterpret_cast<casa::DComplex*>(buf));

            fftw_execute(p);

            // rotate output and copy back to the vectors storage
            std::rotate_copy(reinterpret_cast<casa::DComplex*>(buf),
                             reinterpret_cast<casa::DComplex*>(buf) + (nElements / 2),
                             reinterpret_cast<casa::DComplex*>(buf) + nElements,
                             dataPtr);

            if (!forward) {
                scaleResult(dataPtr, nElements);
            }

            vec.putStorage(dataPtr, deleteIt);
        }

        /**
         * Execute a Single-precision FFT plan.
         */
        static inline void fftExec(casa::Vector<casa::Complex>& vec, const bool forward,
                            fftwf_plan& p, fftwf_complex* buf, size_t bufsz)
        {
            const size_t nElements = vec.nelements();
            ASKAPDEBUGASSERT(nElements == bufsz);

            Bool deleteIt;
            Complex *dataPtr = vec.getStorage(deleteIt);

            // rotate input because the origin for FFTW is at 0, not n/2 (casa fft)
            std::rotate_copy(dataPtr, dataPtr + (nElements / 2), dataPtr + nElements,
                    reinterpret_cast<casa::Complex*>(buf));

            fftwf_execute(p);

            // rotate output
            std::rotate_copy(reinterpret_cast<casa::Complex*>(buf),
                             reinterpret_cast<casa::Complex*>(buf) + (nElements / 2),
                             reinterpret_cast<casa::Complex*>(buf) + nElements,
                             dataPtr);

            if (!forward) {
                scaleResult(dataPtr, nElements);
            }

            vec.putStorage(dataPtr, deleteIt);
        }

        void fft(casa::Vector<casa::DComplex>& vec, const bool forward)
        {
            ASKAPTRACE("fft<casa::DComplex>");
#ifdef _OPENMP
            boost::unique_lock<boost::mutex> lock(fftWrapperMutex);
#endif

            Bool deleteIt;
            DComplex *dataPtr = vec.getStorage(deleteIt);
            const size_t nElements = vec.nelements();

            // rotate input because the origin for FFTW is at 0, not n/2 (casa fft)
            std::rotate(dataPtr, dataPtr + (nElements / 2), dataPtr + nElements);

            fftw_plan p = fftw_plan_dft_1d(nElements, reinterpret_cast<fftw_complex*>(dataPtr),
                                           reinterpret_cast<fftw_complex*>(dataPtr),
                                           (forward) ? FFTW_FORWARD : FFTW_BACKWARD, FFTW_ESTIMATE);

            fftw_execute(p);

            if (!forward) {
                scaleResult(dataPtr, nElements);
            }

            // rotate output
            std::rotate(dataPtr, dataPtr + (nElements / 2), dataPtr + nElements);

            vec.putStorage(dataPtr, deleteIt);
            fftw_destroy_plan(p);
        }

        void fft(casa::Vector<casa::Complex>& vec, const bool forward)
        {
            ASKAPTRACE("fft<casa::Complex>");
#ifdef _OPENMP
            boost::unique_lock<boost::mutex> lock(fftWrapperMutex);
#endif

            Bool deleteIt;
            Complex *dataPtr = vec.getStorage(deleteIt);
            const size_t nElements = vec.nelements();

            // rotate input because the origin for FFTW is at 0, not n/2 (casa fft)
            std::rotate(dataPtr, dataPtr + (nElements / 2), dataPtr + nElements);

            fftwf_plan p = fftwf_plan_dft_1d(nElements, reinterpret_cast<fftwf_complex*>(dataPtr),
                                             reinterpret_cast<fftwf_complex*>(dataPtr),
                                             (forward) ? FFTW_FORWARD : FFTW_BACKWARD, FFTW_ESTIMATE);
            fftwf_execute(p);

            if (!forward) {
                scaleResult(dataPtr, nElements);
            }

            // rotate output
            std::rotate(dataPtr, dataPtr + (nElements / 2), dataPtr + nElements);

            vec.putStorage(dataPtr, deleteIt);
            fftwf_destroy_plan(p);
        }

        void fft2d(casa::Array<casa::Complex>& arr, const bool forward)
        {
            ASKAPTRACE("fft2d<casa::Complex>");
#ifdef _OPENMP
            boost::unique_lock<boost::mutex> lock(fftWrapperMutex);
#endif

            // 1: Make an iterator that returns plane by plane
            casa::ArrayIterator<casa::Complex> it(arr, 2);

            while (!it.pastEnd()) {
                casa::Matrix<casa::Complex> mat(it.array());
                ASKAPDEBUGASSERT(mat.ncolumn() > 0 && mat.nrow() > 0);

                // 2: Setup a buffer and fft plan based on the size of the first column
                size_t bufsz = mat.nrow();
                fftwf_complex* buf = (fftwf_complex*) fftw_malloc(sizeof(fftwf_complex) * bufsz);
                fftwf_plan p = fftwf_plan_dft_1d(bufsz, buf, buf,
                                                 (forward) ? FFTW_FORWARD : FFTW_BACKWARD, FFTW_ESTIMATE);

                // 3: FFT each column
                for (uInt col = 0; col < mat.ncolumn(); col++) {
                    casa::Vector<casa::Complex> vec(mat.column(col));
                    fftExec(vec, forward, p, buf, bufsz);
                }

                // 4: If the row are of different length to the rows then
                // re-allocate buffer and regen the plan
                if (mat.ncolumn() != mat.nrow()) {
                    fftwf_destroy_plan(p);
                    fftwf_free(buf);
                    bufsz = mat.ncolumn();
                    fftwf_complex* buf = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * bufsz);
                    p = fftwf_plan_dft_1d(bufsz, buf, buf,
                                          (forward) ? FFTW_FORWARD : FFTW_BACKWARD, FFTW_MEASURE);
                }

                // 5: FFT each row
                for (uInt row = 0; row < mat.nrow(); row++) {
                    casa::Vector<casa::Complex> vec(mat.row(row));
                    fftExec(vec, forward, p, buf, bufsz);
                }

                // 6: Delete the plan and temporary buffer
                fftwf_destroy_plan(p);
                fftwf_free(buf);

                it.next();
            }
        }

        void fft2d(casa::Array<casa::DComplex>& arr, const bool forward)
        {
            ASKAPTRACE("fft2d<casa::DComplex>");
#ifdef _OPENMP
            boost::unique_lock<boost::mutex> lock(fftWrapperMutex);
#endif

            /// 1: Make an iterator that returns plane by plane
            casa::ArrayIterator<casa::DComplex> it(arr, 2);

            while (!it.pastEnd()) {
                casa::Matrix<casa::DComplex> mat(it.array());
                ASKAPDEBUGASSERT(mat.ncolumn() > 0 && mat.nrow() > 0);

                // 2: Setup a buffer and fft plan based on the size of the first column
                size_t bufsz = mat.nrow();
                fftw_complex* buf = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * bufsz);
                fftw_plan p = fftw_plan_dft_1d(bufsz, buf, buf,
                                               (forward) ? FFTW_FORWARD : FFTW_BACKWARD, FFTW_ESTIMATE);

                // 3: FFT each column
                for (uInt col = 0; col < mat.ncolumn(); col++) {
                    casa::Vector<casa::DComplex> vec(mat.column(col));
                    fftExec(vec, forward, p, buf, bufsz);
                }

                // 4: If the rows are of different length to the columns then
                // re-allocate buffer and regen the plan
                if (mat.ncolumn() != mat.nrow()) {
                    fftw_destroy_plan(p);
                    fftw_free(buf);
                    bufsz = mat.ncolumn();
                    fftw_complex* buf = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * bufsz);
                    p = fftw_plan_dft_1d(bufsz, buf, buf,
                                         (forward) ? FFTW_FORWARD : FFTW_BACKWARD, FFTW_MEASURE);
                }

                // 5: FFT each row
                for (uInt row = 0; row < mat.nrow(); row++) {
                    casa::Vector<casa::DComplex> vec(mat.row(row));
                    fftExec(vec, forward, p, buf, bufsz);
                }

                // 6: Delete the plan and temporary buffer
                fftw_destroy_plan(p);
                fftw_free(buf);

                it.next();
            }
        }
    }
}
