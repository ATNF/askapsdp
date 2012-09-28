/// @file VOTable.h
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

#ifndef ASKAP_CP_ACCESSORS_VOTABLE_VOTABLE_H
#define ASKAP_CP_ACCESSORS_VOTABLE_VOTABLE_H

// System includes
#include <string>
#include <vector>

// ASKAPsoft includes

// Local package includes
#include "votable/VOTableInfo.h"
#include "votable/VOTableResource.h"

namespace askap {
namespace accessors {

/// @brief TODO: Write documentation...
class VOTable {
    public:

        /// @brief Constructor
        VOTable(const std::string& version);

        std::string getDescription() const;
        std::vector<askap::accessors::VOTableInfo> getInfo() const;
        std::vector<askap::accessors::VOTableResource> getResource() const;

        void setDescription(const std::string& desc);

        void addResource(const askap::accessors::VOTableResource& resource);
        void addInfo(const askap::accessors::VOTableInfo& info);

        void toXml(const std::string& filename);
        static VOTable fromXML(const std::string& filename);

    private:
        std::string itsDescription;
        std::vector<askap::accessors::VOTableInfo> itsInfo;
        std::vector<askap::accessors::VOTableResource> itsResource;
};

}
}

#endif
