/// @file VOTable.h
///
/// @copyright (c) 2012 CSIRO
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
#include <istream>
#include <ostream>

// ASKAPsoft includes
#include "xercesc/framework/XMLFormatter.hpp"
#include "xercesc/sax/InputSource.hpp"

// Local package includes
#include "votable/VOTableInfo.h"
#include "votable/VOTableResource.h"
#include "votable/VOTableField.h"
#include "votable/VOTableParam.h"
#include "votable/VOTableRow.h"
#include "votable/VOTableTable.h"
#include "votable/VOTableGroup.h"
// NOTE: Some are unnecessarily included so the user can just include VOTable.h

namespace askap {
    namespace accessors {

        /// Encapsulates a VOTable and provides the ability to serialise/de-serialise
        /// to/from XML.
        class VOTable {
            public:

                /// @brief Constructor
                VOTable(void);

                /// Get the text of the DESCRIPTION element.
                std::string getDescription() const;

                /// Get a vector containing all the INFO elements in the VOTable
                std::vector<askap::accessors::VOTableInfo> getInfo() const;

                /// Get a vector containing all the RESOURCE elements in the VOTable
                std::vector<askap::accessors::VOTableResource> getResource() const;

                /// Set the text of the DESCRIPTION element.
                void setDescription(const std::string& desc);

                /// Add a RESOURCE element to the VOTable
                void addResource(const askap::accessors::VOTableResource& resource);

                /// Add an INFO element to the VOTable
                void addInfo(const askap::accessors::VOTableInfo& info);

                /// Transform the VOTable object into an XML VOTable
                ///
                /// @param[out] os  an ostream to which the XML output string
                ///                 will be written.
                void toXML(std::ostream& os) const;

                /// Transform an XML VOTable to a VOTable object instance
                ///
                /// @param[in] is   an istream from which the XML input string
                ///                 will be read from.
                /// @return a VOTable.
                /// @throw AskapError   if the XML document is empty (i.e. no root).
                static VOTable fromXML(std::istream& is);

                /// Transform the VOTable object into an XML VOTable
                ///
                /// @param[in] filename the file/path to write the XML output to.
                void toXML(const std::string& filename) const;

                /// Transform an XML VOTable to a VOTable object instance
                ///
                /// @param[in] filename the file/path to read the XML input from.
                /// @return a VOTable.
                /// @throw AskapError   if the XML document is empty (i.e. no root)
                ///                     or if the specified file cannot be opened.
                static VOTable fromXML(const std::string& filename);

            private:

                /// Transform the VOTable object into an XML VOTable
                void toXMLImpl(xercesc::XMLFormatTarget& target) const;

                /// Transform an XML VOTable to a VOTable object instance
                ///
                /// @throw AskapError   if the XML document is empty (i.e. no root).
                static VOTable fromXMLImpl(const xercesc::InputSource& source);

                /// The text for the DESCRIPTION element
                std::string itsDescription;

                /// A list of of the INFO elements present in the VOTable
                std::vector<askap::accessors::VOTableInfo> itsInfo;

                /// A list of of the RESOURCE elements present in the VOTable
                std::vector<askap::accessors::VOTableResource> itsResource;
        };

    }
}

#endif
