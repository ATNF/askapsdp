/// @file AmplitudeFlagger.cc
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
#include "AmplitudeFlagger.h"

// Include package level header file
#include "askap_pipelinetasks.h"

// System includes
#include <limits>
#include <set>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "Common/ParameterSet.h"
#include "casa/aipstype.h"
#include "measures/Measures/MDirection.h"
#include "ms/MeasurementSets/MeasurementSet.h"
#include "ms/MeasurementSets/MSColumns.h"
#include "measures/Measures/Stokes.h"
#include "casa/Arrays/Matrix.h"
#include "casa/Arrays/Vector.h"

// Local package includes
#include "cflag/FlaggingStats.h"

ASKAP_LOGGER(logger, ".AmplitudeFlagger");

using namespace askap;
using namespace casa;
using namespace askap::cp::pipelinetasks;

AmplitudeFlagger:: AmplitudeFlagger(const LOFAR::ParameterSet& parset,
                                    const casa::MeasurementSet& /*ms*/)
        : itsStats("AmplitudeFlagger")
{
    if (parset.isDefined("high")) {
        itsHasHighLimit = true;
        itsHighLimit = parset.getFloat("high");
    }

    if (parset.isDefined("low")) {
        itsHasLowLimit = true;
        itsLowLimit = parset.getFloat("low");
    }

    // Converts Stokes vector string to StokesType
    if (parset.isDefined("stokes")) {
        vector<string> strvec = parset.getStringVector("stokes");

        for (size_t i = 0; i < strvec.size(); ++i) {
            itsStokes.insert(Stokes::type(strvec[i]));
        }
    }
}

FlaggingStats AmplitudeFlagger::stats(void) const
{
    return itsStats;
}

casa::Vector<casa::Int> AmplitudeFlagger::getStokesType(
        casa::MSColumns& msc, const casa::uInt row)
{
    const casa::ROMSDataDescColumns& ddc = msc.dataDescription();
    const int dataDescId = msc.dataDescId()(row);
    const casa::ROMSPolarizationColumns& polc = msc.polarization();
    const unsigned int descPolId = ddc.polarizationId()(dataDescId);
    return polc.corrType()(descPolId);
}

void AmplitudeFlagger::processRow(casa::MSColumns& msc, const casa::uInt row,
                                  const bool dryRun)
{
    const Matrix<casa::Complex> data = msc.data()(row);
    Matrix<casa::Bool> flags = msc.flag()(row);

    // Only need to write out the flag matrix if it was updated
    bool wasUpdated = false;

    const casa::Vector<casa::Int> stokesTypesInt = getStokesType(msc, row);

    // Iterate over rows (one row is one correlation product)
    for (size_t corr = 0; corr < data.nrow(); ++corr) {
        // If this row doesn't contain a product we are meant to be flagging,
        // then ignore it
        if (!itsStokes.empty() && (itsStokes.find(Stokes::type(stokesTypesInt(corr))) == itsStokes.end())) {
            continue;
        }

        for (size_t chan = 0; chan < data.ncolumn(); ++chan) {
            if (flags(corr, chan)) continue;

            const float amp = abs(data(corr, chan));

            if ((itsHasLowLimit && (amp < itsLowLimit))
                    || (itsHasHighLimit && (amp > itsHighLimit))) {
                flags(corr, chan) = true;
                wasUpdated = true;
                itsStats.visflagged++;
            }
        }
    }

    if (wasUpdated && !dryRun) {
        msc.flag().put(row, flags);
    }
}
