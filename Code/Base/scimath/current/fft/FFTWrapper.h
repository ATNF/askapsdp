/// @file FFTWrapper.h
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
///

#ifndef ASKAP_SCIMATH_FFTWRAPPER_H
#define ASKAP_SCIMATH_FFTWRAPPER_H

// ASKAPsoft includes
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Array.h>

namespace askap
{
    namespace scimath
    {
        /// @brief 1-D inplace transform
        /// @param vec Complex vector
        /// @param forward Forward transform?
        /// @ingroup fft
        void fft(casa::Vector<casa::DComplex>& vec, const bool forward);

        /// @brief 1-D inplace transform
        /// @param vec Complex vector
        /// @param forward Forward transform?
        /// @ingroup fft
        void fft(casa::Vector<casa::Complex>& vec, const bool forward);

        /// @brief FFT first two axes only
        /// @param arr Complex array
        /// @param forward Forward transform?
        /// @ingroup fft
        void fft2d(casa::Array<casa::Complex>& arr, const bool forward);

        /// @brief FFT first two axes only
        /// @param arr Complex array
        /// @param forward Forward transform?
        /// @ingroup fft
        void fft2d(casa::Array<casa::DComplex>& arr, const bool forward);
    }
}
#endif
