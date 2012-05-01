/// @file StatReporter.h
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

#ifndef ASKAP_STATREPORTER_H
#define ASKAP_STATREPORTER_H

// System includes
#include <string>
#include <fstream>

// ASKAPSoft includes
#include "casa/OS/Timer.h"

namespace askap {

    /// Supports the logging of statistics (memory usage, cpu times) for a process.
    /// This class should be instantiated at process start time, and at process
    /// exit the logSummary() method should be called.
    class StatReporter
    {
        public:
            /// Constructor
           StatReporter();

            /// Report a summary of process memory usage and run/cpu times
            /// to the log. The run/cpu times will be since this class was
            /// instantiated, not since the process was forked.
            void logSummary(void);

            /// Report a summary of memory usage to the log 
            void logMemorySummary(void);

            /// Report a summary of run/cpu times to the log. The run/cpu
            /// times will be since this class was instantiated, not since
            /// the process was forked.
            void logTimeSummary(void);

        private:
            // Utility function - parses the two tokens which should be
            // an integer (size in kB), then the token "kB".
            // @return The integer (i.e. the first token) or -1 in the case the
            //          first token was not an integer or the second token was
            //          not the string "kB".
            long parseValue(std::ifstream& file);

            // Convert "val" to a string
            // @return The value as a string with " MB" appended to the end.
            static std::string kbToMb(long val);

            // Casa timer to measure process execution times
            casa::Timer itsTimer;
    };

} // End namespace askap

#endif
