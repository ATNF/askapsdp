/// @file VOTableResource.h
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

#ifndef ASKAP_CP_ACCESSORS_VOTABLE_VOTABLERESOURCE_H
#define ASKAP_CP_ACCESSORS_VOTABLE_VOTABLERESOURCE_H

// System includes
#include <string>
#include <vector>

// ASKAPsoft includes
#include "xercesc/dom/DOM.hpp" // Includes all DOM

// Local package includes
#include "votable/VOTableInfo.h"
#include "votable/VOTableTable.h"

namespace askap {
    namespace accessors {

        /// @brief TODO: Write documentation...
        class VOTableResource {
            public:

                /// @brief Constructor
                VOTableResource();

                void setDescription(const std::string& description);
                std::string getDescription() const;

                void setName(const std::string& name);
                std::string getName() const;

                void setID(const std::string& ID);
                std::string getID() const;

                void setType(const std::string& type);
                std::string getType() const;

                void addInfo(const VOTableInfo& info);
                std::vector<VOTableInfo> getInfo() const;

                void addTable(const VOTableTable& table);
                std::vector<VOTableTable> getTables() const;

                xercesc::DOMElement* toXmlElement(xercesc::DOMDocument& doc) const;

            private:
                std::string itsDescription;
                std::string itsName;
                std::string itsID;
                std::string itsType;
                std::vector<VOTableInfo> itsInfo;
                std::vector<VOTableTable> itsTables;


        };

    }
}

#endif
