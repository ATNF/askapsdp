/// @file VOTableResource.cc
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
#include "VOTableResource.h"

// Include package level header file
#include "askap_accessors.h"

// System includes
#include <string>
#include <vector>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "xercesc/dom/DOM.hpp" // Includes all DOM

// Local package includes
#include "votable/XercescString.h"
#include "votable/XercescUtils.h"
#include "votable/VOTableInfo.h"
#include "votable/VOTableTable.h"

ASKAP_LOGGER(logger, ".VOTableResource");

using namespace askap;
using namespace askap::accessors;
using namespace xercesc;

VOTableResource::VOTableResource()
{
}

void VOTableResource::setDescription(const std::string& description)
{
    itsDescription = description;
}

std::string VOTableResource::getDescription() const
{
    return itsDescription;
}

void VOTableResource::setName(const std::string& name)
{
    itsName = name;
}

std::string VOTableResource::getName() const
{
    return itsName;
}

void VOTableResource::setID(const std::string& ID)
{
    itsID = ID;
}

std::string VOTableResource::getID() const
{
    return itsID;
}

void VOTableResource::setType(const std::string& type)
{
    itsType = type;
}

std::string VOTableResource::getType() const
{
    return itsType;
}

void VOTableResource::addInfo(const VOTableInfo& info)
{
    itsInfo.push_back(info);
}

std::vector<VOTableInfo> VOTableResource::getInfo() const
{
    return itsInfo;
}

void VOTableResource::addTable(const VOTableTable& table)
{
    itsTables.push_back(table);
}

std::vector<VOTableTable> VOTableResource::getTables() const
{
    return itsTables;
}

xercesc::DOMElement* VOTableResource::toXmlElement(xercesc::DOMDocument& doc) const
{
    DOMElement* e = doc.createElement(XercescString("RESOURCE"));

    // Add attributes
    if (itsID.length() > 0) {
        e->setAttribute(XercescString("ID"), XercescString(itsID));
    }
    if (itsName.length() > 0) {
        e->setAttribute(XercescString("name"), XercescString(itsName));
    }
    if (itsType.length() > 0) {
        e->setAttribute(XercescString("type"), XercescString(itsType));
    }

    // Create DESCRIPTION element
    if (itsDescription.length() > 0) {
        DOMElement* descElement = doc.createElement(XercescString("DESCRIPTION"));
        DOMText* text = doc.createTextNode(XercescString(itsDescription));
        descElement->appendChild(text);
        e->appendChild(descElement);
    }

    // Create INFO elements
    for (std::vector<VOTableInfo>::const_iterator it = itsInfo.begin();
            it != itsInfo.end(); ++it) {
        e->appendChild(it->toXmlElement(doc));
    }

    // Create TABLE elements
    for (std::vector<VOTableTable>::const_iterator it = itsTables.begin();
            it != itsTables.end(); ++it) {
        e->appendChild(it->toXmlElement(doc));
    }

    return e;
}

VOTableResource VOTableResource::fromXmlElement(const xercesc::DOMElement& e)
{
    VOTableResource res;

    // Get attributes
    res.setID(XercescUtils::getAttribute(e, "ID"));
    res.setName(XercescUtils::getAttribute(e, "name"));
    res.setType(XercescUtils::getAttribute(e, "type"));

    // Get description
    res.setDescription(XercescUtils::getDescription(e));

    // Process INFO
    DOMNodeList* children = e.getElementsByTagName(XercescString("INFO"));
    for (XMLSize_t i = 0; i < children->getLength(); ++i) {
        const DOMElement* node = dynamic_cast<xercesc::DOMElement*>(children->item(i));
        const VOTableInfo info = VOTableInfo::fromXmlElement(*node);
        res.addInfo(info);
    }

    // Process TABLE
    children = e.getElementsByTagName(XercescString("TABLE"));
    for (XMLSize_t i = 0; i < children->getLength(); ++i) {
        const DOMElement* node = dynamic_cast<xercesc::DOMElement*>(children->item(i));
        const VOTableTable tab = VOTableTable::fromXmlElement(*node);
        res.addTable(tab);
    }

    return res;
}
