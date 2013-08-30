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
#include <vector>

// ASKAPsoft includes
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <Common/ParameterSet.h>
#include <fitting/Params.h>
#include <dataaccess/IConstDataSource.h>
#include <dataaccess/TableConstDataSource.h>
#include <dataaccess/IConstDataIterator.h>
#include <dataaccess/IDataConverter.h>
#include <dataaccess/IDataSelector.h>
#include <dataaccess/IDataIterator.h>
#include <dataaccess/SharedIter.h>
#include <casa/Quanta.h>

// Local includes
#include "distributedimager/IBasicComms.h"
#include "messages/SpectralLineWorkUnit.h"
#include "messages/SpectralLineWorkRequest.h"

using namespace std;
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
    const vector<string> ms = getDatasets(itsParset);
    if (ms.size() == 0) {
        ASKAPTHROW (std::runtime_error, "No datasets specified in the parameter set file");
    }

    // Get info from each measurement set so we know how many channels, what channels, etc.
    const vector<MSInfo> infovec = getMSInfo(ms);
    ASKAPCHECK(!infovec.empty(), "MeasurementSet info is empty");
    const casa::uInt nChan = getNumChannels(infovec);

    // Create an image cube builder
    itsImageCube.reset(new CubeBuilder(itsParset, nChan, getFirstFreq(infovec), getFreqInc(infovec)));
    itsPSFCube.reset(new CubeBuilder(itsParset, nChan, getFirstFreq(infovec), getFreqInc(infovec), "psf"));
    itsResidualCube.reset(new CubeBuilder(itsParset, nChan, getFirstFreq(infovec), getFreqInc(infovec), "residual"));
    itsWeightsCube.reset(new CubeBuilder(itsParset, nChan, getFirstFreq(infovec), getFreqInc(infovec), "weights"));

    // Send work orders to the worker processes, handling out
    // more work to the workers as needed.

    unsigned int globalChannel = 0;

    // Tracks all outstanding workunits, that is, those that have not
    // been completed
    unsigned int outstanding = 0;

    // Iterate over all measurement sets
    for (unsigned int n = 0; n < ms.size(); ++n) {
        const unsigned int msChannels = infovec[n].nChan;
        ASKAPLOG_DEBUG_STR(logger, "Creating work orders for measurement set "
                << ms[n] << " with " << msChannels << " channels");

        // Iterate over all channels in the measurement set
        for (unsigned int localChan = 0; localChan < msChannels; ++localChan) {

            int id; // Id of the process the WorkRequest message is received from

            // Wait for a worker to request some work
            SpectralLineWorkRequest wrequest;
            itsComms.receiveMessageAnySrc(wrequest, id);
            if (wrequest.get_params().get() != 0) {
                handleImageParams(wrequest.get_params(), wrequest.get_globalChannel());
                --outstanding;
            }

            // Send the workunit to the worker
            ASKAPLOG_DEBUG_STR(logger, "Master is allocating workunit " << ms[n]
                    << ", local channel " <<  localChan << ", global channel "
                    << globalChannel << " to worker " << id);
            SpectralLineWorkUnit wu;
            wu.set_payloadType(SpectralLineWorkUnit::WORK);
            wu.set_dataset(ms[n]);
            wu.set_globalChannel(globalChannel);
            wu.set_localChannel(localChan);
            itsComms.sendMessage(wu, id);
            outstanding++;

            globalChannel++;
        }
    }

    // Wait for all outstanding workunits to complete
    while (outstanding > 0) {
        int id;
        SpectralLineWorkRequest wrequest;
        itsComms.receiveMessageAnySrc(wrequest, id);
        if (wrequest.get_params().get() != 0) {
            handleImageParams(wrequest.get_params(), wrequest.get_globalChannel());
            --outstanding;
        }
    }

    // Send each worker a response to indicate there are
    // no more work units. This is done separate to the above loop
    // since we need to make sure even workers that never received
    // a workunit are send the "DONE" message.
    for (int id = 1; id < itsComms.getNumNodes(); ++id) {
        SpectralLineWorkUnit wu;
        wu.set_payloadType(SpectralLineWorkUnit::DONE);
        itsComms.sendMessage(wu, id);
    }

    itsImageCube.reset();
}

// Utility function to get dataset names from parset.
std::vector<std::string> SpectralLineMaster::getDatasets(const LOFAR::ParameterSet& parset)
{
    if (parset.isDefined("dataset") && parset.isDefined("dataset0")) {
        ASKAPTHROW(std::runtime_error, "Both dataset and dataset0 are specified in the parset");
    }

    // First look for "dataset" and if that does not exist try "dataset0"
    vector<string> ms;
    if (parset.isDefined("dataset")) {
        ms = itsParset.getStringVector("dataset");
    } else {
        string key = "dataset0";   // First key to look for
        long idx = 0;
        while (parset.isDefined(key)) {
            const string value = parset.getString(key);
            ms.push_back(value);

            ostringstream ss;
            ss << "dataset" << idx+1;
            key = ss.str();
            ++idx;
        }
    }

    return ms;
}

void SpectralLineMaster::handleImageParams(askap::scimath::Params::ShPtr params, unsigned int chan)
{
    const vector<string> images = params->names();
    for (size_t i = 0; i < images.size(); ++i) {
        ASKAPLOG_DEBUG_STR(logger, "Got image: " << images[i]);
    }

    // Write image
    {
        const casa::Array<double> imagePixels(params->value("image.slice"));
        casa::Array<float> floatImagePixels(imagePixels.shape());
        casa::convertArray<float, double>(floatImagePixels, imagePixels);
        itsImageCube->writeSlice(floatImagePixels, chan);
    }

    // Write PSF
    {
        const casa::Array<double> imagePixels(params->value("psf.slice"));
        casa::Array<float> floatImagePixels(imagePixels.shape());
        casa::convertArray<float, double>(floatImagePixels, imagePixels);
        itsPSFCube->writeSlice(floatImagePixels, chan);
    }

    // Write residual
    {
        const casa::Array<double> imagePixels(params->value("residual.slice"));
        casa::Array<float> floatImagePixels(imagePixels.shape());
        casa::convertArray<float, double>(floatImagePixels, imagePixels);
        itsResidualCube->writeSlice(floatImagePixels, chan);
    }

    // Write weights
    {
        const casa::Array<double> imagePixels(params->value("weights.slice"));
        casa::Array<float> floatImagePixels(imagePixels.shape());
        casa::convertArray<float, double>(floatImagePixels, imagePixels);
        itsWeightsCube->writeSlice(floatImagePixels, chan);
    }
}

// NOTE: This function makes the assumption that each iteration will have
// the same number of channels. This may not be true, but reading through the
// entire dataset to validate this assumption is going to be too slow.
SpectralLineMaster::MSInfo SpectralLineMaster::getMSInfo(const std::string& ms)
{
    askap::accessors::TableConstDataSource ds(ms);

    askap::accessors::IDataSelectorPtr sel = ds.createSelector();
    askap::accessors::IDataConverterPtr conv = ds.createConverter();
    conv->setFrequencyFrame(casa::MFrequency::Ref(casa::MFrequency::TOPO), "Hz");
    conv->setDirectionFrame(casa::MDirection::Ref(casa::MDirection::J2000));

    const askap::accessors::IConstDataSharedIter it = ds.createConstIterator(sel,conv);

    MSInfo info;
    info.nChan = it->nChannel();
    info.freqs.resize(info.nChan);
    for (size_t i = 0; i < info.nChan; ++i) {
        info.freqs[i] = casa::Quantity(it->frequency()(i), "Hz");
    }

    return info;
}

std::vector<SpectralLineMaster::MSInfo> SpectralLineMaster::getMSInfo(const std::vector<std::string>& ms)
{
    vector<MSInfo> info(ms.size());
    for (size_t i = 0; i < ms.size(); ++i) {
        info[i] = getMSInfo(ms[i]);
    }
    return info;
}

casa::uInt SpectralLineMaster::getNumChannels(const std::vector<MSInfo>& info)
{
    int nchan = 0;
    for (size_t i = 0; i < info.size(); ++i) {
        nchan += info[i].nChan;
    }
    return nchan;
}

casa::Quantity SpectralLineMaster::getFirstFreq(const std::vector<MSInfo>& info)
{
    ASKAPCHECK(!info[0].freqs.empty(), "First MS contains zero channels");
    return info[0].freqs[0];
}

casa::Quantity SpectralLineMaster::getFreqInc(const std::vector<MSInfo>& info)
{
    const Quantity firstfreq = info.front().freqs.front();
    const Quantity lastfreq = info.back().freqs.back();
    const casa::uInt nChan = getNumChannels(info);
    return(lastfreq - firstfreq) / nChan;
}
