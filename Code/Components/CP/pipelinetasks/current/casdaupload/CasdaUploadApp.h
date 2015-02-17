/// @file CasdaUploadApp.h
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

#ifndef ASKAP_CP_PIPELINETASKS_CASDAUPLOADAPP_H
#define ASKAP_CP_PIPELINETASKS_CASDAUPLOADAPP_H

// System includes
#include <vector>
#include <string>

// ASKAPsoft includes
#include "askap/Application.h"
#include "xercesc/dom/DOM.hpp" // Includes all DOM
#include "boost/filesystem.hpp"

// Local package includes
#include "casdaupload/IdentityElement.h"
#include "casdaupload/ObservationElement.h"
#include "casdaupload/ImageElement.h"
#include "casdaupload/CatalogElement.h"
#include "casdaupload/MeasurementSetElement.h"
#include "casdaupload/EvaluationReportElement.h"

namespace askap {
namespace cp {
namespace pipelinetasks {

/// @brief Implementation of the Cflag application
class CasdaUploadApp : public askap::Application {
    public:
        /// Run the application
        virtual int run(int argc, char* argv[]);

    private:

        /// Created the metadata file
        /// @param[in] filename
        static void generateMetadataFile(const boost::filesystem::path& file,
                                         const IdentityElement& identity,
                                         const ObservationElement& obs,
                                         const std::vector<ImageElement>& images,
                                         const std::vector<CatalogElement>& catalogs,
                                         const std::vector<MeasurementSetElement>& ms,
                                         const std::vector<EvaluationReportElement>& reports);

        static void checksumFile(const boost::filesystem::path& filename);

        static void tarAndChecksum(const boost::filesystem::path& infile,
                                   const boost::filesystem::path& outfile);

        static void copyAndChecksum(const boost::filesystem::path& infile,
                                    const boost::filesystem::path& outdir);

        static void writeReadyFile(const boost::filesystem::path& filename);

        template <typename T>
        static void appendElementCollection(const std::vector<T>& elements,
                                            const std::string& tag,
                                            xercesc::DOMElement* root);

        template <typename T>
        std::vector<T> buildArtifactElements(const std::string& key,
                                             bool hasProject = true) const;

        // Size (in bytes) of the buffer for file IO. This is effectivly the I/O
        // size for copy and checksum operations.
        static const size_t IO_BUFFER_SIZE = 1048576;

        // The filename extension used for checksum files
        static const std::string CHECKSUM_EXT;
};

}
}
}

#endif
