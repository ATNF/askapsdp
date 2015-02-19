/// @file CasdaFileUtils.h
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

#ifndef ASKAP_CP_PIPELINETASKS_CASDAFILEUTILS_H
#define ASKAP_CP_PIPELINETASKS_CASDAFILEUTILS_H

// System includes
#include <string>

// ASKAPsoft includes
#include "boost/filesystem.hpp"

namespace askap {
namespace cp {
namespace pipelinetasks {

/// @brief File handling utilities for the CASDA upload utility
class CasdaFileUtils {
    public:

        /// Creates a checusum file, with respect to the given file.
        ///
        /// The resulting file will have the same name as the input file,
        /// with the additional extension appended (given by CHECKSUM_EXT).
        ///
        /// This file contains three strings, each separated by a single space
        /// character:
        /// - First is a CRC-32 checksum of the content displayed as a 32 bit
        ///   lower case hexadecimal number
        /// - Second is the SHA-1 of the content displayed as a 160 bit
        ///   lower case hexadecimal number
        /// - Third is  the size of the file displayed as a 64 bit lower case
        ///   hexadecimal number
        ///
        /// @param[in] infile   the file to generate a checksum for
        static void checksumFile(const boost::filesystem::path& infile);

        /// Create a tarball of a file or directory and create a checksum file
        /// for the resulting tarfile.
        ///
        /// The checksum file is created by the checksumFile() method; see its
        /// documentation for details.
        ///
        /// @param[in] infile   the file or directory to tar and checksum
        /// @param[in] outfile  the output file path for the file (i.e. both
        ///                     the parent directory and filename, although the
        ///                     parent directory part is optional)
        static void tarAndChecksum(const boost::filesystem::path& infile,
                                   const boost::filesystem::path& outfile);

        /// The checksum file is created by the checksumFile() method; see its
        /// documentation for details.
        static void copyAndChecksum(const boost::filesystem::path& infile,
                                    const boost::filesystem::path& outfile);

        /// Write a file - This is just used to signal to CASDA the datasets
        /// in the directory are ready for ingest. This indicates no further
        /// addition or mutation of the data products in the output directory
        /// will take place and the CASDA ingest process can begin.
        static void writeReadyFile(const boost::filesystem::path& outfile);

    private:
        // Size (in bytes) of the buffer for file IO. This is effectivly the I/O
        static const size_t IO_BUFFER_SIZE = 1048576;

        // The filename extension used for checksum files
        static const std::string CHECKSUM_EXT;
};

}
}
}

#endif
