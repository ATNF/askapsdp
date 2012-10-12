/// @file VOTableTable.cc
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

// Include own header file first
#include "VOTableTable.h"

// Include package level header file
#include "askap_accessors.h"

// System includes
#include <string>
#include <vector>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "votable/XercescString.h"
#include "xercesc/dom/DOM.hpp" // Includes all DOM

// Local package includes
#include "votable/VOTableField.h"
#include "votable/VOTableRow.h"

ASKAP_LOGGER(logger, ".VOTableTable");

using namespace askap;
using namespace askap::accessors;
using namespace xercesc;

VOTableTable::VOTableTable()
{
}

void VOTableTable::setID(const std::string& id)
{
    itsID = id;
}

std::string VOTableTable::getID() const
{
    return itsID;
}

void VOTableTable::setName(const std::string& name)
{
    itsName = name;
}

std::string VOTableTable::getName() const
{
    return itsName;
}

void VOTableTable::setDescription(const std::string& description)
{
    itsDescription = description;
}

std::string VOTableTable::getDescription() const
{
    return itsDescription;
}

void VOTableTable::addGroup(const VOTableGroup& group)
{
    itsGroups.push_back(group);
}

void VOTableTable::addField(const VOTableField& field)
{
    itsFields.push_back(field);
}

void VOTableTable::addRow(const VOTableRow& row)
{
    itsRows.push_back(row);
}

xercesc::DOMElement* VOTableTable::toXmlElement(xercesc::DOMDocument& doc) const
{
    DOMElement* e = doc.createElement(XercescString("TABLE"));

    // Add attributes
    if (itsID.length()) {
        e->setAttribute(XercescString("ID"), XercescString(itsID));
    }
    if (itsName.length() > 0) {
        e->setAttribute(XercescString("name"), XercescString(itsName));
    }

    // Create DESCRIPTION element
    if (itsDescription.length() > 0) {
        DOMElement* descElement = doc.createElement(XercescString("DESCRIPTION"));
        DOMText* text = doc.createTextNode(XercescString(itsDescription));
        descElement->appendChild(text);
        e->appendChild(descElement);
    }

    // Create GROUP elements
    for (std::vector<VOTableGroup>::const_iterator it = itsGroups.begin();
            it != itsGroups.end(); ++it) {
        e->appendChild(it->toXmlElement(doc));
    }

    // Create FIELD elements
    for (std::vector<VOTableField>::const_iterator it = itsFields.begin();
            it != itsFields.end(); ++it) {
        e->appendChild(it->toXmlElement(doc));
    }

    // Create DATA element
    DOMElement* dataElement = doc.createElement(XercescString("DATA"));
    e->appendChild(dataElement);

    // Create TABLEDATA element
    DOMElement* tableDataElement = doc.createElement(XercescString("TABLEDATA"));
    dataElement->appendChild(tableDataElement);

    // Add rows
    for (std::vector<VOTableRow>::const_iterator it = itsRows.begin();
            it != itsRows.end(); ++it) {
        tableDataElement->appendChild(it->toXmlElement(doc));
    }

    return e;
}
