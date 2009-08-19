/// @file SimpleMath.cc
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
#include "SimpleMath.h"

// ASKAPsoft includes
#include "Ice/Ice.h"
#include "askap/AskapError.h"
#include "askap/AskapLogging.h"
#include "askap/AskapUtil.h"
#include "APS/ParameterSet.h"

// Local package includes
#include "activities/IPort.h"
#include "activities/InputPort.h"
#include "activities/OutputPort.h"
#include "streams/SimpleNumber.h"

ASKAP_LOGGER(logger, ".SimpleMath");

using namespace askap::cp;
using namespace askap::cp::frontend;

SimpleMath::SimpleMath(const Ice::CommunicatorPtr ic,
        const Ice::ObjectAdapterPtr adapter,
        const LOFAR::ACC::APS::ParameterSet& parset)
    : itsComm(ic),
    itsParset(parset),
    itsInPort0(ic, adapter),
    itsInPort1(ic, adapter),
    itsOutPort0(ic)
{
    const std::string opString = toLower(itsParset.getString("op", "add"));
    if (opString == "add") {
        itsOperation = SimpleMath::ADD;
    } else if (opString == "mul") {
        itsOperation = SimpleMath::MUL;
    } else {
        ASKAPTHROW(AskapError, "Invalid operation type specified");
    }
}

SimpleMath::~SimpleMath()
{
}

void SimpleMath::run(void)
{
    const unsigned int timeout = 500;

    ASKAPLOG_INFO_STR(logger, "SimpleMath thread is running...");
    while (!stopRequested()) {
        boost::shared_ptr<SimpleNumber> a;
        while (!a) {
            a = itsInPort0.receive(timeout);
            if (stopRequested()) {
                return;
            }
        }

        boost::shared_ptr<SimpleNumber> b;
        while (!b) {
            b = itsInPort1.receive(timeout);
            if (stopRequested()) {
                return;
            }
        }

        SimpleNumber c;
        switch (itsOperation) {
            case SimpleMath::ADD :
                c.i = a->i + b->i;
                break;
            case SimpleMath::MUL :
                c.i = a->i * b->i;
                break;
            default :
                ASKAPTHROW(AskapError, "Invalid operation type");
        }
        itsOutPort0.send(c);
    }
}

void SimpleMath::attachInputPort(int port, const std::string& topic)
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

void SimpleMath::attachOutputPort(int port, const std::string& topic)
{
    switch (port) {
        case 0:
            itsOutPort0.attach(topic);
            break;
        default:
            ASKAPTHROW(AskapError, "Invalid port number");
    }
}

void SimpleMath::detachInputPort(int port)
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

void SimpleMath::detachOutputPort(int port)
{
    switch (port) {
        case 0:
            itsOutPort0.detach();
            break;
        default:
            ASKAPTHROW(AskapError, "Invalid port number");
    }
}
