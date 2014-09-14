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
#include "Common/ParameterSet.h"
#include "casa/Arrays/Matrix.h"
#include "casa/Arrays/MatrixMath.h"
#include "measures/Measures.h"
#include "measures/Measures/MeasConvert.h"
#include "measures/Measures/MCEpoch.h"
#include "measures/Measures/MDirection.h"
#include "measures/Measures/MEpoch.h"
#include "measures/Measures/MeasFrame.h"
#include "measures/Measures/MCDirection.h"
#include "casa/Quanta/MVAngle.h"
#include "scimath/Mathematics/RigidVector.h"
#include "cpcommon/VisChunk.h"
#include "measures/Measures/UVWMachine.h"

// Local package includes
#include "configuration/Configuration.h" // Includes all configuration attributes too

ASKAP_LOGGER(logger, ".CalcUVWTask");

using namespace casa;
using namespace askap;
using namespace askap::cp::common;
using namespace askap::cp::ingest;

CalcUVWTask::CalcUVWTask(const LOFAR::ParameterSet& parset,
        const Configuration& config)
{
    ASKAPLOG_DEBUG_STR(logger, "Constructor");

    createPositionMatrix(config);
    setupBeamOffsets(config);
}

CalcUVWTask::~CalcUVWTask()
{
    ASKAPLOG_DEBUG_STR(logger, "Destructor");
}

void CalcUVWTask::process(VisChunk::ShPtr chunk)
{
    for (casa::uInt row = 0; row < chunk->nRow(); ++row) {
        calcForRow(chunk, row);
    }
}

/// @brief obtain phase centre for a given beam
/// @details This method encapsulates common operations to obtain the direction
/// of the phase centre for an (off-axis) beam by shifting dish pointing centre
/// @param[in] dishPointing pointing centre for the whole dish
/// @param[in] beam beam index to work 
/// @return direction measure for the phase centre
casa::MDirection CalcUVWTask::phaseCentre(const casa::MDirection &dishPointing,
                                          const casa::uInt beam) const
{
    // Current phase center
    casa::MDirection fpc(dishPointing);
    ASKAPCHECK(beam < itsBeamOffset.size(), "Beam index (" << beam << ") is invalid");

    // Shift per beam offsets
    const RigidVector<double, 2> offset = itsBeamOffset(beam);
    fpc.shift(-offset(0), offset(1), True);
    return fpc;
}

/// @brief obtain gast for the given epoch
/// @param[in] epoch UTC epoch to convert to GAST
/// @return gast in radians modulo 2pi
double CalcUVWTask::calcGAST(const casa::MVEpoch &epoch)
{
    // Determine Greenwich Apparent Sidereal Time
    MEpoch epUT1(epoch, MEpoch::UTC);
    MEpoch::Ref refGAST(MEpoch::GAST);
    MEpoch::Convert epGAST(epUT1, refGAST);
    const double gast = epGAST().get("d").getValue("d");
    return (gast - Int(gast)) * C::_2pi; // Into Radians
}

void CalcUVWTask::calcForRow(VisChunk::ShPtr chunk, const casa::uInt row)
{
    const casa::uInt ant1 = chunk->antenna1()(row);
    const casa::uInt ant2 = chunk->antenna2()(row);

    const casa::uInt nAnt = nAntennas();

    ASKAPCHECK(ant1 < nAnt, "Antenna index (" << ant1 << ") is invalid");
    ASKAPCHECK(ant2 < nAnt, "Antenna index (" << ant2 << ") is invalid");

    // Determine Greenwich Mean Sidereal Time
    const double gast = calcGAST(chunk->time()); 
    casa::MeasFrame frame(casa::MEpoch(chunk->time(), casa::MEpoch::UTC));

    // phase center for a given beam
    //const casa::MDirection fpc = phaseCentre(chunk->phaseCentre1()(row),chunk->beam1()(row));
    const casa::MDirection fpc = casa::MDirection::Convert(phaseCentre(chunk->phaseCentre1()(row),chunk->beam1()(row)),
                                    casa::MDirection::Ref(casa::MDirection::TOPO, frame))();
    const double ra = fpc.getAngle().getValue()(0);
    const double dec = fpc.getAngle().getValue()(1);

    // Transformation from antenna position difference (ant2-ant1) to uvw
    const double H0 = gast - ra;
    const double sH0 = sin(H0);
    const double cH0 = cos(H0);
    const double sd = sin(dec);
    const double cd = cos(dec);
    Matrix<double> trans(3, 3, 0);
    trans(0, 0) = -sH0; trans(0, 1) = -cH0;
    trans(1, 0) = sd * cH0; trans(1, 1) = -sd * sH0; trans(1, 2) = -cd;
    trans(2, 0) = -cd * cH0; trans(2, 1) = cd * sH0; trans(2, 2) = -sd;

    // Rotate antennas to correct frame

    /*
    // there is no need to calculate uvw per antenna here as we recalculate it per row
    // caching it per row and beam would be the optimal approach in terms of the number of
    // operations, but we leave such implementation for some time in the future

    Matrix<double> antUVW(3, nAnt);
    
    for (uInt i = 0; i < nAnt; ++i) {
        antUVW.column(i) = casa::product(trans, antXYZ(i));
    }

    double x1 = antUVW(0, ant1), y1 = antUVW(1, ant1), z1 = antUVW(2, ant1);
    double x2 = antUVW(0, ant2), y2 = antUVW(1, ant2), z2 = antUVW(2, ant2);
    Vector<double> uvwvec(3);
    uvwvec(0) = x2 - x1;
    uvwvec(1) = y2 - y1;
    uvwvec(2) = z2 - z1;
    */

    const Vector<double> baseline = antXYZ(ant2) - antXYZ(ant1);
    ASKAPDEBUGASSERT(baseline.nelements() == 3);
    Vector<double> uvwvec = casa::product(trans,baseline);
    ASKAPDEBUGASSERT(uvwvec.nelements() == 3);
    // do the conversion to J2000 in a quick and dirty way for now
    // some optimisation and caching of rotation matrix are definitely possible here
    // but cache class in accessors needs to be adapted first.
    casa::UVWMachine uvm(casa::MDirection::Ref(casa::MDirection::J2000), fpc);
    uvm.convertUVW(uvwvec);
    ASKAPDEBUGASSERT(uvwvec.nelements() == 3);

    // Finally set the uvwvec in the VisChunk
    chunk->uvw()(row) = uvwvec;
}

/// @brief obtain ITRF coordinates of a given antenna
/// @details
/// @param[in] ant antenna index
/// @return 3-element vector with X,Y and Z
casa::Vector<double> CalcUVWTask::antXYZ(const casa::uInt ant) const
{
   return itsAntXYZ.column(ant);
}

void CalcUVWTask::createPositionMatrix(const Configuration& config)
{
    const std::vector<Antenna> antennas = config.antennas();
    const size_t nAnt = antennas.size();
    itsAntXYZ = casa::Matrix<double>(3, nAnt);
    for (size_t i = 0; i < nAnt; ++i) {
        itsAntXYZ(0, i) = antennas.at(i).position()(0); // x
        itsAntXYZ(1, i) = antennas.at(i).position()(1); // y
        itsAntXYZ(2, i) = antennas.at(i).position()(2); // z
    }
}

void CalcUVWTask::setupBeamOffsets(const Configuration& config)
{
        const FeedConfig& feedConfig = config.feed();
        const uInt nFeeds = feedConfig.nFeeds();
        itsBeamOffset.resize(nFeeds);
        for (uInt feed = 0; feed < nFeeds; feed++) {
            itsBeamOffset(feed)(0) = feedConfig.offsetX(feed).getValue("rad");
            itsBeamOffset(feed)(1) = feedConfig.offsetY(feed).getValue("rad");
        }
}
