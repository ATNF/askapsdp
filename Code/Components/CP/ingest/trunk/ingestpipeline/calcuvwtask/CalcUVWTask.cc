/// @file CalcUVWTask.cc
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
#include "CalcUVWTask.h"

// Include package level header file
#include "askap_cpingest.h"

// System includes
#include <string>
#include <vector>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "casa/Arrays/Matrix.h"
#include "casa/Arrays/MatrixMath.h"
#include "measures/Measures.h"
#include "measures/Measures/MeasConvert.h"
#include "measures/Measures/MCEpoch.h"
#include "measures/Measures/MDirection.h"
#include "measures/Measures/MEpoch.h"
#include "casa/Quanta/MVAngle.h"

// Local package includes
#include "ingestpipeline/datadef/VisChunk.h"

ASKAP_LOGGER(logger, ".CalcUVWTask");

using namespace casa;
using namespace askap;
using namespace askap::cp;

CalcUVWTask::CalcUVWTask(const LOFAR::ParameterSet& parset) :
    itsParset(parset)
{
    ASKAPLOG_DEBUG_STR(logger, "Constructor");
    const LOFAR::ParameterSet antSubset(itsParset.makeSubset("uvw.antennas."));
    itsAntennaPositions.reset(new AntennaPositions(antSubset));
}

CalcUVWTask::~CalcUVWTask()
{
    ASKAPLOG_DEBUG_STR(logger, "Destructor");
}

void CalcUVWTask::process(VisChunk::ShPtr chunk)
{
    ASKAPLOG_DEBUG_STR(logger, "process()");

    for (casa::uInt row = 0; row < chunk->nRow(); ++row) {
        calcForRow(chunk, row);
    }
}

void CalcUVWTask::calcForRow(VisChunk::ShPtr chunk, const casa::uInt row)
{
    const casa::uInt ant1 = chunk->antenna1()(row);
    const casa::uInt ant2 = chunk->antenna2()(row);

    // The antenna positions. Size is 3 (x, y & z) rows by nAntenna columns.
    // Rows are x, y, z and columns are indexed by antenna id.
    casa::Matrix<double> antXYZ = itsAntennaPositions->getPositionMatrix();
    const casa::uInt nAnt = antXYZ.ncolumn();

    ASKAPCHECK(ant1 < nAnt, "Antenna index (" << ant1 << ") is invalid");
    ASKAPCHECK(ant2 < nAnt, "Antenna index (" << ant2 << ") is invalid");

    // Determine Greenwich Mean Sidereal Time
    MEpoch epUT1(chunk->time(), MEpoch::UTC);
    MEpoch::Ref refGMST1(MEpoch::GMST1);
    MEpoch::Convert epGMST1(epUT1, refGMST1);
    double gmst = epGMST1().get("d").getValue("d");
    gmst = (gmst - Int(gmst)) * C::_2pi; // Into Radians

    // Current phase center
    const casa::MDirection fpc = chunk->pointingDir1()(row);
    const double ra = fpc.getAngle().getValue()(0);
    const double dec = fpc.getAngle().getValue()(1);

    // Transformation from antenna position difference (ant2-ant1) to uvw
    const double H0 = gmst - ra;
    const double sH0 = sin(H0);
    const double cH0 = cos(H0);
    const double sd = sin(dec);
    const double cd = cos(dec);
    Matrix<double> trans(3, 3, 0);
    trans(0, 0) = -sH0; trans(0, 1) = -cH0;
    trans(1, 0) = sd * cH0; trans(1, 1) = -sd * sH0; trans(1, 2) = -cd;
    trans(2, 0) = -cd * cH0; trans(2, 1) = cd * sH0; trans(2, 2) = -sd;

    // Rotate antennas to correct frame
    Matrix<double> antUVW(3, nAnt);

    for (uInt i = 0; i < nAnt; ++i) {
        antUVW.column(i) = casa::product(trans, antXYZ.column(i));
    }

    double x1 = antUVW(0, ant1), y1 = antUVW(1, ant1), z1 = antUVW(2, ant1);
    double x2 = antUVW(0, ant2), y2 = antUVW(1, ant2), z2 = antUVW(2, ant2);
    Vector<double> uvwvec(3);
    uvwvec(0) = x2 - x1;
    uvwvec(1) = y2 - y1;
    uvwvec(2) = z2 - z1;

    // Finally set the uvwvec in the VisChunk
    chunk->uvw()(row) = uvwvec;
}
