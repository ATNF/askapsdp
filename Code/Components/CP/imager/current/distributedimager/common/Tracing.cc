/// @file Tracing.cc
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

// Include own header file first
#include "Tracing.h"

// System includes
#include <string>

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "mpe.h"

using namespace askap::cp;

void Tracing::createState(State s, const std::string& name, const std::string& color)
{
    const int entryId = static_cast<int>(s * 2) - 1;
    const int exitId = static_cast<int>(s * 2);
    MPE_Describe_state(entryId, exitId, name.c_str(), color.c_str());
}

void Tracing::init()
{
    if (MPE_Initialized_logging() != 0) {
        ASKAPTHROW(AskapError, "Tracing has already been initialised");
    }

    MPE_Init_log();
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) {
        createState(Send, "Send", "red");
        createState(Receive, "Receive", "blue");
        createState(Broadcast, "Broadcast", "green");
        createState(CalcNE, "CalcNE", "yellow");
        createState(SolveNE, "SolveNE", "magenta");
        createState(WriteModel, "WriteModel", "white");
    }
}

void Tracing::finish(const std::string& logfile)
{
    if (MPE_Initialized_logging() != 1) {
        ASKAPTHROW(AskapError,
                   "Tracing not initialised or has already been finaslized");
    }

    MPE_Finish_log(logfile.c_str());
}

void Tracing::entry(State s)
{
    const int id = static_cast<int>(s * 2) - 1;
    logEvent(id);
}

void Tracing::exit(State s)
{
    const int id = static_cast<int>(s * 2);
    logEvent(id);
}
void Tracing::logEvent(const int id)
{
    MPE_Log_bare_event(id);
}
