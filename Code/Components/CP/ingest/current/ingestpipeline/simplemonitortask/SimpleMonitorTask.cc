/// @file
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


#include "ingestpipeline/simplemonitortask/SimpleMonitorTask.h"

// Include package level header file
#include "askap_cpingest.h"

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "askap/AskapUtil.h"


ASKAP_LOGGER(logger, ".SimpleMonitorTask");

using namespace askap;
using namespace askap::cp::common;
using namespace askap::cp::ingest;


/// @breif Constructor
/// @param[in] parset the configuration parameter set.
SimpleMonitorTask::SimpleMonitorTask(const LOFAR::ParameterSet& parset,
             const Configuration& config) : itsCurrentTime(-1.), itsStartTime(-1.), itsBaselineMap(parset),
             itsDelayEstimator(1.), itsFileName(parset.getString("prefix",""))
{
  ASKAPLOG_DEBUG_STR(logger, "Constructor");
  ASKAPCHECK(itsBaselineMap.size() == static_cast<size_t>(itsBaselineMap.maxID() + 1), 
          "Only contiguous baseline/polarisation IDs are supported by the monitor task for simplicity");
  casa::uInt nBeam = parset.getUint32("nbeam");
  itsVisBuffer.resize(itsBaselineMap.size(), nBeam);
  itsDelayBuffer.resize(itsBaselineMap.size(), nBeam);
  itsVisBuffer.set(casa::Complex(0.,0.));
  itsDelayBuffer.set(0.);
  itsFileName += "visplot_" + utility::toString(config.rank()) + ".dat";
  ASKAPLOG_INFO_STR(logger, "Average visibilities and delays for "<<itsBaselineMap.size()<<" baseline/polarisation products and "<<nBeam<<
                            " beams will be written into "<<itsFileName);
}

/// @brief destructor
SimpleMonitorTask::~SimpleMonitorTask()
{
}

/// @brief Extract required information from visibilities in the specified VisChunk.
/// @details There is no modification of the data, just internal buffers are updated.
///
/// @param[in,out] chunk  the instance of VisChunk for which the
///                       phase factors will be applied.
void SimpleMonitorTask::process(askap::cp::common::VisChunk::ShPtr chunk)
{
   ASKAPDEBUGASSERT(chunk);
   // first check whether we have a new integration
   const double cTime = chunk->time().get() * 1440.; // in minutes
   if (cTime > itsCurrentTime) {
       // first integration?
       if (itsCurrentTime < 0) {
           itsStartTime = cTime;
       } else {
         // process the buffer
         publishBuffer();
         // start a new cycle
         itsVisBuffer.set(casa::Complex(0.,0.));
         itsDelayBuffer.set(0.);
       }
       itsCurrentTime = cTime;
   }
   //
   ASKAPCHECK(chunk->nPol() == 4, "Support only chunks with 4 polarisation products");
   const casa::Vector<casa::Stokes::StokesTypes>& stokes = chunk->stokes();
   ASKAPDEBUGASSERT(stokes.nelements() == 4);
   ASKAPCHECK(stokes[0] == casa::Stokes::XX, "The first polarisation product of the chunk is supposed to be XX");
   ASKAPCHECK(stokes[3] == casa::Stokes::YY, "The last polarisation product of the chunk is supposed to be YY");
   
   const casa::Vector<casa::uInt>& antenna1 = chunk->antenna1();
   const casa::Vector<casa::uInt>& antenna2 = chunk->antenna2();
   const casa::Vector<casa::uInt>& beam1 = chunk->beam1();
   ASKAPDEBUGASSERT(antenna1.nelements() == chunk->nRow());
   ASKAPDEBUGASSERT(antenna2.nelements() == chunk->nRow());
   ASKAPDEBUGASSERT(beam1.nelements() == chunk->nRow());

   if (chunk->nChannel() >= 2) {
       // assume equidistant channels
       const double resolution = chunk->frequency()[1] - chunk->frequency()[0];
       ASKAPCHECK(abs(resolution) > 0., "Zero frequency increment has been encountered for time: "<<chunk->time());
       itsDelayEstimator.setResolution(resolution);
   } else {
       ASKAPLOG_WARN_STR(logger, "Chunk corresponding to time="<<chunk->time()<<
                   " has insufficient number of spectral channels for a delay solution");
   }
   
   casa::uInt nMatch = 0;
   for (casa::uInt row=0; row<chunk->nRow(); ++row) {
        const casa::uInt beam = beam1[row];
        if (beam < itsVisBuffer.ncolumn()) {
            casa::Matrix<casa::Complex> thisRow  = chunk->visibility().yzPlane(row);
            ASKAPDEBUGASSERT(thisRow.ncolumn() == 4);
            // we'd probably be better off with a forward lookup as number of baselines to monitor is
            // expected to be much less than the total number of baselines. Might change it in the future
            const casa::Int idXX = itsBaselineMap.getID(antenna1[row], antenna2[row], casa::Stokes::XX);
            if (idXX > -1) {
                processRow(thisRow.column(0), static_cast<const casa::uInt>(idXX), beam);
                ++nMatch;
            }
            const casa::Int idYY = itsBaselineMap.getID(antenna1[row], antenna2[row], casa::Stokes::YY);
            if (idYY > -1) {
                processRow(thisRow.column(3), static_cast<const casa::uInt>(idYY), beam);
                ++nMatch;
            }
        }
   }
   if ((itsBaselineMap.size() != 0) && (nMatch == 0)) {
       ASKAPLOG_WARN_STR(logger, 
                "The baseline/polarisation products selected to be monitored are not found for the whole chunk with time="<<
                chunk->time());
   }
}

/// @details Process one row of data.
/// @param[in] vis vis spectrum for the given baseline/pol index to work with
/// @param[in] baseline baseline ID 
/// @param[in] beam beam ID
void SimpleMonitorTask::processRow(const casa::Vector<casa::Complex> &vis, const casa::uInt baseline, const casa::uInt beam)
{
  ASKAPDEBUGASSERT(beam < itsDelayBuffer.ncolumn());
  // average visibilities in frequency
  if (vis.nelements() > 0) {
      //const casa::Complex avgVis = vis[150];
      const casa::Complex avgVis = casa::sum(vis) / float(vis.nelements());
      itsVisBuffer(baseline,beam) = avgVis;
  }

  // estimate delays if we can
  if (vis.nelements() >= 2) {
      itsDelayBuffer(baseline,beam) = itsDelayEstimator.getDelay(vis);
  }
  // temporary code to export the spectrum for debugging of the hw correlator. 
  // the expectation is that it would be hard to keep up if we export everything. If
  // something like this is necessary then we probably need to write a separate task.
  if (beam == 0) {
      // we don't need to cater for the full MPI case
      ASKAPDEBUGASSERT(itsFileName.find("_0") != std::string::npos);
      const std::string fname = "spectra" + utility::toString(baseline)+".dat";
      std::ofstream os(fname.c_str());
      for (casa::uInt ch = 0; ch<vis.nelements(); ++ch) {
           os<<ch<<" "<<abs(vis[ch])<<" "<<arg(vis[ch]) * 180. / casa::C::pi<<std::endl;
      }
  }
  //
}

/// @brief Publish the buffer
void SimpleMonitorTask::publishBuffer() 
{
  if (!itsOStream.is_open()) {
      try {
         itsOStream.open(itsFileName.c_str());
      }
      catch (const std::exception &ex) {
         ASKAPLOG_FATAL_STR(logger, "Error opening output ascii file for monitoring information: "<<ex.what());
      }
  }
  // time is in minutes
  itsOStream<<double(itsCurrentTime - itsStartTime)<<" ";
  for (casa::uInt beam=0; beam < itsVisBuffer.ncolumn(); ++beam) {
       for (casa::uInt baseline = 0; baseline < itsVisBuffer.nrow(); ++baseline) {
            itsOStream<<abs(itsVisBuffer(baseline,beam))<<" "<<arg(itsVisBuffer(baseline,beam))/casa::C::pi*180.<<" "
               <<itsDelayBuffer(baseline,beam)*1e9<<" ";
        }
  }
  itsOStream<<std::endl;
  itsOStream.flush();
}

