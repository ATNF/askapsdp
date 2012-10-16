/// @file VOTableRow.cc
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
#include "VOTableRow.h"

// Include package level header file
#include "askap_accessors.h"

// System includes
# include <vector>
# include <string>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "boost/algorithm/string/trim.hpp"
#include "xercesc/dom/DOM.hpp" // Includes all DOM

// Local package includes
#include "votable/XercescUtils.h"
#include "votable/XercescString.h"

ASKAP_LOGGER(logger, ".VOTableRow");

using namespace askap;
using namespace askap::accessors;
using namespace xercesc;

VOTableRow::VOTableRow()
{
}

void VOTableRow::addCell(const std::string& cell)
{
    itsCells.push_back(cell);
}

std::vector<std::string> VOTableRow::getCells() const
{
    return itsCells;
}

xercesc::DOMElement* VOTableRow::toXmlElement(xercesc::DOMDocument& doc) const
{
    DOMElement* tr = doc.createElement(XercescString("TR"));

    for (std::vector<std::string>::const_iterator it = itsCells.begin();
            it != itsCells.end(); ++it) {
        DOMElement* td = doc.createElement(XercescString("TD"));
        DOMText* text = doc.createTextNode(XercescString(*it));
        td->appendChild(text);
        tr->appendChild(td);
    }

    return tr;
}

VOTableRow VOTableRow::fromXmlElement(const xercesc::DOMElement& e)
{
    VOTableRow r;

    // Process TD
    DOMNodeList* children = e.getElementsByTagName(XercescString("TD"));
    const XMLSize_t nCells = children->getLength();
    for (XMLSize_t i = 0; i < nCells; ++i) {
        const DOMElement* node = dynamic_cast<xercesc::DOMElement*>(children->item(i));
        const DOMText* text = dynamic_cast<xercesc::DOMText*>(node->getChildNodes()->item(0));
        std::string str = XercescString(text->getWholeText());
        boost::trim(str);
        r.addCell(str);
    }

    return r;
}
