/// @file CModelMaster.cc
///
/// @copyright (c) 2011 CSIRO
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
#include "cmodel/CModelMaster.h"

// Include package level header file
#include "askap_pipelinetasks.h"

// System includes
#include <cmath>
#include <string>
#include <vector>
#include <iterator>
#include <algorithm>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "askap/AskapUtil.h"
#include "boost/scoped_ptr.hpp"
#include "Common/ParameterSet.h"
#include "skymodelclient/Component.h"

// Casacore includes
#include "casa/aipstype.h"
#include "casa/Quanta/Quantum.h"
#include "images/Images/PagedImage.h"

// Local package includes
#include "cmodel/DuchampAccessor.h"
#include "cmodel/DataserviceAccessor.h"
#include "cmodel/MPIBasicComms.h"
#include "cmodel/ImageFactory.h"

// Using
using namespace askap;
using namespace askap::cp::pipelinetasks;
using namespace casa;

ASKAP_LOGGER(logger, ".CModelMaster");

CModelMaster::CModelMaster(const LOFAR::ParameterSet& itsParset, MPIBasicComms& comms)
        : itsParset(itsParset), itsComms(comms)
{
}

void CModelMaster::run(void)
{
    ASKAPLOG_INFO_STR(logger, "Running master");
    // Broadcast the parset
    LOFAR::ParameterSet parset = itsParset; // Avoid const_cast
    itsComms.broadcastParset(parset, 0);

    // Interface to GSM data
    boost::scoped_ptr<IGlobalSkyModel> gsm;

    // Setup the GSM accessor
    const std::string database = itsParset.getString("gsm.database");

    if (database == "duchamp") {
        const std::string filename = itsParset.getString("gsm.file");
        gsm.reset(new DuchampAccessor(filename));
    } else if (database == "dataservice") {
        const std::string host = itsParset.getString("gsm.locator_host");
        const std::string port = itsParset.getString("gsm.locator_port");
        const std::string serviceName = itsParset.getString("gsm.service_name");
        gsm.reset(new DataserviceAccessor(host, port, serviceName));
    } else {
        ASKAPTHROW(AskapError, "Unknown GSM database type: " << database);
    }

    // Get the flux limit
    const std::string fluxLimitStr = itsParset.getString("flux_limit");
    const Quantity fluxLimit = asQuantity(fluxLimitStr, "Jy");

    // Get the centre of the image
    const std::vector<std::string> dirVector = itsParset.getStringVector("direction");
    const Quantity ra = asQuantity(dirVector.at(0), "deg");
    const Quantity dec = asQuantity(dirVector.at(1), "deg");

    // Detrmine the search radius
    // At the moment just use the 1D size of the image multiplied by the
    // cellsize to determine the search radius. Because the dimensions or scale
    // may not be identical, use the larger of the two. This is almost 2x the
    // field, but given the current implementations of cone search do not
    // include extended components with centre outside the field, it is best
    // to search a larger radius anyway.
    const casa::uInt nx = itsParset.getUintVector("shape").at(0);
    const casa::uInt ny = itsParset.getUintVector("shape").at(1);
    const std::vector<std::string> cellSizeVector = itsParset.getStringVector("cellsize");
    const Quantity xcellsize = asQuantity(cellSizeVector.at(0), "arcsec");
    const Quantity ycellsize = asQuantity(cellSizeVector.at(1), "arcsec");
    const casa::Quantity searchRadius(
            std::max(xcellsize.getValue("deg") * nx, ycellsize.getValue("deg") * ny),
            "deg");

    const std::vector<askap::cp::skymodelservice::Component> list = gsm->coneSearch(ra, dec, searchRadius, fluxLimit);
    gsm.reset(0);
    ASKAPLOG_INFO_STR(logger, "Number of components in result set: " << list.size());

    const casa::uInt batchSize = itsParset.getUint("batchsize", 200);
    const unsigned int nterms = itsParset.getUint("nterms", 1);

    // Send components to each worker until complete
    for (unsigned int term = 0; term < nterms; ++term) {
        if (nterms > 1) {
            ASKAPLOG_INFO_STR(logger, "Imaging taylor term " << term);
        }
        size_t idx = 0;
        std::vector<askap::cp::skymodelservice::Component> subset;
        while (idx < list.size()) {
            // Get a batch ready - an "nelement" subset of "list" will be sent
            const size_t nelements = (idx + batchSize < list.size()) ? batchSize : (list.size() - idx);
            subset.assign(list.begin() + idx, list.begin() + idx + nelements);
            idx += nelements;

            // Wait for a worker to become available
            const int worker = itsComms.getReadyWorkerId();
            ASKAPLOG_DEBUG_STR(logger, "Allocating " << subset.size()
                    << " components to worker " << worker);
            itsComms.sendComponents(subset, worker);
            ASKAPLOG_INFO_STR(logger, "Master has allocated " << idx << " of "
                    << list.size() << " components");
        }

        // Send each worker an empty list to signal completion, need to first consume
        // the ready signals so the workers will unblock.
        subset.clear();
        for (int i = 1; i < itsComms.getNumNodes(); ++i) {
            const int worker = itsComms.getReadyWorkerId();
            itsComms.sendComponents(subset, worker);
        }

        // Create an image and sum all workers images to the master
        std::string filename = parset.getString("filename");
        if (nterms > 1) {
            filename += ".";
            filename += askap::utility::toString(term);
        }

        casa::PagedImage<casa::Float> image = ImageFactory::createPagedImage(parset, filename);
        ASKAPLOG_INFO_STR(logger, "Beginning reduction step");
        itsComms.sumImages(image, 0);
        ASKAPLOG_INFO_STR(logger, "Completed reduction step");
    }
}
