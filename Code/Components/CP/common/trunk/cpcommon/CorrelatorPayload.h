/// @file CorrelatorPayload.h
///
/// @copyright (c) 2009 CSIRO
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
/// @author Ben Humphreys <ben.humphreys@csiro.au>

#ifndef ASKAP_CP_CORRELATORPAYLOAD_H
#define ASKAP_CP_CORRELATORPAYLOAD_H

// ASKAPsoft includes
#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Cube.h>
#include <measures/Measures/Stokes.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>

namespace askap {
    namespace cp {

        struct CorrelatorPayload {

            /// Timestamp - Binary Atomic Time (BAT). The number of microseconds
            /// since Modified Julian Day (MJD) = 0
            casa::uLong timestamp;

            /// Coarse Channel - Which coarse channel this block of data relates to.
            casa::uInt coarseChannel;

            /// The number of rows in this block of data
            casa::uInt nRow;

            /// The number of spectral channels (equal for all rows)
            casa::uInt nChannel;

            /// The number of polarization products (equal for all rows)
            casa::uInt nPol;

            /// A vector of length nRow, with IDs of the first antenna corresponding
            /// to each visibility (one for each row)
            casa::Vector<casa::uInt> antenna1;

            /// A vector of length nRow, with IDs of the second antenna corresponding
            /// to each visibility (one for each row)
            casa::Vector<casa::uInt> antenna2;

            /// A vector of length nRow, with IDs of the first beam corresponding
            /// to each visibility (one for each row)
            casa::Vector<casa::uInt> beam1;

            /// A vector of length nRow, with IDs of the second beam corresponding
            /// to each visibility (one for each row)
            casa::Vector<casa::uInt> beam2;

            /// A vector of length nPol, indicating what polarisation products are
            /// available in the "vis" cube and what order they are in.
            casa::Vector<casa::Stokes::StokesTypes> polarisations;

            /// Visibilities (a cube is nRow x nChannel x nPol)
            /// Each element is a complex visibility.
            casa::Cube<casa::Complex> vis;

            /// The number of voltage samples that made up the visibility for
            /// this integration. This is a cube of long integers of the same
            /// dimensions as the vis cube. i.e. one nSamples value per
            /// visibility in the vis cube.
            /// No value in this cube shall exceed nominalNSamples.
            casa::Cube<casa::uInt> nSamples;

            /// The nominal number of voltage samples indicates how many there
            /// should be in the case where none are discarded.
            casa::uInt nominalNSamples;
        };

    };
};

///////////////////////
// Serializer functions
///////////////////////

/// @brief operator to store the object in a blob stream
/// @param[in] os the output stream
/// @param[in] obj a serializable object
/// @return the output steam to be able to use chain semantics
LOFAR::BlobOStream& operator<<(LOFAR::BlobOStream &os, const askap::cp::CorrelatorPayload& obj);

/// @brief operator to load an object from a blob stream
/// @param[in] is the input stream
/// @param[in] obj a serializable object
/// @return the input steam to be able to use chain semantics
LOFAR::BlobIStream& operator>>(LOFAR::BlobIStream &is, askap::cp::CorrelatorPayload& obj);

#endif

