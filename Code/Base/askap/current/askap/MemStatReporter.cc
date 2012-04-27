/// @file MemStatReporter.cc
/// @brief
///
/// @copyright (c) 2012 CSIRO
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
#include "askap/MemStatReporter.h"

// System includes
#include <sys/types.h>
#include <unistd.h>
#include <fstream>
#include <sstream>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "askap/AskapUtil.h"

// Using
using namespace askap;

ASKAP_LOGGER(logger, ".MemStatReporter");

void MemStatReporter::logSummary(void)
{
    // Open /proc/<pid>/status
    std::stringstream ss;
    ss << "/proc/" << int(getpid()) << "/status";
    std::ifstream file(ss.str().c_str());
    if (!file) {
        ASKAPLOG_INFO_STR(logger, "Could not open procfs to obtain status");
        return;
    }

    // Find the VmPeak and RSSPeak
    int vmpeak = -1;
    int rsspeak = -1;
    while (file.good() && (vmpeak < 0 || rsspeak < 0)) {
        std::string token;
        file >> token;
        if (token.compare("VmPeak:") == 0) {
            file >> token;
            vmpeak = utility::fromString<int>(token);
            file >> token;
            if (token.compare("kB") != 0) {
                ASKAPLOG_WARN_STR(logger, "Unexpected token: " << token);
                vmpeak = -1;
            }
            continue;
        }
        if (token.compare("VmHWM:") == 0) {
            file >> token;
            rsspeak = utility::fromString<int>(token);
            file >> token;
            if (token.compare("kB") != 0) {
                ASKAPLOG_WARN_STR(logger, "Unexpected token: " << token);
                rsspeak = -1;
            }
        }
    }
    file.close();

    // Report
    ASKAPLOG_INFO_STR(logger, "Memory stats -   PeakVM: "
            << MemStatReporter::kbToMb(vmpeak) << "   PeakRSS: "
            << MemStatReporter::kbToMb(rsspeak) << "");
}

std::string MemStatReporter::kbToMb(int val)
{
    if (val < 0) {
        return "<unknown>";
    } else {
        std::string s;
        s = utility::toString(float(val / 1024));
        s.append(" MB");
        return s;
    }
}
