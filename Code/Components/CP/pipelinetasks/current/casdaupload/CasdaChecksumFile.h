/// @file CasdaChecksumFile.h
///
/// @copyright (c) 2015 CSIRO
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

#ifndef ASKAP_CP_PIPELINETASKS_CASDACHECKSUMFILE_H
#define ASKAP_CP_PIPELINETASKS_CASDACHECKSUMFILE_H

// System includes
#include <string>
#include <fstream>

// ASKAPsoft includes
#include "boost/crc.hpp"
#include "openssl/sha.h"

namespace askap {
namespace cp {
namespace pipelinetasks {

/// Handles the creation of the CASDA checksum file.
///
/// Usage of this class involves first creating the object, followed by one or
/// more class to processBytes(), then a call to close() which actually writes
/// the checksum file. If the call to close is omitted the file is written when
/// the descructor is called.
class CasdaChecksumFile {
    public:
        /// Constructor
        /// @param[in] filename the filename of the checkdum file. This may be
        ///                     just a filename, or may be a relative or
        ///                     absolute path.
        /// @throws AskapError if the file could not be opened for writing
        CasdaChecksumFile(const std::string& filename);

        /// Destructor
        /// If the checksums have not been finalised and written to the output
        /// file by a call to close() then close() will be called by the
        /// destructor.
        ~CasdaChecksumFile();

        /// Apply these bytes to the checksum generator.
        /// @param[in] buf  a pointer to the bytes to apply
        /// @param[in] sz   number of bytes from the buffer to read
        /// @throws AskapError if close() has already been called
        void processBytes(const char* buf, size_t sz);

        /// Finalises the checksum creation and writes the output file.
        /// @throws AskapError if close() has already been called
        void close(void);

    private:
        // Returns the SHA1 checksum. This method should only be called
        // once per object. Failure to do so has unspecified behaviour.
        std::string finaliseSha1(void);

        // Returns the CRC32 checksum. This method should only be called
        // once per object. Failure to do so has unspecified behaviour.
        std::string finaliseCrc(void);

        // The size of the file. Since the file size is written as part of the
        // checksum file, we determine the size of the file via summation of the
        // "sz" values passed to processBytes()
        size_t itsFileSize;

        // The file to which the checksums will be written
        std::ofstream itsFile;

        // Context for the ongoing creation of a CRC32 checksum
        boost::crc_32_type itsCrcResult;

        // Context for the ongoing creation of a SHA1 checksum
        SHA_CTX itsSha1Ctx;
};

}
}
}

#endif
