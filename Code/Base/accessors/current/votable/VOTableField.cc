/// @file VOTableField.cc
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
#include "VOTableField.h"

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

ASKAP_LOGGER(logger, ".VOTableField");

using namespace askap;
using namespace askap::accessors;
using namespace xercesc;

VOTableField::VOTableField()
{
}

void VOTableField::setDescription(const std::string& description)
{
    itsDescription = description;
}

std::string VOTableField::getDescription() const
{
    return itsDescription;
}

void VOTableField::setName(const std::string& name)
{
    itsName = name;
}

std::string VOTableField::getName() const
{
    return itsName;
}

void VOTableField::setID(const std::string& id)
{
    itsID = id;
}

std::string VOTableField::getID() const
{
    return itsID;
}

void VOTableField::setDatatype(const std::string& datatype)
{
    itsDatatype = datatype;
}

std::string VOTableField::getDatatype() const
{
    return itsDatatype;
}

void VOTableField::setArraysize(const std::string& arraysize)
{
    itsArraysize = arraysize;
}

std::string VOTableField::getArraysize() const
{
    return itsArraysize;
}

void VOTableField::setUnit(const std::string& unit)
{
    itsUnit = unit;
}

std::string VOTableField::getUnit() const
{
    return itsUnit;
}

void VOTableField::setUCD(const std::string& ucd)
{
    itsUCD = ucd;
}

std::string VOTableField::getUCD() const
{
    return itsUCD;
}

void VOTableField::setUType(const std::string& utype)
{
    itsUType = utype;
}

std::string VOTableField::getUType() const
{
    return itsUType;
}

void VOTableField::setRef(const std::string& ref)
{
    itsRef = ref;
}

std::string VOTableField::getRef() const
{
    return itsRef;
}

xercesc::DOMElement* VOTableField::toXmlElement(xercesc::DOMDocument& doc) const
{
    DOMElement* e = doc.createElement(XercescString("FIELD"));

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

    // Create DESCRIPTION element
    if (itsDescription.length() > 0) {
        DOMElement* descElement = doc.createElement(XercescString("DESCRIPTION"));
        DOMText* text = doc.createTextNode(XercescString(itsDescription));
        descElement->appendChild(text);
        e->appendChild(descElement);
    }
    return e;
}

VOTableField VOTableField::fromXmlElement(const xercesc::DOMElement& e)
{
    VOTableField f;

    // Get attributes
    f.setName(XercescUtils::getAttribute(e, "name"));
    f.setID(XercescUtils::getAttribute(e, "ID"));
    f.setDatatype(XercescUtils::getAttribute(e, "datatype"));
    f.setArraysize(XercescUtils::getAttribute(e, "arraysize"));
    f.setUnit(XercescUtils::getAttribute(e, "unit"));
    f.setUCD(XercescUtils::getAttribute(e, "ucd"));
    f.setUType(XercescUtils::getAttribute(e, "utype"));
    f.setRef(XercescUtils::getAttribute(e, "ref"));

    // Get description
    f.setDescription(XercescUtils::getDescription(e));

    return f;
}
