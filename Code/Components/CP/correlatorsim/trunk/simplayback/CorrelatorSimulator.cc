/// @file CorrelatorSimulator.cc
///
/// @copyright (c) 2010 CSIRO
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
#include "CorrelatorSimulator.h"

// Include package level header file
#include "askap_correlatorsim.h"

// System includes
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include <cmath>

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "askap/AskapLogging.h"
#include "ms/MeasurementSets/MeasurementSet.h"
#include "ms/MeasurementSets/MSColumns.h"
#include "casa/Arrays/Matrix.h"
#include "measures/Measures/MDirection.h"
#include "cpcommon/VisDatagram.h"

// Using
using namespace askap;
using namespace askap::cp;
using namespace casa;

ASKAP_LOGGER(logger, ".CorrelatorSimulator");

CorrelatorSimulator::CorrelatorSimulator(const std::string& dataset,
        const std::string& hostname, const std::string& port)
: itsCurrentRow(0)
{
    itsMS.reset(new casa::MeasurementSet(dataset, casa::Table::Old));
    itsPort.reset(new askap::cp::VisPort(hostname, port));
}

CorrelatorSimulator::~CorrelatorSimulator()
{
    itsMS.reset();
    itsPort.reset();
}

bool CorrelatorSimulator::sendNext(void)
{
    ROMSColumns msc(*itsMS);

    // Get a reference to the columns of interest
    //const casa::ROMSAntennaColumns& antc = msc.antenna();
    //const casa::ROMSFeedColumns& feedc = msc.feed();
    const casa::ROMSFieldColumns& fieldc = msc.field();
    const casa::ROMSSpWindowColumns& spwc = msc.spectralWindow();
    const casa::ROMSDataDescColumns& ddc = msc.dataDescription();
    //const casa::ROMSPolarizationColumns& polc = msc.polarization();
    //const casa::ROMSPointingColumns& pointingc = msc.pointing();

    // Define some useful variables
    const int dataDescId = msc.dataDescId()(itsCurrentRow);
    //const unsigned int descPolId = ddc.polarizationId()(dataDescId);
    const unsigned int descSpwId = ddc.spectralWindowId()(dataDescId);
    const unsigned int nRow = msc.nrow(); // In the whole table, not just for this integration
    //const unsigned int nCorr = polc.numCorr()(descPolId);
    const unsigned int nChan = spwc.numChan()(descSpwId);
    //const unsigned int nAntenna = antc.nrow();
    //const unsigned int nBeam = feedc.nrow() / nAntenna;
    //const unsigned int nCoarseChan = 1;

    // Record the timestamp for the current integration that is
    // being processed
    casa::Double currentIntegration = msc.time()(itsCurrentRow);
    ASKAPLOG_DEBUG_STR(logger, "Processing integration with timestamp "
            << std::setprecision (13) << currentIntegration);

    // Some general constraints
    ASKAPCHECK(fieldc.nrow() == 1, "Currently only support a single field");

    ////////////////////////////////////////
    // Visibilities
    ////////////////////////////////////////

    // Process rows until none are left or the timestamp
    // changes, indicating the end of this integration
    while (itsCurrentRow != nRow && (currentIntegration == msc.time()(itsCurrentRow))) {

        // Some per row constraints
        // This code needs the dataDescId to remain constant for all rows
        // in the integration being processed
        ASKAPCHECK(msc.dataDescId()(itsCurrentRow) == dataDescId,
                "Data description ID must remain constant for a given integration");

        // Populate the VisDatagram
        askap::cp::VisDatagram payload;
        payload.version = VISPAYLOAD_VERSION;
        const long timestamp =
            static_cast<long>(msc.time()(itsCurrentRow) * 1000 * 1000);
        payload.timestamp = timestamp;
        payload.antenna1 = msc.antenna1()(itsCurrentRow);
        payload.antenna2 = msc.antenna2()(itsCurrentRow);
        payload.beam1 = msc.feed1()(itsCurrentRow);
        payload.beam2 = msc.feed2()(itsCurrentRow);

        // Set all nSamples to 1 and ensure nominalNSamples is also 1
        for (unsigned int i = 0; i < N_FINE_PER_COARSE * N_POL; ++i) {
            payload.nSamples[i] = 1;
        }

        // This matrix is: Matrix<Complex> data(nCorr, nChan)
        const casa::Matrix<casa::Complex> data = msc.data()(itsCurrentRow);
        for (unsigned int coarseChan = 0; coarseChan < nChan; ++coarseChan) {
            payload.coarseChannel = coarseChan;
            for (unsigned int fineChan = 0; fineChan < N_FINE_PER_COARSE; ++fineChan) {
                for (unsigned int pol = 0; pol < N_POL; ++pol) {
                    const int idx = pol + (N_POL * fineChan);
                    payload.vis[idx].real = data(pol, coarseChan).real();
                    payload.vis[idx].imag = data(pol, coarseChan).imag();
                }
            }
            // Finished populating, send this payload but then reuse it in the
            // next iteration of the loop for the next coarse channel
            itsPort->send(payload);
        }

        itsCurrentRow++;
    }

    if (itsCurrentRow == nRow) {
        return false; // Indicate there is no more data after this payload
    } else {
        return true;
    }
}
