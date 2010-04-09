/// @file CommunicatorFactory.h
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

// System includes
#include <string>
#include <map>

// ASKAPsoft includes
#include "Ice/Ice.h"

#ifndef ASKAP_CP_COMMUNICATORFACTORY_H
#define ASKAP_CP_COMMUNICATORFACTORY_H

namespace askap {
    namespace cp {

        class CommunicatorFactory
        {
            public:

                /// @brief Creates an IceCommunicator given a locator hostname
                /// or IP-address plus a port number.
                ///
                /// @param[in] locatorHost the hostname or IP-address of the
                ///     locator instance the created IceCommunicator instance
                ///     will use.
                /// @param[in] locatorPort the port of the locator instance
                ///     the created IceCommunicator instance will use.
                ///
                /// @return a pointer to an Ice communicator.
                Ice::CommunicatorPtr createCommunicator(
                        const std::string& locatorHost,
                        const std::string& locatorPort);
        };

    };
};

#endif
