/// @file Tracing.h
///
/// @copyright (c) 2010 CSIRO
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

#ifndef ASKAP_CP_TRACING_H
#define ASKAP_CP_TRACING_H

// System includes
#include <string>

namespace askap {
namespace cp {

class Tracing {
    public:

        // Must be called to initialize the Tracing framework
        static void init();

        // Called to write out the log file and finalise the
        // tracing framework.
        // @param logfile[in] filename/path of the log file to write out
        static void finish(const std::string& logfile);

        // Enum of valid states
        //
        // @todo This is a bit of a hack. A better solution would be
        // to dynamically manage states, using MPE_Log_get_event_number()
        // to allocate IDs. The hardcoded IDs below start at 600 simply
        // because that is what MPE_Log_get_event_number() seems to
        // start with and it ensure they are clear of any private/internal
        // IDs.
        enum State {
            Send = 600,
            Receive = 601,
            Broadcast = 602,
            CalcNE = 603,
            SolveNE = 604,
            WriteModel = 605
        };

        // Call this to indicate state entry
        // @param s[in] the state which is being entered.
        static void entry(State s);

        // Call this to indicate state exit
        // @param s[in] the state which is being exited.
        static void exit(State s);

    private:
        // Utility function to create states
        static void createState(State s, const std::string& name, const std::string& color);

        // Utility function to log events
        static void logEvent(const int id);

        // No support for assignment
        Tracing& operator=(const Tracing& rhs);

        // No support for copy constructor
        Tracing(const Tracing& src);
};

}
}

#endif
