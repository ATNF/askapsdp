/// @file ImageElement.cc
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
#include "casdaupload/ImageElement.h"

// Include package level header file
#include "askap_pipelinetasks.h"

// System includes
#include <string>

// ASKAPsoft includes
#include "xercesc/dom/DOM.hpp" // Includes all DOM
#include "boost/filesystem.hpp"
#include "votable/XercescString.h"
#include "votable/XercescUtils.h"

// Using
using namespace askap::cp::pipelinetasks;
using namespace xercesc;
using askap::accessors::XercescString;
using askap::accessors::XercescUtils;

ImageElement::ImageElement(const boost::filesystem::path& filepath,
                           const std::string& project)
    : itsFilepath(filepath), itsProject(project)
{
}

xercesc::DOMElement* ImageElement::toXmlElement(xercesc::DOMDocument& doc) const
{
    DOMElement* e = doc.createElement(XercescString("image"));

    XercescUtils::addTextElement(*e, "filename", itsFilepath.filename().string());
    XercescUtils::addTextElement(*e, "format", "fits");
    XercescUtils::addTextElement(*e, "project", itsProject);

    return e;
}


boost::filesystem::path ImageElement::getFilepath(void) const
{
    return itsFilepath;
}
