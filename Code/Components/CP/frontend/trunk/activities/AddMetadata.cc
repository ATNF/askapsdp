/// @file AddMetadata.cc
///
/// @copyright (c) 2009 CSIRO
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
#include "AddMetadata.h"

// ASKAPsoft includes
#include "Ice/Ice.h"
#include "askap/AskapError.h"
#include "askap/AskapLogging.h"
#include "APS/ParameterSet.h"

// Local package includes
#include "activities/IPort.h"
#include "activities/InputPort.h"
#include "activities/OutputPort.h"
#include "streams/Visibilities.h"
#include "streams/Metadata.h"

ASKAP_LOGGER(logger, ".AddMetadata");

using namespace askap::cp;
using namespace askap::cp::frontend;

AddMetadata::AddMetadata(const Ice::CommunicatorPtr ic,
        const Ice::ObjectAdapterPtr adapter,
        const LOFAR::ACC::APS::ParameterSet& parset)
    : itsComm(ic),
    itsParset(parset),
    itsInPort0(ic, adapter),
    itsInPort1(ic, adapter),
    itsOutPort0(ic)
{
}

AddMetadata::~AddMetadata()
{
}

void AddMetadata::run(void)
{
    ASKAPLOG_INFO_STR(logger, "AddMetadata thread is running...");
    //Metadata md = itsInPort0.receive();
    //Visibilities vis = itsInPort1.receive();

    //itsOutPort0.send(vis);
    while (!stopRequested()) {
    }
}

void AddMetadata::attachInputPort(int port, const std::string& topic)
{
    switch (port) {
        case 0:
            itsInPort0.attach(topic);
            break;
        case 1:
            itsInPort1.attach(topic);
            break;
        default:
            ASKAPTHROW(AskapError, "Invalid port number");
    }
}

void AddMetadata::attachOutputPort(int port, const std::string& topic)
{
    switch (port) {
        case 0:
            itsOutPort0.attach(topic);
            break;
        default:
            ASKAPTHROW(AskapError, "Invalid port number");
    }
}

void AddMetadata::detachInputPort(int port)
{
    switch (port) {
        case 0:
            itsInPort0.detach();
            break;
        case 1:
            itsInPort1.detach();
            break;
        default:
            ASKAPTHROW(AskapError, "Invalid port number");
    }
}

void AddMetadata::detachOutputPort(int port)
{
    switch (port) {
        case 0:
            itsOutPort0.detach();
            break;
        default:
            ASKAPTHROW(AskapError, "Invalid port number");
    }
}
