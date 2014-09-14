/// @file PhaseTrackTask.cc
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
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#include "ingestpipeline/phasetracktask/PhaseTrackTask.h"

// Include package level header file
#include "askap_cpingest.h"

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"

// casa includes
#include <measures/Measures/MeasFrame.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MCEpoch.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MCDirection.h>
#include <measures/Measures/MeasConvert.h>
#include <casa/Arrays/MatrixMath.h>

ASKAP_LOGGER(logger, ".PhaseTrackTask");

using namespace casa;
using namespace askap;
using namespace askap::cp::common;
using namespace askap::cp::ingest;

/// @brief helper method to obtain effective LO frequency
/// @details The effective LO frequency is deduced from the sky frequency as
/// ASKAP has a simple conversion chain (the effective LO and the sky frequency of
/// the first channel always have a fixed offset which is hard coded). 
/// It is handy to encapsulate the formula in one method as it is used by more
/// than one class.
/// @param[in] chunk the visibility chunk for this integration cycle
/// @return Effective LO frequency in Hz
double askap::cp::ingest::getEffectiveLOFreq(const VisChunk& chunk)
{
   // here we need the effective LO frequency, we can deduce it from the start frequency of the very first
   // channel (global, not local for this rank)
   // Below we hardcoded the formula derived from the BETA simple conversion chain (note, it may change
   // for ADE - need to check)
   //
   // BETA has 3 frequency conversions with effective LO being TunableLO - 4432 MHz - 768 MHz
   // (the last one is because digitisation acts like another LO). As a result, the spectrum is always inverted.
   // The start frequency corresponds to the top of the band and is a fixed offset from TunableLO which we need
   // to calculate the effective LO frequency. Assuming that the software correlator got the bottom of the band,
   // i.e. the last 16 of 304 channels, the effective LO is expected to be 40 MHz below the bottom of the band or
   // 344 MHz below the top of the band. This number needs to be checked when we get the actual system observing
   // an astronomical source. 
   //
   // Investigations in January 2014 revealed that the effective LO is 343.5 MHz below the top of the band which
   // is the centre of the first fine channel. The correct frequency mapping is realised if 0.5 MHz is added to
   // the centre the top coarse channel (the tunable LO corresponds to the centre of the coarse channel in the
   // middle of the band, we probably wrongfully assumed the adjacent channel initially therefore there is a 
   // correction of 1 MHz one way and 0.5 MHz the other). The tunable LO of 5872 MHz corresponds to 
   // the top fine channel frequency of 1015.5 MHz. The 343.5 MHz offset for the effective LO has been verified
   // with the 3h track on the Galactic centre and DRx delay update tolerance of 51 steps (the phase didn't jump
   // within the uncertainty of the measurement when DRx delay was updated). Note the accuracy of the measurement
   // is equivalent to a few fine channels, but there doesn't seem to be any reason why such small offset might be
   // present.
   return chunk.frequency()(0) - 343.5e6;
}


/// @brief Constructor.
/// @param[in] parset the configuration parameter set.
PhaseTrackTask::PhaseTrackTask(const LOFAR::ParameterSet& parset,
                               const Configuration& config)
    : CalcUVWTask(parset, config), itsConfig(config),
        itsTrackDelay(parset.getBool("trackdelay", false)),
        itsTrackedSouthPole(parset.getBool("trackedsouthpole", true)),
        itsFixedDelays(parset.getDoubleVector("fixeddelays", std::vector<double>()))
{
    ASKAPLOG_DEBUG_STR(logger, "Constructor");

    if (itsTrackDelay) {
        ASKAPLOG_INFO_STR(logger,
                "The phase tracking task will track the geometric delays as well (note, accuracy depends on the spectral resolution)");
    } else if (itsFixedDelays.size() != 0) {
        ASKAPLOG_INFO_STR(logger, "The phase tracking task will apply fixed delays in addition to phase rotation");
    }

    if (itsTrackDelay || (itsFixedDelays.size() != 0)) {
        if (itsTrackedSouthPole) {
            ASKAPLOG_INFO_STR(logger, "It is assumed that delays are corrected for the South Pole");
        } else {
            ASKAPLOG_INFO_STR(logger, "It is assumed that delays are corrected for the logal zenith");
        }

        if (itsFixedDelays.size()) {
            ASKAPLOG_INFO_STR(logger, "Fixed delays specified for " << itsFixedDelays.size() << " antennas:");

            for (size_t id = 0; id < itsFixedDelays.size(); ++id) {
                ASKAPLOG_INFO_STR(logger, "    antenna: " << id << " delay: " << itsFixedDelays[id] << " ns");
            }
        } else {
            ASKAPLOG_INFO_STR(logger, "No fixed delay specified");
        }
    }
}

/// @brief Phase-rotate visibilities in the specified VisChunk.
///
/// @param[in,out] chunk  the instance of VisChunk for which the
///                       phase factors will be applied.
void PhaseTrackTask::process(askap::cp::common::VisChunk::ShPtr chunk)
{
    // it may be practical to cache delay per antenna, beam
    // for now calculate it from scratch (although it is not very efficient)
    for (casa::uInt row = 0; row < chunk->nRow(); ++row) {
        phaseRotateRow(chunk, row);
    }
}

/// @brief phase rotate one row of the chunk
/// @details
/// @param[in] chunk vis chunk to work with
/// @param[in] row the row of the chunk to work with
void PhaseTrackTask::phaseRotateRow(askap::cp::common::VisChunk::ShPtr chunk,
                                    const casa::uInt row) const
{
    ASKAPDEBUGASSERT(row < chunk->nRow());
    const casa::uInt ant1 = chunk->antenna1()(row);
    const casa::uInt ant2 = chunk->antenna2()(row);

    const casa::uInt nAnt = nAntennas();

    ASKAPCHECK(ant1 < nAnt, "Antenna index (" << ant1 << ") is invalid");
    ASKAPCHECK(ant2 < nAnt, "Antenna index (" << ant2 << ") is invalid");

    // Determine Greenwich Apparent Sidereal Time
    const double gast = calcGAST(chunk->time());

    // Current JTRUE phase center
    casa::MeasFrame frame(casa::MEpoch(chunk->time(), casa::MEpoch::UTC));
    const casa::MDirection fpc = casa::MDirection::Convert(phaseCentre(chunk->phaseCentre1()(row), chunk->beam1()(row)),
                                 casa::MDirection::Ref(casa::MDirection::JTRUE, frame))();
    const double ra = fpc.getAngle().getValue()(0);
    const double dec = fpc.getAngle().getValue()(1);

    // Transformation from antenna position difference (ant2-ant1) to uvw
    const double H0 = gast - ra;
    const double sH0 = sin(H0);
    const double cH0 = cos(H0);
    const double cd = cos(dec);
    // JTRUE delay is a scalar, so transformation matrix is just a vector
    casa::Vector<double> baseline = antXYZ(ant2) - antXYZ(ant1);
    ASKAPDEBUGASSERT(baseline.nelements() == 3);
    const double delayInMetres = -cd * cH0 * baseline(0) + cd * sH0 * baseline(1) - sin(dec) * baseline(2);
    const double polDelayInMetres = baseline(2);

    // slice to get this row of data
    casa::Matrix<casa::Complex> thisRow = chunk->visibility().yzPlane(row);

    if (!itsTrackDelay) {
        const double effLOFreq = getEffectiveLOFreq(*chunk);

        const float phase = -2. * static_cast<float>(casa::C::pi * effLOFreq * delayInMetres / casa::C::c);
        const casa::Complex phasor(cos(phase), sin(phase));

        // actual rotation
        thisRow *= phasor;
    }

    if (itsTrackDelay || (ant1 < itsFixedDelays.size()) || (ant2 < itsFixedDelays.size())) {
        // fixed component of the delay in seconds
        const double fixedDelay = 1e-9 * ((ant2 < itsFixedDelays.size()) ? itsFixedDelays[ant2] : 0. - ((ant1 < itsFixedDelays.size()) ? itsFixedDelays[ant1] : 0.));
        const double delayBy2pi = -2. * casa::C::pi * (fixedDelay + (itsTrackDelay ? delayInMetres - (itsTrackedSouthPole ? polDelayInMetres : 0.) : 0.) / casa::C::c);
        const casa::Vector<double>& freqs = chunk->frequency();
        ASKAPDEBUGASSERT(thisRow.nrow() == freqs.nelements());

        for (casa::uInt ch = 0; ch < thisRow.nrow(); ++ch) {
            const float phase = static_cast<float>(delayBy2pi * (freqs[ch]));
            const casa::Complex phasor(cos(phase), sin(phase));
            casa::Vector<casa::Complex> allPols = thisRow.row(ch);
            allPols *= phasor;
            /*
            // for debugging
            if ((ant1!=ant2) && (ch == 4000)) {
                std::cout<<ch<<" "<<delayInMetres<<" "<<delayBy2pi<<" "<<freqs[ch]-freqs[0]<<std::endl;
            }
            */
        }
    }
}
