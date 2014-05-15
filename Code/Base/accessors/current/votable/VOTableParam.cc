/// @file VOTableParam.cc
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
#include "VOTableParam.h"

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
#include "votable/XercescUtils.h"
#include "votable/XercescString.h"

ASKAP_LOGGER(logger, ".VOTableParam");

using namespace askap;
using namespace askap::accessors;
using namespace xercesc;

VOTableParam::VOTableParam()
{
}

void VOTableParam::setDescription(const std::string& description)
{
    itsDescription = description;
}

std::string VOTableParam::getDescription() const
{
    return itsDescription;
}

void VOTableParam::setName(const std::string& name)
{
    itsName = name;
}

std::string VOTableParam::getName() const
{
    return itsName;
}

void VOTableParam::setID(const std::string& id)
{
    itsID = id;
}

std::string VOTableParam::getID() const
{
    return itsID;
}

void VOTableParam::setDatatype(const std::string& datatype)
{
    itsDatatype = datatype;
}

std::string VOTableParam::getDatatype() const
{
    return itsDatatype;
}

void VOTableParam::setArraysize(const std::string& arraysize)
{
    itsArraysize = arraysize;
}

std::string VOTableParam::getArraysize() const
{
    return itsArraysize;
}

void VOTableParam::setUnit(const std::string& unit)
{
    itsUnit = unit;
}

std::string VOTableParam::getUnit() const
{
    return itsUnit;
}

void VOTableParam::setUCD(const std::string& ucd)
{
    itsUCD = ucd;
}

std::string VOTableParam::getUCD() const
{
    return itsUCD;
}

void VOTableParam::setUType(const std::string& utype)
{
    itsUType = utype;
}

std::string VOTableParam::getUType() const
{
    return itsUType;
}

void VOTableParam::setRef(const std::string& ref)
{
    itsRef = ref;
}

std::string VOTableParam::getRef() const
{
    return itsRef;
}

void VOTableParam::setValue(const std::string& value)
{
    itsValue = value;
}

std::string VOTableParam::getValue() const
{
    return itsValue;
}

xercesc::DOMElement* VOTableParam::toXmlElement(xercesc::DOMDocument& doc) const
{
    DOMElement* e = doc.createElement(XercescString("PARAM"));

    // Add attributes
    if (itsName.length() > 0) {
        e->setAttribute(XercescString("name"), XercescString(itsName));
    }
    if (itsID.length() > 0) {
        e->setAttribute(XercescString("ID"), XercescString(itsID));
    }
    if (itsDatatype.length() > 0) {
        e->setAttribute(XercescString("datatype"), XercescString(itsDatatype));
    }
    if (itsArraysize.length() > 0) {
        e->setAttribute(XercescString("arraysize"), XercescString(itsArraysize));
    }
    if (itsUnit.length() > 0) {
        e->setAttribute(XercescString("unit"), XercescString(itsUnit));
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
    if (itsValue.length() > 0) {
        e->setAttribute(XercescString("value"), XercescString(itsValue));
    }

    // Create DESCRIPTION element
    if (itsDescription.length() > 0) {
        DOMElement* descElement = doc.createElement(XercescString("DESCRIPTION"));
        DOMText* text = doc.createTextNode(XercescString(itsDescription));
        descElement->appendChild(text);
        e->appendChild(descElement);
    }
    return e;
}

VOTableParam VOTableParam::fromXmlElement(const xercesc::DOMElement& e)
{
    VOTableParam p;

    // Get attributes
    p.setName(XercescUtils::getAttribute(e, "name"));
    p.setID(XercescUtils::getAttribute(e, "ID"));
    p.setDatatype(XercescUtils::getAttribute(e, "datatype"));
    p.setArraysize(XercescUtils::getAttribute(e, "arraysize"));
    p.setUnit(XercescUtils::getAttribute(e, "unit"));
    p.setUCD(XercescUtils::getAttribute(e, "ucd"));
    p.setUType(XercescUtils::getAttribute(e, "utype"));
    p.setRef(XercescUtils::getAttribute(e, "ref"));
    p.setValue(XercescUtils::getAttribute(e, "value"));

    // Get description
    p.setDescription(XercescUtils::getDescription(e));

    return p;
}
