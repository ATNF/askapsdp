/// @file ServiceManager.h
///
/// @copyright (c) 2014 CSIRO
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

#ifndef ASKAP_CP_ICEWRAPPER_SERVICEMANAGER_H
#define ASKAP_CP_ICEWRAPPER_SERVICEMANAGER_H

// System includes
#include <string>

// ASKAPsoft includes
#include "Ice/Ice.h"

namespace askap {
namespace cp {
namespace icewrapper {

/// This class encapsulates much of the management of an Ice service.
/// @ingroup iceutils
class ServiceManager {

    public:

        /// Constructor
        ///
        /// @param[in] ic   the Ice communicator that will host the adapter
        ///                 & object
        /// @param[in] obj  the object that implements the service interface
        ///                 to be registered
        /// @param[in] serviceName  the identity of the service that will be
        ///                         registered in the locator service.
        /// @param[in] adapaterName the key used to lookup the adapter configuration
        ///                         in the Ice communicator properties.
        ServiceManager(Ice::CommunicatorPtr ic, Ice::ObjectPtr obj,
                const std::string& serviceName,
                const std::string& adapterName);

        /// Starts a service.
        ///
        /// This method performs the following:
        /// - Creates an adapter given the constructor parameter "adapterName"
        /// - Registers the service object
        /// - Activates the adapter
        void start(void);

        /// Block until shutdown has been indicated via the Ice communicator
        void waitForShutdown(void);

        /// Deactivates then destroys the Ice adapter
        void stop(void);

    private:
        // The Ice communicator - Not owned by this class
        Ice::CommunicatorPtr itsComm;

        // The object to publish - Not owned by this class
        Ice::ObjectPtr itsObject;

        // The name of the service as it will be registered in the locator
        // service
        const std::string itsServiceName;

        // The name of the adapter that will be created
        const std::string itsAdapterName;

        // A pointer to the monitoring systems object adapter. This is owned
        // by this class.
        Ice::ObjectAdapterPtr itsAdapter;
};

}
}
}

#endif
