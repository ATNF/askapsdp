/// @file FringeRotationTask.cc
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

#include "ingestpipeline/phasetracktask/FringeRotationTask.h"
#include "ingestpipeline/phasetracktask/PhaseTrackTask.h"
#include "ingestpipeline/phasetracktask/FrtDrxDelays.h"
#include "ingestpipeline/phasetracktask/FrtHWAndDrx.h"

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
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/MatrixMath.h>

ASKAP_LOGGER(logger, ".FringeRotationTask");

using namespace casa;
using namespace askap;
using namespace askap::cp::common;
using namespace askap::cp::ingest;


/// @brief Constructor.
/// @param[in] parset the configuration parameter set.
/// @param[in] config configuration
FringeRotationTask::FringeRotationTask(const LOFAR::ParameterSet& parset,
                               const Configuration& config)
    : CalcUVWTask(parset, config), itsConfig(config),
      itsFixedDelays(parset.getDoubleVector("fixeddelays", std::vector<double>())),
      itsFrtMethod(fringeRotationMethod(parset,config)) 
{
    ASKAPLOG_DEBUG_STR(logger, "constructor of the generalised fringe rotation task");
    ASKAPLOG_INFO_STR(logger, "This is a specialised version of fringe rotation tasks used for debugging; use data on your own risk");

    if (itsFixedDelays.size() != 0) {
        ASKAPLOG_INFO_STR(logger, "The phase tracking task will apply fixed delays in addition to phase rotation");
        ASKAPLOG_INFO_STR(logger, "Fixed delays specified for " << itsFixedDelays.size() << " antennas:");

        const std::vector<Antenna> antennas = config.antennas();
        const size_t nAnt = antennas.size();
        for (size_t id = 0; id < casa::min(casa::uInt(nAnt),casa::uInt(itsFixedDelays.size())); ++id) {
             ASKAPLOG_INFO_STR(logger, "    antenna: " << antennas.at(id).name()<<" (id="<<id << ") delay: "
                     << itsFixedDelays[id] << " ns");
        }
        if (nAnt < itsFixedDelays.size()) {
            ASKAPLOG_INFO_STR(logger,  "    other fixed delays are ignored");
        }
    } else {
            ASKAPLOG_INFO_STR(logger, "No fixed delay specified");
    }
}

/// @brief process one VisChunk 
/// @details Perform fringe tracking, correct residual effects on visibilities in the
/// specified VisChunk.
///
/// @param[in,out] chunk  the instance of VisChunk for which the
///                       phase factors will be applied.
void FringeRotationTask::process(askap::cp::common::VisChunk::ShPtr chunk)
{
    ASKAPLOG_DEBUG_STR(logger, "process()");
    ASKAPDEBUGASSERT(itsFrtMethod);
    ASKAPDEBUGASSERT(chunk);

    casa::Matrix<double> delays(nAntennas(), nBeams(),0.);
    casa::Matrix<double> rates(nAntennas(),nBeams(),0.);
    // calculate delays (in seconds) and rates (in radians per seconds) for each antenna
    // and beam the values are absolute per antenna w.r.t the Earth centre

    // Determine Greenwich Mean Sidereal Time
    const double gmst = calcGMST(chunk->time());
    //casa::MeasFrame frame(casa::MEpoch(chunk->time(), casa::MEpoch::UTC));
    casa::MeasFrame frame(casa::MEpoch(chunk->time(), casa::MEpoch::UTC));
    const double effLOFreq = getEffectiveLOFreq(*chunk);
    const double siderealRate = casa::C::_2pi / 86400. / (1. - 1./365.25);

    for (casa::uInt ant = 0; ant < nAntennas(); ++ant) {
         ASKAPASSERT(chunk->phaseCentre1().nelements() > 0);
         const casa::MDirection dishPnt = casa::MDirection(chunk->phaseCentre1()[0],chunk->directionFrame());
         // fixed delay in seconds
         const double fixedDelay = ant < itsFixedDelays.size() ? itsFixedDelays[ant]*1e-9 : 0.;
         for (casa::uInt beam = 0; beam < nBeams(); ++beam) {
              // Current JTRUE phase center
              const casa::MDirection fpc = casa::MDirection::Convert(phaseCentre(dishPnt, beam),
                                    casa::MDirection::Ref(casa::MDirection::TOPO, frame))();
              const double ra = fpc.getAngle().getValue()(0);
              const double dec = fpc.getAngle().getValue()(1);

              // Transformation from antenna position to the geocentric delay
              const double H0 = gmst - ra;
              const double sH0 = sin(H0);
              const double cH0 = cos(H0);
              const double cd = cos(dec);
              // JTRUE delay is a scalar, so transformation matrix is just a vector
              // we could probably use the matrix math to process all antennas at once, however
              // do it explicitly for now for simplicity
              const casa::Vector<double> xyz = antXYZ(ant);
              ASKAPDEBUGASSERT(xyz.nelements() == 3);
              const double delayInMetres = -cd * cH0 * xyz(0) + cd * sH0 * xyz(1) - sin(dec) * xyz(2);
              delays(ant,beam) = fixedDelay + delayInMetres / casa::C::c;
              rates(ant,beam) = (cd * sH0 * xyz(0) + cd * cH0 * xyz(1)) * siderealRate * casa::C::_2pi / casa::C::c * effLOFreq;
         }
    }

    itsFrtMethod->process(chunk, delays, rates, effLOFreq);
}


/// @brief factory method for the fringe rotation approach classes
/// @details This class is used to create implementations of 
/// IFrtApproach interface based on the parset. These classes do
/// actual work on application of delays and rates
/// @param[in] parset the configuration parameter set.
// @param[in] config configuration
IFrtApproach::ShPtr FringeRotationTask::fringeRotationMethod(const LOFAR::ParameterSet& parset, 
               const Configuration & config)
{
  const std::string name = parset.getString("method");
  ASKAPLOG_INFO_STR(logger, "Selected fringe rotation method: "<<name);
  IFrtApproach::ShPtr result;
  if (name == "drxdelays") {
      result.reset(new FrtDrxDelays(parset,config));
  } else if (name == "hwanddrx") {
      result.reset(new FrtHWAndDrx(parset,config));
  }
  ASKAPCHECK(result, "Method "<<name<<" is unknown");
  return result;
}
