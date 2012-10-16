/// @file VOTableInfo.cc
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
#include "VOTableInfo.h"

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

ASKAP_LOGGER(logger, ".VOTableInfo");

using namespace askap;
using namespace askap::accessors;
using namespace xercesc;

VOTableInfo::VOTableInfo()
{
}

void VOTableInfo::setID(const std::string& id)
{
    itsID = id;
}

std::string VOTableInfo::getID() const
{
    return itsID;
}

void VOTableInfo::setName(const std::string& name)
{
    itsName = name;
}

std::string VOTableInfo::getName() const
{
    return itsName;
}

void VOTableInfo::setValue(const std::string& value)
{
    itsValue = value;
}

std::string VOTableInfo::getValue() const
{
    return itsValue;
}

void VOTableInfo::setText(const std::string& text)
{
    itsText = text;
}

std::string VOTableInfo::getText() const
{
    return itsText;
}

DOMElement* VOTableInfo::toXmlElement(DOMDocument& doc) const
{
    DOMElement* e = doc.createElement(XercescString("INFO"));

    // Add attributes
    if (itsID.length() > 0) {
        e->setAttribute(XercescString("ID"), XercescString(itsID));
    }
    if (itsName.length() > 0) {
        e->setAttribute(XercescString("name"), XercescString(itsName));
    }
    if (itsValue.length()) {
        e->setAttribute(XercescString("value"), XercescString(itsValue));
    }

    // Add text
    if (itsText.length()) {
        DOMText* text = doc.createTextNode(XercescString(itsText));
        e->appendChild(text);
    }

    return e;
}

VOTableInfo VOTableInfo::fromXmlElement(const xercesc::DOMElement& e)
{
    VOTableInfo info;

    info.setID(XercescUtils::getAttribute(e, "ID"));
    info.setName(XercescUtils::getAttribute(e, "name"));
    info.setValue(XercescUtils::getAttribute(e, "value"));

     const DOMText* text = dynamic_cast<xercesc::DOMText*>(e.getChildNodes()->item(0));
     info.setText(XercescString(text->getWholeText()));


    return info;
}
