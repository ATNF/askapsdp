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
#include "Common/ParameterSet.h"
#include "askap/StatReporter.h"
#include "boost/scoped_ptr.hpp"
#include "boost/filesystem.hpp"
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
#include "casdaupload/CasdaChecksumFile.h"
#include "casdaupload/CasdaFileUtils.h"

// Using
using namespace std;
using namespace askap::cp::pipelinetasks;
using namespace xercesc;
using askap::accessors::XercescString;
namespace fs = boost::filesystem;

ASKAP_LOGGER(logger, ".CasdaUploadApp");

int CasdaUploadApp::run(int argc, char* argv[])
{
    StatReporter stats;

    const IdentityElement identity(config());

    const vector<ImageElement> images(
        buildArtifactElements<ImageElement>("images.artifactlist"));
    const vector<CatalogElement> catalogs(
        buildArtifactElements<CatalogElement>("catalogs.artifactlist"));
    const vector<MeasurementSetElement> ms(
        buildArtifactElements<MeasurementSetElement>("measurementsets.artifactlist"));
    const vector<EvaluationReportElement> reports(
        buildArtifactElements<EvaluationReportElement>("evaluation.artifactlist", false));

    if (images.empty() && catalogs.empty() && ms.empty()) {
        ASKAPTHROW(AskapError, "No artifacts declared for upload");
    }

    // If a measurement set is present, we can determine the time range for the
    // observation. Note, only the first measurement set (if there are multiple)
    // is used in this calculation.
    ObservationElement obs;
    if (!ms.empty()) {
        if (ms.size() > 1) {
            ASKAPLOG_WARN_STR(logger, "Multiple measurement set were specified."
                              << " Only the first one will be used to populate the"
                              << " observation metadata");
        }
        const MeasurementSetElement& firstMs = ms[0];
        obs.setObsTimeRange(firstMs.getObsStart(), firstMs.getObsEnd());
    }

    // Create the output directory
    const fs::path outbase(config().getString("outputdir"));
    if (!is_directory(outbase)) {
        ASKAPTHROW(AskapError, "Directory " << outbase
                   << " does not exists or is not a directory");
    }
    const fs::path outdir = outbase / config().getString("sbid");
    ASKAPLOG_INFO_STR(logger, "Using output directory: " << outdir);
    if (!is_directory(outdir)) {
        create_directory(outdir);
    }
    if (!exists(outdir)) {
        ASKAPTHROW(AskapError, "Failed to create directory " << outdir);
    }
    const fs::path metadataFile = outdir / "observation.xml";
    generateMetadataFile(metadataFile, identity, obs, images, catalogs, ms, reports);
    CasdaFileUtils::checksumFile(metadataFile);

    // Tar up measurement sets
    for (vector<MeasurementSetElement>::const_iterator it = ms.begin();
            it != ms.end(); ++it) {
        const fs::path in(it->getFilepath());
        fs::path out(outdir / in.filename());
        out += ".tar";
        ASKAPLOG_INFO_STR(logger, "Tarring file " << in << " to " << out);
        CasdaFileUtils::tarAndChecksum(in, out);
    }

    // Copy artifacts and checksum
    copyAndChecksumElements<ImageElement>(images, outdir);
    copyAndChecksumElements<CatalogElement>(catalogs, outdir);
    copyAndChecksumElements<EvaluationReportElement>(reports, outdir);

    // Finally, and specifically as the last step, write the READY file
    const fs::path readyFilename = outdir / "READY";
    CasdaFileUtils::writeReadyFile(readyFilename);

    stats.logSummary();
    return 0;
}

void CasdaUploadApp::generateMetadataFile(
    const fs::path& file,
    const IdentityElement& identity,
    const ObservationElement& obs,
    const std::vector<ImageElement>& images,
    const std::vector<CatalogElement>& catalogs,
    const std::vector<MeasurementSetElement>& ms,
    const std::vector<EvaluationReportElement>& reports)
{
    xercesc::XMLPlatformUtils::Initialize();

    boost::scoped_ptr<LocalFileFormatTarget> target(new LocalFileFormatTarget(
                XercescString(file.string())));

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

    // Create artifact elements
    appendElementCollection<ImageElement>(images, "images", root);
    appendElementCollection<CatalogElement>(catalogs, "catalogs", root);
    appendElementCollection<MeasurementSetElement>(ms, "measurement_sets", root);
    appendElementCollection<EvaluationReportElement>(reports, "evaluation", root);

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

template <typename T>
std::vector<T> CasdaUploadApp::buildArtifactElements(const std::string& key, bool hasProject) const
{
    vector<T> elements;

    if (config().isDefined(key)) {
        const vector<string> names = config().getStringVector(key);
        for (vector<string>::const_iterator it = names.begin(); it != names.end(); ++it) {
            const LOFAR::ParameterSet subset = config().makeSubset(*it + ".");
            const string filename = subset.getString("filename");

            if (hasProject && !subset.isDefined("project")) {
                ASKAPTHROW(AskapError, "Project is not defined for artifact: " << *it);
            }
            const string project = subset.getString("project", "");
            elements.push_back(T(filename, project));
        }
    }

    return elements;
}

template <typename T>
void CasdaUploadApp::appendElementCollection(const std::vector<T>& elements,
        const std::string& tag,
        xercesc::DOMElement* root)
{
    // Create measurement set elements
    if (!elements.empty()) {
        DOMDocument* doc = root->getOwnerDocument();
        DOMElement* child = doc->createElement(XercescString(tag));
        for (typename vector<T>::const_iterator it = elements.begin();
                it != elements.end(); ++it) {
            child->appendChild(it->toXmlElement(*doc));
        }
        root->appendChild(child);
    }
}

template <typename T>
void CasdaUploadApp::copyAndChecksumElements(const std::vector<T>& elements,
        const fs::path& outdir)
{
    for (typename vector<T>::const_iterator it = elements.begin();
            it != elements.end(); ++it) {
        const fs::path in(it->getFilepath());
        const fs::path out(outdir / in.filename());
        ASKAPLOG_INFO_STR(logger, "Copying and calculating checksum for " << in);
        CasdaFileUtils::copyAndChecksum(in, out);
    }
}
