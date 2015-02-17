/// @file CasdaChecksumFile.cc
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
#include "casdaupload/CasdaChecksumFile.h"

// Include package level header file
#include "askap_pipelinetasks.h"

// System includes
#include <string>
#include <sstream>

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "boost/crc.hpp"
#include "openssl/sha.h"

// Using
using namespace std;
using namespace askap::cp::pipelinetasks;

CasdaChecksumFile::CasdaChecksumFile(const std::string& filename)
    : itsFileSize(0)
{
    itsFile.open(filename.c_str(), ofstream::trunc);
    if (!itsFile) {
        ASKAPTHROW(AskapError, "Error opening file: " << filename);
    }
    SHA1_Init(&itsSha1Ctx);
}

CasdaChecksumFile::~CasdaChecksumFile()
{
    if (itsFile.is_open()) {
        close();
    }
}

void CasdaChecksumFile::processBytes(const char* buf, size_t sz)
{
    if (!itsFile.is_open()) {
        ASKAPTHROW(AskapError, "Checksum file is already closed");
    }
    itsFileSize += sz;
    itsCrcResult.process_bytes(buf, sz);
    SHA1_Update(&itsSha1Ctx, buf, sz);
}

void CasdaChecksumFile::close(void)
{
    if (!itsFile.is_open()) {
        ASKAPTHROW(AskapError, "Checksum file is already closed");
    }

    itsFile << std::hex << std::nouppercase
            << finaliseCrc() << " "
            << finaliseSha1() << " "
            << itsFileSize << endl;
    itsFile.close();
}

std::string CasdaChecksumFile::finaliseSha1(void)
{
    unsigned char sha1hash[SHA_DIGEST_LENGTH];
    SHA1_Final(sha1hash, &itsSha1Ctx);
    stringstream ss;
    ss << std::hex << std::nouppercase;
    for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) {
        ss << ((sha1hash[i] & 0x000000F0) >> 4) << (sha1hash[i] & 0x0000000F);
    }
    return ss.str();
}

std::string CasdaChecksumFile::finaliseCrc(void)
{
    stringstream ss;
    ss << std::hex << std::nouppercase << itsCrcResult.checksum();
    return ss.str();
}
