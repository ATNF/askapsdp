/// @file VOTableGroup.cc
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
#include "VOTableGroup.h"

// Include package level header file
#include "askap_accessors.h"

// System includes
#include <string>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "votable/XercescString.h"
#include "xercesc/dom/DOM.hpp" // Includes all DOM

// Local package includes
#include "votable/XercescString.h"
#include "votable/XercescUtils.h"
#include "votable/VOTableParam.h"

ASKAP_LOGGER(logger, ".VOTableGroup");

using namespace askap;
using namespace askap::accessors;
using namespace xercesc;

VOTableGroup::VOTableGroup()
{
}

void VOTableGroup::setDescription(const std::string& description)
{
    itsDescription = description;
}

std::string VOTableGroup::getDescription() const
{
    return itsDescription;
}

void VOTableGroup::setName(const std::string& name)
{
    itsName = name;
}

std::string VOTableGroup::getName() const
{
    return itsName;
}

void VOTableGroup::setID(const std::string& id)
{
    itsID = id;
}

std::string VOTableGroup::getID() const
{
    return itsID;
}

void VOTableGroup::setUCD(const std::string& ucd)
{
    itsUCD = ucd;
}

std::string VOTableGroup::getUCD() const
{
    return itsUCD;
}

void VOTableGroup::setUType(const std::string& utype)
{
    itsUType = utype;
}

std::string VOTableGroup::getUType() const
{
    return itsUType;
}

void VOTableGroup::setRef(const std::string& ref)
{
    itsRef = ref;
}

std::string VOTableGroup::getRef() const
{
    return itsRef;
}

void VOTableGroup::addParam(const VOTableParam& param)
{
    itsParams.push_back(param);
}

std::vector<VOTableParam> VOTableGroup::getParams() const
{
    return itsParams;
}

void VOTableGroup::addFieldRef(const std::string& fieldRef)
{
    itsFieldRefs.push_back(fieldRef);
}

std::vector<std::string> VOTableGroup::getFieldRefs() const
{
    return itsFieldRefs;
}

void VOTableGroup::addParamRef(const std::string& paramRef)
{
    itsParamRefs.push_back(paramRef);
}

std::vector<std::string> VOTableGroup::getParamRefs() const
{
    return itsParamRefs;
}

xercesc::DOMElement* VOTableGroup::toXmlElement(xercesc::DOMDocument& doc) const
{
    DOMElement* e = doc.createElement(XercescString("GROUP"));

    // Add attributes
    if (itsName.length() > 0) {
        e->setAttribute(XercescString("name"), XercescString(itsName));
    }
    if (itsID.length() > 0) {
        e->setAttribute(XercescString("ID"), XercescString(itsID));
    }
    if (itsUCD.length() > 0) {
        e->setAttribute(XercescString("ucd"), XercescString(itsUCD));
    }
    if (itsUType.length() > 0) {
        e->setAttribute(XercescString("utype"), XercescString(itsUType));
    }
    if (itsRef.length() > 0) {
        e->setAttribute(XercescString("ref"), XercescString(itsRef));
    }

    // Create DESCRIPTION element
    if (itsDescription.length() > 0) {
        DOMElement* descElement = doc.createElement(XercescString("DESCRIPTION"));
        DOMText* text = doc.createTextNode(XercescString(itsDescription));
        descElement->appendChild(text);
        e->appendChild(descElement);
    }

    // Create PARAM elements
    for (std::vector<VOTableParam>::const_iterator it = itsParams.begin();
            it != itsParams.end(); ++it) {
        e->appendChild(it->toXmlElement(doc));
    }

    // Create FIELDref elements
    for (std::vector<std::string>::const_iterator it = itsFieldRefs.begin();
            it != itsFieldRefs.end(); ++it) {
        DOMElement* fr = doc.createElement(XercescString("FIELDref"));
        fr->setAttribute(XercescString("ref"), XercescString(*it));
        e->appendChild(fr);
    }

    // Create PARAMref elements
    for (std::vector<std::string>::const_iterator it = itsParamRefs.begin();
            it != itsParamRefs.end(); ++it) {
        DOMElement* fr = doc.createElement(XercescString("PARAMref"));
        fr->setAttribute(XercescString("ref"), XercescString(*it));
        e->appendChild(fr);
    }
    return e;
}

VOTableGroup VOTableGroup::fromXmlElement(const xercesc::DOMElement& e)
{
    VOTableGroup g;

    // Get attributes
    g.setName(XercescUtils::getAttribute(e, "name"));
    g.setID(XercescUtils::getAttribute(e, "ID"));
    g.setUCD(XercescUtils::getAttribute(e, "ucd"));
    g.setUType(XercescUtils::getAttribute(e, "utype"));
    g.setRef(XercescUtils::getAttribute(e, "ref"));

    // Get description
    g.setDescription(XercescUtils::getDescription(e));

    // Process PARAM
    DOMNodeList* children = e.getElementsByTagName(XercescString("PARAM"));
    for (XMLSize_t i = 0; i < children->getLength(); ++i) {
        const DOMElement* node = dynamic_cast<xercesc::DOMElement*>(children->item(i));
        const VOTableParam param = VOTableParam::fromXmlElement(*node);
        g.addParam(param);
    }

    // Process FIELDref elements
    children = e.getElementsByTagName(XercescString("FIELDref"));
    for (XMLSize_t i = 0; i < children->getLength(); ++i) {
        const DOMElement* node = dynamic_cast<xercesc::DOMElement*>(children->item(i));
        g.addFieldRef(XercescUtils::getAttribute(*node, "ref"));
    }

    // Process PARAMref elements
    children = e.getElementsByTagName(XercescString("PARAMref"));
    for (XMLSize_t i = 0; i < children->getLength(); ++i) {
        const DOMElement* node = dynamic_cast<xercesc::DOMElement*>(children->item(i));
        g.addParamRef(XercescUtils::getAttribute(*node, "ref"));
    }

    return g;
}
