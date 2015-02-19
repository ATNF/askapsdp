/// @file CasdaFileUtils.cc
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

// Include own header file first
#include "casdaupload/CasdaFileUtils.h"

// Include package level header file
#include "askap_pipelinetasks.h"

// System includes
#include <string>
#include <vector>
#include <fstream>

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "askap/AskapUtil.h"
#include "casa/Quanta/MVTime.h"
#include "boost/filesystem.hpp"

// Local package includes
#include "casdaupload/CasdaChecksumFile.h"

// Using
using namespace std;
using namespace askap;
using namespace askap::cp::pipelinetasks;
namespace fs = boost::filesystem;

// Initialise statics
const std::string askap::cp::pipelinetasks::CasdaFileUtils::CHECKSUM_EXT = ".checksum";

void CasdaFileUtils::tarAndChecksum(const fs::path& infile, const fs::path& outfile)
{
    stringstream cmd;
    cmd << "tar -cf " << outfile << " ";

    // If the infile has a parent path, either relative or absolute, we need
    // to have tar change to the parent path first. For example the path
    // "/foo/bar/dataset.ms" has a parent path "/foo/bar". Failure to do this
    // results in the parent path incorporated in the tarfile, where in the
    // above example we want the contents of the tarfile to be rooted at
    // directory "dataset.ms"
    if (infile.has_parent_path()) {
        cmd << "--directory " << infile.parent_path() << " " << infile.filename();
    } else {
        cmd << infile;
    }

    const int status = system(cmd.str().c_str());
    if (status != 0) {
        ASKAPTHROW(AskapError, "Tar command failed with error code: " << status
                   << " - Cmd: " << cmd);
    }
    checksumFile(outfile);
}

void CasdaFileUtils::checksumFile(const fs::path& infile)
{
    const string checksumFile = infile.string() + CHECKSUM_EXT;
    CasdaChecksumFile csum(checksumFile);

    ifstream src(infile.c_str(), std::ios::binary);
    vector<char> buffer(IO_BUFFER_SIZE);
    do {
        src.read(&buffer[0], IO_BUFFER_SIZE);
        csum.processBytes(&buffer[0], src.gcount());
    } while (src);
}

void CasdaFileUtils::copyAndChecksum(const boost::filesystem::path& infile,
                                     const boost::filesystem::path& outfile)
{
    if (!exists(infile)) {
        ASKAPTHROW(AskapError, "File not found: " << infile);
    }
    if (is_directory(infile)) {
        ASKAPTHROW(AskapError, "Error: " << infile << " is a directory, expecting a file");
    }

    const string checksumFile = outfile.string() + CHECKSUM_EXT;
    CasdaChecksumFile csum(checksumFile);

    ifstream src(infile.c_str(), std::ios::binary);
    ofstream dst(outfile.c_str(), std::ios::binary);
    vector<char> buffer(IO_BUFFER_SIZE);
    do {
        src.read(&buffer[0], IO_BUFFER_SIZE);
        const streamsize readsz = src.gcount();
        csum.processBytes(&buffer[0], readsz);
        dst.write(&buffer[0], readsz);
        if (!dst) {
            ASKAPTHROW(AskapError, "Error writing to file " << outfile);
        }
    } while (src);
}

void CasdaFileUtils::writeReadyFile(const boost::filesystem::path& outfile)
{
    ofstream fs(outfile.c_str());
    casa::Quantity today;
    casa::MVTime::read(today, "today");
    fs << casa::MVTime(today).string(casa::MVTime::FITS) << endl;
    if (!fs) {
        ASKAPTHROW(AskapError, "Error writing READY file");
    }
    fs.close();
}
