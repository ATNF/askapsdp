/// @file CatalogElement.cc
///
/// @copyright (c) 2015 CSIRO
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
#include "casdaupload/CatalogElement.h"

// Include package level header file
#include "askap_pipelinetasks.h"

// System includes
#include <string>

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "xercesc/dom/DOM.hpp" // Includes all DOM
#include "votable/XercescString.h"
#include "votable/XercescUtils.h"

// Using
using namespace askap;
using namespace askap::cp::pipelinetasks;
using namespace xercesc;
using askap::accessors::XercescString;
using askap::accessors::XercescUtils;

CatalogElement::CatalogElement(const std::string& filename, const std::string& project)
    : itsFilename(filename), itsProject(project)
{
}

xercesc::DOMElement* CatalogElement::toXmlElement(xercesc::DOMDocument& doc) const
{
    DOMElement* e = doc.createElement(XercescString("catalog"));

    XercescUtils::addTextElement(*e, "filename", itsFilename);
    XercescUtils::addTextElement(*e, "format", "votable");
    XercescUtils::addTextElement(*e, "project", itsProject);

    return e;
}

std::string CatalogElement::getFilename(void) const
{
    return itsFilename;
}
