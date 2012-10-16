/// @file XercescUtils.cc
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
#include "XercescUtils.h"

// Include package level header file
#include "askap_accessors.h"

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "xercesc/dom/DOM.hpp" // Includes all DOM
#include "xercesc/util/XMLString.hpp"

// Local package includes
#include "votable/XercescString.h"

using namespace xercesc;
using namespace askap::accessors;

std::string XercescUtils::getAttribute(const xercesc::DOMElement& element, const std::string& key)
{
    const XMLCh* val = element.getAttribute(XercescString(key));
    return XercescString(val);
}


std::string XercescUtils::getDescription(const xercesc::DOMElement& element)
{
    const DOMNodeList* children = element.getElementsByTagName(XercescString("DESCRIPTION"));
    ASKAPCHECK(children->getLength() < 2, "Expected at most one description element");
    if (children->getLength() == 1) {
        const DOMElement* descNode = dynamic_cast<xercesc::DOMElement*>(children->item(0));
        const DOMText* text = dynamic_cast<xercesc::DOMText*>(descNode->getChildNodes()->item(0));
        return XercescString(text->getWholeText());
    } else {
        return "";
    }
}

