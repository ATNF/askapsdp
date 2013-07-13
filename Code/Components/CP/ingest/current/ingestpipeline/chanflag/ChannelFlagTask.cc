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


#include "ingestpipeline/chanflag/ChannelFlagTask.h"
#include "configuration/BaselineMap.h"

// Include package level header file
#include "askap_cpingest.h"

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "askap/AskapUtil.h"


ASKAP_LOGGER(logger, ".ChannelFlagTask");

using namespace askap;
using namespace askap::cp::common;
using namespace askap::cp::ingest;


/// @breif Constructor
/// @param[in] parset the configuration parameter set.
/// @param[in] config configuration
ChannelFlagTask::ChannelFlagTask(const LOFAR::ParameterSet& parset, const Configuration& config) :
             itsBaselineMap(parset)
{
  ASKAPLOG_DEBUG_STR(logger, "Constructor");
  ASKAPCHECK(itsBaselineMap.size() == static_cast<size_t>(itsBaselineMap.maxID() + 1), 
          "Only contiguous baseline/polarisation IDs are supported by the monitor task for simplicity");
  const std::vector<std::string> fileNames = parset.getStringVector("flagfiles");
  ASKAPCHECK(fileNames.size() == itsBaselineMap.size(), 
            "Number of flag files given in the parset ("<<fileNames.size()<<") is expected to match the number of defined baseline/polarisation products ("<<
            itsBaselineMap.size()<<")");
  itsChannelsToFlag.resize(itsBaselineMap.size());
  for (size_t i=0; i<fileNames.size(); ++i) {
       ASKAPLOG_INFO_STR(logger, "Caching flagging rule for baseline "<<
            itsBaselineMap.idToAntenna1(int32_t(i))<<" - "<<
            itsBaselineMap.idToAntenna2(int32_t(i))<<
            ", polarisation "<<casa::Stokes::name(itsBaselineMap.idToStokes(i))<<" from file "<<fileNames[i]);
       itsChannelsToFlag[i].reserve(500);
       std::ifstream is(fileNames[i].c_str());
       while (is) {
          int buf;
          is>>buf;
          if (!is) {
              ASKAPCHECK((buf>=0) && (buf<16416), "Each channel number is expected to be between 0 and 16415 inclusive");
              itsChannelsToFlag[i].push_back(size_t(buf));
          }
       }
       ASKAPLOG_INFO_STR(logger, "    will flag "<<itsChannelsToFlag[i].size()<<" spectral channels");
  }
}

/// @brief destructor
ChannelFlagTask::~ChannelFlagTask()
{
}

/// @brief Flag seleted channels in the specified VisChunk.
/// @details This method applies static flags to excise the spikes like
/// CFB DC offset. Note, the intention is to run this task early in the chain
/// to work on full resolution. There is no check of any kind that the supplied
/// channel numbers are valid.
///
/// @param[in,out] chunk  the instance of VisChunk for which the
///                       flags will be applied.
void ChannelFlagTask::process(askap::cp::common::VisChunk::ShPtr chunk)
{
   ASKAPDEBUGASSERT(chunk);
   //
   ASKAPCHECK(chunk->nPol() == 4, "Support only chunks with 4 polarisation products");
   const casa::Vector<casa::Stokes::StokesTypes>& stokes = chunk->stokes();
   ASKAPDEBUGASSERT(stokes.nelements() == 4);
   ASKAPCHECK(stokes[0] == casa::Stokes::XX, "The first polarisation product of the chunk is supposed to be XX");
   ASKAPCHECK(stokes[1] == casa::Stokes::XY, "The second polarisation product of the chunk is supposed to be XY");
   ASKAPCHECK(stokes[2] == casa::Stokes::YX, "The third polarisation product of the chunk is supposed to be YX");
   ASKAPCHECK(stokes[3] == casa::Stokes::YY, "The last polarisation product of the chunk is supposed to be YY");
   
   const casa::Vector<casa::uInt>& antenna1 = chunk->antenna1();
   const casa::Vector<casa::uInt>& antenna2 = chunk->antenna2();
   const casa::Vector<casa::uInt>& beam1 = chunk->beam1();
   ASKAPDEBUGASSERT(antenna1.nelements() == chunk->nRow());
   ASKAPDEBUGASSERT(antenna2.nelements() == chunk->nRow());
   ASKAPDEBUGASSERT(beam1.nelements() == chunk->nRow());

   casa::uInt nMatch = 0;
   for (casa::uInt row=0; row<chunk->nRow(); ++row) {
        const casa::uInt beam = beam1[row];

        casa::Matrix<casa::Complex> thisRow  = chunk->visibility().yzPlane(row);
        casa::Matrix<casa::Bool> thisFlagRow  = chunk->flag().yzPlane(row);
        ASKAPDEBUGASSERT(thisRow.ncolumn() == 4);

        for (casa::uInt pol = 0; pol<chunk->nPol(); ++pol) {
             const casa::Int id = itsBaselineMap.getID(antenna1[row], antenna2[row], stokes[pol]);
             if (id > -1) {
                 casa::Vector<casa::Complex> thisPol = thisRow.column(pol);
                 casa::Vector<casa::Bool> thisFlagPol = thisFlagRow.column(pol);
                 processRow(thisPol, thisFlagPol, static_cast<const casa::uInt>(id), beam);
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
/// @param[in,out] vis vis spectrum for the given baseline/pol index to work with
/// @param[in,out] flag flag spectrum for the given baseline/pol index to work with
/// @param[in] baseline baseline ID 
/// For future use: param[in] beam beam ID
void ChannelFlagTask::processRow(casa::Vector<casa::Complex> &vis, casa::Vector<casa::Bool> &flag, const casa::uInt baseline, const casa::uInt)
{
  ASKAPDEBUGASSERT(baseline < itsChannelsToFlag.size());
  ASKAPDEBUGASSERT(vis.nelements() == flag.nelements());
  const std::vector<size_t>::const_iterator endMark = itsChannelsToFlag[baseline].end();
  for (std::vector<size_t>::const_iterator ci = itsChannelsToFlag[baseline].begin(); ci != endMark; ++ci) {
       ASKAPCHECK(*ci < vis.nelements(), "Encountered channel "<<*ci<<" during flagging which exceeds the total number of channels "<<vis.nelements());
       vis[*ci] = 0.;
       flag[*ci] = casa::True;
  }
}

