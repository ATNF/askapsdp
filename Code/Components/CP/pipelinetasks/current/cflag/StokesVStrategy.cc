/// @file StokesVStrategy.cc
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
#include "StokesVStrategy.h"

// Include package level header file
#include "askap_pipelinetasks.h"

// System includes
#include <map>
#include <limits>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "Common/ParameterSet.h"
#include "casa/aipstype.h"
#include "casa/Arrays/ArrayMath.h"
#include "casa/Arrays/Vector.h"
#include "casa/Arrays/Matrix.h"
#include "measures/Measures/Stokes.h"
#include "ms/MeasurementSets/MeasurementSet.h"
#include "ms/MeasurementSets/MSColumns.h"
#include "ms/MeasurementSets/MSPolColumns.h"
#include "ms/MeasurementSets/StokesConverter.h"

// Local package includes
#include "cflag/FlaggingStats.h"

ASKAP_LOGGER(logger, ".StokesVStrategy");

using namespace std;
using namespace askap;
using namespace casa;
using namespace askap::cp::pipelinetasks;

StokesVStrategy:: StokesVStrategy(const LOFAR::ParameterSet& parset,
                                  const casa::MeasurementSet& /*ms*/)
        : itsStats("StokesVStrategy")
{
    itsThreshold = parset.getFloat("threshold", 5.0);
    ASKAPCHECK(itsThreshold > 0.0, "Threshold must be greater than zero");
}

FlaggingStats StokesVStrategy::stats(void) const
{
    return itsStats;
}

casa::StokesConverter& StokesVStrategy::getStokesConverter(
    const casa::ROMSPolarizationColumns& polc, const casa::Int polId)
{
    const casa::Vector<Int> corrType = polc.corrType()(polId);
    std::map<casa::Int, casa::StokesConverter>::iterator it = itsConverterCache.find(polId);
    if (it == itsConverterCache.end()) {
        ASKAPLOG_DEBUG_STR(logger, "Creating StokesConverter for pol table entry " << polId);
        const casa::Vector<Int> target(1, Stokes::V);
        itsConverterCache.insert(pair<casa::Int, casa::StokesConverter>(polId,
                                 casa::StokesConverter(target, corrType)));
    }

    return itsConverterCache[polId];
}

void StokesVStrategy::processRow(casa::MSColumns& msc, const casa::uInt row,
                                 const bool dryRun)
{
    // Get a description of what correlation products are in the data table.
    const casa::ROMSDataDescColumns& ddc = msc.dataDescription();
    const casa::Int dataDescId = msc.dataDescId()(row);
    const casa::Int polId = ddc.polarizationId()(dataDescId);

    // Get the (potentially cached) stokes converter
    const StokesConverter& stokesconv = getStokesConverter(msc.polarization(), polId);

    // Convert data to Stokes V
    const Matrix<casa::Complex> data = msc.data()(row);
    casa::Matrix<casa::Complex> vmatrix(1, data.ncolumn());
    stokesconv.convert(vmatrix, data);
    casa::Vector<casa::Complex> vdata = vmatrix.row(0);

    // Build a vector with the amplitudes
    Matrix<casa::Bool> flags = msc.flag()(row);
    std::vector<casa::Float> tmpamps;
    for (size_t i = 0; i < vdata.size(); ++i) {
        bool anyFlagged = anyEQ(flags.column(i), true);
        if (!anyFlagged) {
            tmpamps.push_back(abs(vdata(i)));
        }
    }

    // Convert to a casa::Vector so we can use ArrayMath functions
    // to determine the mean and stddev
    casa::Vector<casa::Float> amps(tmpamps);

    // Flag all correlations where the Stokes V product
    // is greater than the threshold
    const casa::Float sigma = stddev(amps);
    const casa::Float avg = mean(amps);

    // If stokes-v can't be formed due to lack of the necessary input products
    // then vdata will contain all zeros. In this case, no flagging can be done.
    const casa::Float epsilon = std::numeric_limits<casa::Float>::epsilon();
    if (near(sigma, 0.0, epsilon) && near(avg, 0.0, epsilon)) {
        return;
    }

    // Apply threshold based flagging
    bool wasUpdated = false;
    for (size_t i = 0; i < amps.size(); ++i) {
        if (abs(vdata(i)) > (avg + (sigma * itsThreshold))) {
            for (casa::uInt pol = 0; pol < flags.nrow(); ++pol) {
                flags(pol, i) = true;
                wasUpdated = true;
            }
            itsStats.visflagged += flags.nrow();
        }
    }

    if (wasUpdated && !dryRun) {
        msc.flag().put(row, flags);
    }
}
