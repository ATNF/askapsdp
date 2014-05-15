/// @file
///
/// Provides general utility functions to support moving objects around with LOFAR Blobs
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
/// @author Matthew Whiting <matthew.whiting@csiro.au>
///
#ifndef ASKAP_ANALYSIS_BLOBRELATED_H_
#define ASKAP_ANALYSIS_BLOBRELATED_H_

#include <string>
#include <vector>

#include <sourcefitting/RadioSource.h>

#include <duchamp/Detection/detection.hh>

#include <Common/LofarTypedefs.h>
using namespace LOFAR::TYPES;
#include <Blob/BlobString.h>
#include <Blob/BlobIBufString.h>
#include <Blob/BlobOBufString.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>
#include <Common/Exceptions.h>

namespace askap {
    namespace analysis {

        /// @brief Send a RadioSource over a Blob Stream
        /// @ingroup blobrelated
        /*     LOFAR::BlobOStream& operator<<(LOFAR::BlobOStream& blob, sourcefitting::RadioSource& src); */

        /// @brief Receive a duchamp::Detection from a Blob Stream
        /// @ingroup blobrelated
        /*     duchamp::Detection receiveDetection(LOFAR::BlobIStream blob); */
    }

}


#endif
