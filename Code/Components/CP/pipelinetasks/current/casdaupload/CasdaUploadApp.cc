/// @file CasdaUploadApp.cc
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
#include "casdaupload/CasdaUploadApp.h"

// Include package level header file
#include "askap_pipelinetasks.h"

// System includes
#include <string>
#include <vector>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "askap/AskapUtil.h"
#include "Common/ParameterSet.h"
#include "askap/StatReporter.h"
#include "casa/aipstype.h"
#include "boost/scoped_ptr.hpp"
#include "xercesc/dom/DOM.hpp" // Includes all DOM
#include "xercesc/framework/LocalFileFormatTarget.hpp"
#include "xercesc/framework/XMLFormatter.hpp"
#include "votable/XercescString.h"
#include "votable/XercescUtils.h"

// Local package includes
#include "casdaupload/IdentityElement.h"
#include "casdaupload/ObservationElement.h"
#include "casdaupload/ImageElement.h"
#include "casdaupload/CatalogElement.h"
#include "casdaupload/MeasurementSetElement.h"
#include "casdaupload/EvaluationReportElement.h"

// Using
using namespace std;
using namespace askap;
using namespace askap::cp::pipelinetasks;
using namespace casa;
using namespace xercesc;
using askap::accessors::XercescString;

ASKAP_LOGGER(logger, ".CasdaUploadApp");

int CasdaUploadApp::run(int argc, char* argv[])
{
    StatReporter stats;

    IdentityElement identity(config());

    std::vector<ImageElement> images(buildImageElements());
    std::vector<CatalogElement> catalogs(buildCatalogElements());
    std::vector<MeasurementSetElement> ms(buildMeasurementSetElements());
    std::vector<EvaluationReportElement> reports(buildEvaluationElements());

    ObservationElement obs;

    // If a measurement set is present, we can determine the time range for the
    // observation. Note, only the first measurement set (if there are multiple)
    // is used in this calculation.
    if (!ms.empty()) {
        MeasurementSetElement& firstMs = ms[0];
        obs.setObsTimeRange(firstMs.getObsStart(), firstMs.getObsEnd());
    }

    generateMetadataFile(identity, obs, images, catalogs, ms, reports);

    stats.logSummary();
    return 0;
}

void CasdaUploadApp::generateMetadataFile(
    const IdentityElement& identity,
    const ObservationElement& obs,
    const std::vector<ImageElement>& images,
    const std::vector<CatalogElement>& catalogs,
    const std::vector<MeasurementSetElement>& ms,
    const std::vector<EvaluationReportElement>& reports)
{
    const std::string filename = "metadata.xml";

    xercesc::XMLPlatformUtils::Initialize();

    boost::scoped_ptr<LocalFileFormatTarget> target(new LocalFileFormatTarget(XercescString(filename)));

    // Create document
    DOMImplementation *impl = DOMImplementationRegistry::getDOMImplementation(XercescString("LS"));
    DOMDocument* doc = impl->createDocument();
    doc->setXmlVersion(XercescString("1.0"));
    doc->setXmlStandalone(true);

    // Create the root element and add it to the document
    DOMElement* root = doc->createElement(XercescString("dataset"));
    doc->appendChild(root);

    // Add identity element
    root->appendChild(identity.toXmlElement(*doc));

    // Add observation element
    root->appendChild(obs.toXmlElement(*doc));

    // Create image elements
    if (!images.empty()) {
        DOMElement* imagesElement = doc->createElement(XercescString("images"));
        for (vector<ImageElement>::const_iterator it = images.begin();
                it != images.end(); ++it) {
            imagesElement->appendChild(it->toXmlElement(*doc));
        }
        root->appendChild(imagesElement);
    }

    // Create catalog elements
    if (!catalogs.empty()) {
        DOMElement* catalogElement = doc->createElement(XercescString("catalogs"));
        for (vector<CatalogElement>::const_iterator it = catalogs.begin();
                it != catalogs.end(); ++it) {
            catalogElement->appendChild(it->toXmlElement(*doc));
        }
        root->appendChild(catalogElement);
    }

    // Create measurement set elements
    if (!ms.empty()) {
        DOMElement* msElement = doc->createElement(XercescString("measurement_sets"));
        for (vector<MeasurementSetElement>::const_iterator it = ms.begin();
                it != ms.end(); ++it) {
            msElement->appendChild(it->toXmlElement(*doc));
        }
        root->appendChild(msElement);
    }

    // Create evalation report elements
    if (!reports.empty()) {
        DOMElement* reportsElement = doc->createElement(XercescString("evaluation"));
        for (vector<EvaluationReportElement>::const_iterator it = reports.begin();
                it != reports.end(); ++it) {
            reportsElement->appendChild(it->toXmlElement(*doc));
        }
        root->appendChild(reportsElement);
    }

    // Write
    DOMLSSerializer* writer = ((DOMImplementationLS*)impl)->createLSSerializer();

    if (writer->getDomConfig()->canSetParameter(XMLUni::fgDOMWRTFormatPrettyPrint, true)) {
        writer->getDomConfig()->setParameter(XMLUni::fgDOMWRTFormatPrettyPrint, true);
    }

    DOMLSOutput* output = ((DOMImplementationLS*)impl)->createLSOutput();
    output->setByteStream(target.get());
    writer->write(doc, output);

    // Cleanup
    output->release();
    writer->release();
    doc->release();
    target.reset(0);
    xercesc::XMLPlatformUtils::Terminate();
}

std::vector<ImageElement> CasdaUploadApp::buildImageElements() const
{
    const string KEY = "images.artifactlist";
    vector<ImageElement> elements;

    if (config().isDefined(KEY)) {
        const vector<string> names = config().getStringVector(KEY);
        for (vector<string>::const_iterator it = names.begin(); it != names.end(); ++it) {
            const LOFAR::ParameterSet subset = config().makeSubset(*it + ".");
            const string filename = subset.getString("filename");
            const string project = subset.getString("project");
            elements.push_back(ImageElement(filename, project));
        }
    }

    return elements;
}

std::vector<MeasurementSetElement> CasdaUploadApp::buildMeasurementSetElements(void) const
{
    const string KEY = "measurementsets.artifactlist";
    vector<MeasurementSetElement> elements;

    if (config().isDefined(KEY)) {
        const vector<string> names = config().getStringVector(KEY);
        for (vector<string>::const_iterator it = names.begin(); it != names.end(); ++it) {
            const LOFAR::ParameterSet subset = config().makeSubset(*it + ".");
            const string filename = subset.getString("filename");
            const string project = subset.getString("project");
            elements.push_back(MeasurementSetElement(filename, project));
        }
    }

    return elements;
}

std::vector<CatalogElement> CasdaUploadApp::buildCatalogElements(void) const
{
    const string KEY = "catalogs.artifactlist";
    vector<CatalogElement> elements;

    if (config().isDefined(KEY)) {
        const vector<string> names = config().getStringVector(KEY);
        for (vector<string>::const_iterator it = names.begin(); it != names.end(); ++it) {
            const LOFAR::ParameterSet subset = config().makeSubset(*it + ".");
            const string filename = subset.getString("filename");
            const string project = subset.getString("project");
            elements.push_back(CatalogElement(filename, project));
        }
    }

    return elements;
}

std::vector<EvaluationReportElement> CasdaUploadApp::buildEvaluationElements(void) const
{
    const string KEY = "evaluation.artifactlist";
    vector<EvaluationReportElement> elements;

    if (config().isDefined(KEY)) {
        const vector<string> names = config().getStringVector(KEY);
        for (vector<string>::const_iterator it = names.begin(); it != names.end(); ++it) {
            const LOFAR::ParameterSet subset = config().makeSubset(*it + ".");
            const string filename = subset.getString("filename");
            elements.push_back(EvaluationReportElement(filename));
        }
    }

    return elements;
}
