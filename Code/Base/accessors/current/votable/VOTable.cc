/// @file VOTable.cc
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
#include "VOTable.h"

// Include package level header file
#include "askap_accessors.h"

// System includes

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"

// For XML
#include "votable/XercescString.h"
#include "xercesc/dom/DOM.hpp" // Includes all DOM
#include "xercesc/framework/LocalFileFormatTarget.hpp"
#include "xercesc/parsers/XercesDOMParser.hpp"

// Local package includes
#include "votable/VOTableInfo.h"
#include "votable/VOTableResource.h"

ASKAP_LOGGER(logger, ".VOTable");

using namespace std;
using namespace askap;
using namespace askap::accessors;
using namespace xercesc;

VOTable::VOTable(const std::string& version)
{
}

std::string VOTable::getDescription() const
{
    return itsDescription;
}

std::vector<askap::accessors::VOTableInfo> VOTable::getInfo() const
{
    return itsInfo;
}

std::vector<askap::accessors::VOTableResource> VOTable::getResource() const
{
    return itsResource;
}

void VOTable::setDescription(const std::string& desc)
{
    itsDescription = desc;
}

void VOTable::addResource(const askap::accessors::VOTableResource& resource)
{
    itsResource.push_back(resource);
}

void VOTable::addInfo(const askap::accessors::VOTableInfo& info)
{
    itsInfo.push_back(info);
}

void VOTable::toXml(const std::string& filename)
{
    // Init Xercesc
    xercesc::XMLPlatformUtils::Initialize();

    // Create document
    DOMImplementation *impl = DOMImplementationRegistry::getDOMImplementation(XercescString("LS"));
    DOMDocument* doc = impl->createDocument();
    doc->setXmlVersion(XercescString("1.0"));

    // Create the root element and add it to the document
    DOMElement* root = doc->createElement(XercescString("VOTABLE"));
    root->setAttribute(XercescString("version"), XercescString("1.2"));
    root->setAttribute(XercescString("xmlns:xsi"),
            XercescString("http://www.w3.org/2001/XMLSchema-instance"));
    root->setAttribute(XercescString("xmlns"),
            XercescString("http://www.ivoa.net/xml/VOTable/v1.2"));
    root->setAttribute(XercescString("xmlns:stc"),
            XercescString("http://www.ivoa.net/xml/STC/v1.30"));
    doc->appendChild(root);


    // Create DESCRIPTION element
    if (itsDescription != "") {
        DOMElement* descElement = doc->createElement(XercescString("DESCRIPTION"));
        DOMText* text = doc->createTextNode(XercescString(itsDescription));
        descElement->appendChild(text);
        root->appendChild(descElement);
    }

    // Create INFO elements
    for (vector<VOTableInfo>::const_iterator it = itsInfo.begin();
            it != itsInfo.end(); ++it) {
        root->appendChild(it->toXmlElement(*doc));
    }

    // Create RESOURCE elements
    for (vector<VOTableResource>::const_iterator it = itsResource.begin();
            it != itsResource.end(); ++it) {
        root->appendChild(it->toXmlElement(*doc));
    }

    // Write
    DOMLSSerializer* writer = ((DOMImplementationLS*)impl)->createLSSerializer();
    LocalFileFormatTarget* target = new LocalFileFormatTarget(XercescString(filename));

    DOMLSOutput* output = ((DOMImplementationLS*)impl)->createLSOutput();
    output->setByteStream(target);
    writer->write(root, output);

    // Cleanup
    output->release();
    delete target;
    writer->release();
    delete doc;
    xercesc::XMLPlatformUtils::Terminate();
}

askap::accessors::VOTable VOTable::fromXML(const std::string& filename)
{
    // Init Xercesc
    xercesc::XMLPlatformUtils::Initialize();

    // Setup a parser
    xercesc::XercesDOMParser* parser = new XercesDOMParser;
    parser->setValidationScheme(XercesDOMParser::Val_Never);
    parser->setDoNamespaces(false);
    parser->setDoSchema(false);
    parser->setLoadExternalDTD(false);

    // Parse file
    parser->parse(filename.c_str());

    // no need to free the doc pointer - owned by the parent parser object
    DOMDocument* doc = parser->getDocument();
    DOMElement* root = doc->getDocumentElement();
    if (!root) {
        delete parser;
        xercesc::XMLPlatformUtils::Terminate();
        ASKAPTHROW(AskapError, "empty XML document");
    }

    // Build the VOTable
    VOTable vot("1.2");

    // Cleanup
    delete parser;
    xercesc::XMLPlatformUtils::Terminate();

    return vot;
}
