/// @file ServiceConfig.h
///
/// @copyright (c) 2011 CSIRO
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

#ifndef ASKAP_CP_INGEST_SERVICECONFIG_H
#define ASKAP_CP_INGEST_SERVICECONFIG_H

// System includes
#include <string>

namespace askap {
namespace cp {
namespace ingest {

/// @brief This class encapsulates the information needed to connect to
/// an Ice service.
class ServiceConfig {
    public:

        /// @brief Constructor
        /// @param[in] registryHost the hostname or IP address the registry is running on
        /// @param[in] registryPort the network port the registry is running on
        /// @param[in] serviceIdentity  the identity of the service as it is registered
        ///                             in the registry
        ServiceConfig(const std::string& registryHost,
                      const std::string& registryPort,
                      const std::string& serviceIdentity);

        /// @return the hostname or IP address the registry is running on
        std::string registryHost(void) const;

        /// @return the network port the registry is running on
        std::string registryPort(void) const;

        /// @return the identity of the service as it is registered in the registry
        std::string serviceIdentity(void) const;

    private:

        std::string itsRegistryHost;
        std::string itsRegistryPort;
        std::string itsServiceIdentity;
};

}
}
}

#endif
