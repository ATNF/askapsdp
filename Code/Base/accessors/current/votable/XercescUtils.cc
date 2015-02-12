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
#include "boost/scoped_ptr.hpp"
#include "boost/algorithm/string/trim.hpp"
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

xercesc::DOMElement* XercescUtils::getFirstElementByTagName(const xercesc::DOMElement& element,
        const std::string& name)
{
    const DOMNodeList* children = element.getChildNodes();
    ASKAPDEBUGASSERT(children != 0);
    const XMLSize_t len = children->getLength();

    for (XMLSize_t i = 0; i < len; ++i) {
        const DOMNode* node = children->item(i);
        ASKAPDEBUGASSERT(node != 0);
        if (node->getNodeType() != DOMNode::ELEMENT_NODE) {
            continue;
        }
        DOMElement* e1 = dynamic_cast<xercesc::DOMElement*>(children->item(i));
        ASKAPDEBUGASSERT(e1 != 0);

        const string nodeName = XercescString(e1->getNodeName());
        if (nodeName.compare(name) == 0) {
            return e1;
        }
    }

    return 0;
}

std::string XercescUtils::getStringFromDOMText(const xercesc::DOMText& text)
{
        boost::scoped_ptr<const XMLCh> wholeText(text.getWholeText());
        std::string str = XercescString(wholeText.get());
        wholeText.reset(0);
        return str;
}

std::string XercescUtils::getDescription(const xercesc::DOMElement& element)
{
    // Find the DESCRIPTION node
    DOMElement* descNode = getFirstElementByTagName(element, "DESCRIPTION");

    if (!descNode || descNode->getChildNodes()->getLength() < 1) {
        return "";
    }

    const DOMText* text = dynamic_cast<xercesc::DOMText*>(descNode->getChildNodes()->item(0));
    if (text) {
        std::string desc = getStringFromDOMText(*text);
        boost::trim(desc);
        return desc;
    }

    return "";
}

xercesc::DOMElement* XercescUtils::addTextElement(xercesc::DOMElement& parent,
        const std::string& tag, const std::string& value)
{
    DOMDocument* doc = parent.getOwnerDocument();
    DOMElement* child = doc->createElement(XercescString(tag));
    DOMText* text = doc->createTextNode(XercescString(value));
    child->appendChild(text);
    parent.appendChild(child);
    return child;
}
