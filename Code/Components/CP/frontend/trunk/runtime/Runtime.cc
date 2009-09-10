/// @file Runtime.cc
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
#include "Runtime.h"

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "askap/AskapLogging.h"
#include "Common/ParameterSet.h"

// Local package includes
#include "runtime/Workflow.h"

// Using
using namespace askap;
using namespace askap::cp;
using askap::cp::frontend::WorkflowDesc;

ASKAP_LOGGER(logger, ".Runtime");

Runtime::Runtime(const Ice::CommunicatorPtr ic)
    : itsComm(ic)
{
    ASKAPLOG_DEBUG_STR(logger, "Creating Runtime");
}

Runtime::~Runtime()
{
    ASKAPLOG_DEBUG_STR(logger, "Destroying Runtime");
}

void Runtime::run(void)
{
    ASKAPLOG_DEBUG_STR(logger, "Running Runtime");
    ASKAPCHECK(itsComm, "Initialization of Ice communicator failed");

    itsAdapter = itsComm->createObjectAdapter("CpfeRuntimeAdapter");
    ASKAPCHECK(itsAdapter, "Creation of Ice Adapter failed");

    Ice::ObjectPtr object = this;
    itsAdapter->add(object, itsComm->stringToIdentity("cpfe_runtime1"));

    itsAdapter->activate();

    // Wait for shutdown of ICE. This occurs when a call to shutdown() on this
    // object occurs.
    itsComm->waitForShutdown();
}

// Ice "Frontend" interfaces
void Runtime::startWorkflow(const askap::cp::frontend::WorkflowDesc& wfDesc, const Ice::Current& cur)
{
    const std::string name = "cpfe_runtime1";

    // Convert the Ice Workflow description into a ParameterSet
    LOFAR::ParameterSet wfParset;
    WorkflowDesc::const_iterator it;
    for (it = wfDesc.begin(); it != wfDesc.end(); ++it) {
        wfParset.add(it->first, it->second);
    }

    itsWorkflow.reset(new Workflow(itsComm, itsAdapter, wfParset, name));
    itsWorkflow->start();
}

void Runtime::stopWorkflow(const Ice::Current& cur)
{
    itsWorkflow->stop();
    itsWorkflow.reset();
}

void Runtime::shutdown(const Ice::Current& cur)
{
    itsComm->shutdown();
}

