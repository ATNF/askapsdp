/// @file ElevationStrategy.cc
///
/// @copyright (c) 2013 CSIRO
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
#include "ElevationStrategy.h"

// Include package level header file
#include "askap_pipelinetasks.h"

// System includes
#include <limits>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "Common/ParameterSet.h"
#include "casa/aipstype.h"
#include "measures/Measures/MDirection.h"
#include "ms/MeasurementSets/MeasurementSet.h"
#include "ms/MeasurementSets/MSColumns.h"
#include "ms/MeasurementSets/MSDerivedValues.h"
#include "casa/Arrays/ArrayMath.h" // for near()

// Local package includes
#include "cflag/FlaggingStats.h"

ASKAP_LOGGER(logger, ".ElevationStrategy");

using namespace askap;
using namespace casa;
using namespace askap::cp::pipelinetasks;

ElevationStrategy:: ElevationStrategy(const LOFAR::ParameterSet& parset,
                                      const casa::MeasurementSet& /*ms*/)
        : itsStats("ElevationStrategy"),
        itsHighLimit(parset.getFloat("high", 90.0), "deg"),
        itsLowLimit(parset.getFloat("low", 0.0), "deg"),
        itsTimeElevCalculated(0.0)
{
}

FlaggingStats ElevationStrategy::stats(void) const
{
    return itsStats;
}

void ElevationStrategy::updateElevations(casa::MSColumns& msc,
                                         const casa::uInt row)
{
    // 1: Ensure the antenna elevation array is the correct size
    const casa::uInt nAnt= msc.antenna().nrow();
    if (itsAntennaElevations.size() != nAnt) {
        itsAntennaElevations.resize(nAnt);
    }

    // 2: Setup MSDerivedValues with antenna positions, field direction, and date/time
    MSDerivedValues msd;
    msd.setAntennas(msc.antenna());
    msd.setEpoch(msc.timeMeas()(row));

    const casa::ROMSFieldColumns& fieldc = msc.field();
    const casa::Int fieldId = msc.fieldId()(row);
    const casa::Vector<casa::MDirection> dirVec = fieldc.phaseDirMeasCol()(fieldId);
    const casa::MDirection direction = dirVec(0);
    msd.setFieldCenter(direction);

    // 3: Calculate elevations for all antennas. Calculate each antenna
    // individually in case very long baselines exist.
    for (casa::uInt i = 0 ; i < nAnt; ++i) {
        msd.setAntenna(i);
        const Vector<double> azel = msd.azel().getAngle("deg").getValue("deg");
        itsAntennaElevations(i) = Quantity(azel(1), "deg");
    }

    itsTimeElevCalculated = msc.time()(row);
}

void ElevationStrategy::processRow(casa::MSColumns& msc, const casa::uInt row,
                                   const bool dryRun)
{
    // 1: If new timestamp then update the antenna elevations
    const casa::Double epsilon = std::numeric_limits<casa::Double>::epsilon();
    if (!casa::near(msc.time()(row), itsTimeElevCalculated, epsilon)) {
        updateElevations(msc, row);
    }

    // 2: Do flagging
    const int ant1 = msc.antenna1()(row);
    const int ant2 = msc.antenna2()(row);
    if (itsAntennaElevations(ant1) < itsLowLimit ||
            itsAntennaElevations(ant2) < itsLowLimit ||
            itsAntennaElevations(ant1) > itsHighLimit ||
            itsAntennaElevations(ant2) > itsHighLimit)
    {
        flagRow(msc, row, dryRun);
    }
}

void ElevationStrategy::flagRow(casa::MSColumns& msc, const casa::uInt row, const bool dryRun)
{
    Matrix<casa::Bool> flags = msc.flag()(row);
    flags = true;

    itsStats.visflagged += flags.size();
    itsStats.rowsflagged++;

    if (!dryRun) {
        msc.flagRow().put(row, true);
        msc.flag().put(row, flags);
    }
}
