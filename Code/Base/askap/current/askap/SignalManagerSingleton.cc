/// @file SignalManagerSingleton.cc
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
#include "askap/SignalManagerSingleton.h"

// System includes
#include <csignal>
#include <vector>

// Local package includes
#include "askap/AskapError.h"
#include "ISignalHandler.h"

using namespace askap;

// Initialize statics
SignalManagerSingleton* SignalManagerSingleton::itsInstance;
std::vector<ISignalHandler *> SignalManagerSingleton::itsSignalHandlers(NSIG);

SignalManagerSingleton::SignalManagerSingleton()
{
}

SignalManagerSingleton* SignalManagerSingleton::instance(void)
{
    if (!itsInstance) {
        itsInstance = new SignalManagerSingleton();
    }

    return itsInstance;
}

ISignalHandler* SignalManagerSingleton::registerHandler(int signum,
        ISignalHandler* handler)
{
    ISignalHandler* old = SignalManagerSingleton::itsSignalHandlers[signum];

    // First purge any pending signals
    removeHandler(signum);

    // Then install the new signal handler
    SignalManagerSingleton::itsSignalHandlers[signum] = handler;

    struct sigaction sa;
    sa.sa_handler = SignalManagerSingleton::dispatcher;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    int err = sigaction(signum, &sa, 0);

    if (err == -1) {
        ASKAPTHROW(AskapError, "Failed to register signal handler for signal " << signum);
    }

    return old;
}

void SignalManagerSingleton::removeHandler(int signum)
{
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    int err = sigaction(signum, &sa, 0);

    if (err == -1) {
        ASKAPTHROW(AskapError, "Failed to remove signal handler for signal " << signum);
    }

    SignalManagerSingleton::itsSignalHandlers[signum] = 0;
}

void SignalManagerSingleton::dispatcher(int signum)
{
    if (SignalManagerSingleton::itsSignalHandlers[signum] != 0) {
        SignalManagerSingleton::itsSignalHandlers[signum]->handleSignal(signum);
    }
}
