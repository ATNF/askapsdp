/// @file CommunicatorConfig.h
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

#ifndef ASKAP_CP_COMMUNICATORCONFIG_H
#define ASKAP_CP_COMMUNICATORCONFIG_H

// System includes
#include <string>
#include <map>

// ASKAPsoft includes
#include "Ice/Ice.h"

namespace askap {
namespace cp {

/// @brief TODO
/// @ingroup iceutils
class CommunicatorConfig {
    public:
        CommunicatorConfig(const std::string& locatorHost,
                           const std::string& locatorPort);

        void setProperty(const std::string& key, const std::string& value);

        void removeProperty(const std::string& key);

        void setAdapter(const std::string& name, const std::string& endpoints);

        void removeAdapter(const std::string& name);

        Ice::PropertiesPtr convertToIceProperties(void) const;

    private:
        std::map<std::string, std::string> itsProperties;
};

};
};

#endif
