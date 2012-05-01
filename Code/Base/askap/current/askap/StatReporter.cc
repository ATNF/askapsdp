/// @file StatReporter.cc
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

// Include own header file first
#include "askap/StatReporter.h"

// System includes
#include <sys/types.h>
#include <unistd.h>
#include <fstream>
#include <sstream>

// OSX specific
#ifdef __MACH__
#include <sys/resource.h>
#endif

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "askap/AskapUtil.h"
#include "casa/OS/Timer.h"

// Using
using namespace askap;

ASKAP_LOGGER(logger, ".StatReporter");

StatReporter::StatReporter()
{
    itsTimer.mark();
}

void StatReporter::logSummary(void)
{
    logMemorySummary();
    logTimeSummary();
}

void StatReporter::logTimeSummary(void)
{
    ASKAPLOG_INFO_STR(logger, "Total times  - user: " << itsTimer.user()
            << "  system: " << itsTimer.system()
            << "  real: " << itsTimer.real());
}

void StatReporter::logMemorySummary(void)
{
    long vmpeak = -1;
    long rsspeak = -1;

#ifdef __MACH__
    struct rusage ru;
    int err = getrusage(RUSAGE_SELF, &ru);
    if (err != 0) {
        ASKAPLOG_INFO_STR(logger,
                "Memory stats - Error: getrusage() failed (" << err << ")");
        return;
    }

    rsspeak = ru.ru_maxrss / 1024L; // ru_maxrss is in bytes

#else // __MACH__

    // Open /proc/<pid>/status
    std::stringstream ss;
    ss << "/proc/" << int(getpid()) << "/status";
    std::ifstream file(ss.str().c_str());
    if (!file) {
        ASKAPLOG_INFO_STR(logger,
                "Memory stats - Error: Could not open procfs to obtain status");
        return;
    }

    // Find the VmPeak and RSSPeak
    while (file.good() && (vmpeak < 0 || rsspeak < 0)) {
        std::string token;
        file >> token;
        if (token.compare("VmPeak:") == 0) {
            vmpeak = parseValue(file);
            continue;
        }
        if (token.compare("VmHWM:") == 0) {
            rsspeak = parseValue(file);
            continue;
        }
    }
    file.close();

#endif //__MACH__

    // Report
    ASKAPLOG_INFO_STR(logger, "Memory stats - PeakVM: "
            << StatReporter::kbToMb(vmpeak) << "  PeakRSS: "
            << StatReporter::kbToMb(rsspeak));
}

long StatReporter::parseValue(std::ifstream& file)
{
    long val = -1;
    std::string token;
    file >> token;
    try {
        val = utility::fromString<long>(token);
        file >> token;
        if (token.compare("kB") != 0) {
            ASKAPLOG_WARN_STR(logger, "Unexpected token: " << token);
            val = -1;
        }
    } catch (AskapError&) {
        val = -1;
    }
    return val;
}

std::string StatReporter::kbToMb(long val)
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
