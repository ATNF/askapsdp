/// @file AdminInterface.cc
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
#include "AdminInterface.h"

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "askap/AskapLogging.h"

// Local package includes

// Using
using namespace askap;
using namespace askap::cp;
using namespace askap::interfaces::component;

ASKAP_LOGGER(logger, ".AdminInterface");

AdminInterface::AdminInterface(const Ice::CommunicatorPtr ic)
    : itsComm(ic), itsState(LOADED)
    
{
    ASKAPLOG_INFO_STR(logger, "Creating AdminInterface");
}

AdminInterface::~AdminInterface()
{
    ASKAPLOG_INFO_STR(logger, "Destroying AdminInterface");

    if (itsAdapter) {
        itsAdapter->deactivate();
        itsAdapter->waitForDeactivate();
    }

    // Shutdown ICE
    itsComm->shutdown();
    itsComm->waitForShutdown();
}

void AdminInterface::run(void)
{
    ASKAPLOG_INFO_STR(logger, "Running AdminInterface");

    ASKAPCHECK(itsComm, "Ice communicator is not initialized");
    itsAdapter = itsComm->createObjectAdapter("CentralProcessorAdapter");
    ASKAPCHECK(itsAdapter, "Creation of Ice Adapter failed");

    Ice::ObjectPtr object = this;
    itsAdapter->add(object, itsComm->stringToIdentity("CentralProcessorAdmin"));
    itsAdapter->activate(); 

    // Block here so main() can block on this
    itsComm->waitForShutdown();
}

Ice::ObjectAdapterPtr AdminInterface::createAdapter(void)
{
    Ice::ObjectAdapterPtr adapter = itsComm->createObjectAdapter("CentralProcessorAdapter");
    ASKAPCHECK(adapter, "Creation of Ice Adapter failed");

    return adapter;
}

// Ice "IComponent" interfaces
void AdminInterface::startup(const askap::interfaces::ParameterMap& params, const Ice::Current& cur)
{
    if (itsState != LOADED)
    {
        throw TransitionException("Not in UNLOADED state");
    }
    itsState = STANDBY;
}

void AdminInterface::shutdown(const Ice::Current& cur)
{
    if (itsState != STANDBY)
    {
        throw TransitionException("Not in STANDBY state");
    }
    itsState = LOADED;
}

void AdminInterface::activate(const Ice::Current& cur)
{
    if (itsState != STANDBY)
    {
        throw TransitionException("Not in STANDBY state");
    }
    itsState = ONLINE;
}

void AdminInterface::deactivate(const Ice::Current& cur)
{
    if (itsState != ONLINE)
    {
        throw TransitionException("Not in ONLINE state");
    }
    itsState = STANDBY;
}

askap::interfaces::component::ComponentTestResultSeq AdminInterface::selfTest(const Ice::Current& cur)
{
    askap::interfaces::component::ComponentTestResultSeq results;
    return results;
}

askap::interfaces::component::ComponentState AdminInterface::getState(const Ice::Current& cur)
{
    if (itsState != STANDBY)
    {
        throw CannotTestException("Not in STANDBY state");
    }
    return itsState;
}

