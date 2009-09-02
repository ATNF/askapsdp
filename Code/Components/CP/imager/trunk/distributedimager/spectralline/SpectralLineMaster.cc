/// @file SpectralLineMaster.cc
///
/// @copyright (c) 2009 CSIRO
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
#include "SpectralLineMaster.h"

// System includes
#include <string>
#include <sstream>
#include <stdexcept>

// ASKAPsoft includes
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <Common/ParameterSet.h>
#include <dataaccess/IConstDataSource.h>
#include <dataaccess/TableConstDataSource.h>
#include <dataaccess/IConstDataIterator.h>
#include <dataaccess/IDataConverter.h>
#include <dataaccess/IDataSelector.h>
#include <dataaccess/IDataIterator.h>
#include <dataaccess/SharedIter.h>

// Local includes
#include "distributedimager/common/IBasicComms.h"
#include "messages/SpectralLineWorkUnit.h"
#include "messages/SpectralLineWorkRequest.h"

using namespace askap::cp;
using namespace askap;

ASKAP_LOGGER(logger, ".SpectralLineMaster");

SpectralLineMaster::SpectralLineMaster(LOFAR::ParameterSet& parset,
        askap::cp::IBasicComms& comms)
: itsParset(parset), itsComms(comms)
{
}

SpectralLineMaster::~SpectralLineMaster()
{
}

void SpectralLineMaster::run(void)
{
    // Read from the configruation the list of datasets to process
    const std::vector<std::string> ms = getDatasets(itsParset);
    if (ms.size() == 0) {
        ASKAPTHROW (std::runtime_error, "No datasets specified in the parameter set file");
    }

    // Get the base image name
    if (!itsParset.isDefined("Images.name")) {
        ASKAPTHROW(std::runtime_error, "Image name is not defined in parameter set");
    }
    const std::string imageName = itsParset.getString("Images.name");

    // Send work orders to the worker processes, handling out
    // more work to the workers as needed.
    int channelOffset = 0;
    for (unsigned int n = 0; n < ms.size(); ++n) {
        int id; // Id of the process the WorkRequest message is received from

        // Wait for a worker to request some work
        SpectralLineWorkRequest wrequest;
        itsComms.receiveMessageAnySrc(wrequest, id);

        const int msChannels = getNumChannels(ms[n]);

        // Send the workunit to the worker
        ASKAPLOG_INFO_STR(logger, "Master is allocating workunit " << ms[n]
                << ", containing channels " << (channelOffset + 1) << "-"
                << (channelOffset + msChannels) << " to worker " << id);
        SpectralLineWorkUnit wu;
        wu.set_payloadType(SpectralLineWorkUnit::WORK);
        wu.set_dataset(ms[n]);
        wu.set_imagename(imageName);
        wu.set_channelOffset(channelOffset);
        itsComms.sendMessage(wu, id);

        channelOffset += msChannels;
    }

    // Send each worker a response to indicate there are
    // no more work units
    for (int id = 1; id < itsComms.getNumNodes(); ++id) {
        SpectralLineWorkUnit wu;
        wu.set_payloadType(SpectralLineWorkUnit::DONE);
        itsComms.sendMessage(wu, id);
    }
}


// Utility function to get dataset names from parset.
std::vector<std::string> SpectralLineMaster::getDatasets(const LOFAR::ParameterSet& parset)
{
    if (parset.isDefined("dataset") && parset.isDefined("dataset0")) {
        ASKAPTHROW(std::runtime_error, "Both dataset and dataset0 are specified in the parset");
    }

    // First look for "dataset" and if that does not exist try "dataset0"
    std::vector<std::string> ms;
    if (parset.isDefined("dataset")) {
        ms = itsParset.getStringVector("dataset");
    } else {
        std::string key = "dataset0";   // First key to look for
        long idx = 0;
        while (parset.isDefined(key)) {
            const std::string value = parset.getString(key);
            ms.push_back(value);

            std::ostringstream ss;
            ss << "dataset" << idx+1;
            key = ss.str();
            ++idx;
        }
    }

    return ms;
}

// NOTE: This function makes the assumption that each iteration will have
// the same number of channels. This may not be true, but reading through the
// entire dataset to validate this assumption is going to be too slow.
int SpectralLineMaster::getNumChannels(const std::string& ms)
{
    askap::synthesis::TableConstDataSource ds(ms);

    askap::synthesis::IDataSelectorPtr sel = ds.createSelector();
    askap::synthesis::IDataConverterPtr conv = ds.createConverter();
    conv->setFrequencyFrame(casa::MFrequency::Ref(casa::MFrequency::TOPO), "Hz");
    conv->setDirectionFrame(casa::MDirection::Ref(casa::MDirection::J2000));

    const askap::synthesis::IConstDataSharedIter it = ds.createConstIterator(sel,conv);
    return it->nChannel();
}
