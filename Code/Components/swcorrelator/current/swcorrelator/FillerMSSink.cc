/// @file 
///
/// @brief Actual MS writer class doing the low-level dirty job
/// @details This class is heavily based on Ben's MSSink in the CP/ingestpipeline package.
/// I just copied the appropriate code from there. The basic approach is to set up as
/// much of the metadata as we can via the parset file. It is envisaged that we may
/// use this class also for the conversion of the DiFX output into MS. 
///
/// @copyright (c) 2007 CSIRO
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

// own includes
#include <swcorrelator/FillerMSSink.h>
#include <askap/AskapError.h>
#include <askap_swcorrelator.h>
#include <askap/AskapLogging.h>
#include <askap/AskapUtil.h>
#include <utils/PolConverter.h>

// casa includes
#include <casa/OS/File.h>
#include <casa/OS/Path.h>
#include <ms/MeasurementSets/MSColumns.h>
#include "tables/Tables/TableDesc.h"
#include "tables/Tables/SetupNewTab.h"
#include "tables/Tables/IncrementalStMan.h"
#include "tables/Tables/StandardStMan.h"
#include "tables/Tables/TiledShapeStMan.h"
#include <casa/Arrays/MatrixMath.h>

// std includes
#include <sstream>

ASKAP_LOGGER(logger, ".fillermssink");

namespace askap {

namespace swcorrelator {

/// @brief constructor, sets up  MS writer
/// @details Configuration is done via the parset, a lot of the metadata are just filled
/// via the parset.
/// @param[in] parset parset file with configuration info
FillerMSSink::FillerMSSink(const LOFAR::ParameterSet &parset) : itsParset(parset), itsDataDescID(0),
   itsFieldID(0), itsBeamOffsetUVW(parset.getBool("beamoffsetuvw",true)), itsNumberOfDataDesc(-1),
   itsNumberOfBeams(-1), itsExtraAntennas(parset.getString("beams2ants","")), itsAntHandlingExtras(-1)
{
  if (itsExtraAntennas.nRules()) {
      ASKAPLOG_INFO_STR(logger, "Some beams will be written as antennas (all indices after substitution) according to the following rule:");
      ASKAPLOG_INFO_STR(logger, "     (beamId:antId) "<<parset.getString("beams2ants"));
      itsAntHandlingExtras = parset.getInt32("hostantenna");
      ASKAPCHECK((itsAntHandlingExtras >= 0) && (itsAntHandlingExtras <= 2), "Host antenna index should be 0, 1 or 2, you have "<<itsAntHandlingExtras);
      ASKAPLOG_INFO_STR(logger, "     Host antenna Id is "<<itsAntHandlingExtras);
  } else {
      ASKAPCHECK(!parset.isDefined("hostantenna"), "hostantenna parameter is defined without beam to antenna substituting rule! Define beam2ants as well.");
  }
  create();
  initAntennasAndBeams(); 
  addObs("ASKAP", "team", 0, 0);
  initFields();
  initDataDesc();
  if (itsBeamOffsetUVW) {
      ASKAPLOG_INFO_STR(logger, "UVW will be calculated taking beam offsets into account (i.e. assuming phase tracking per beam)");   
  } else {
      ASKAPLOG_INFO_STR(logger, "UVW will be calculated for the same position for all beams (i.e. same phase tracking for all beams)");   
  }
  // trigger a dummy UVW calculation to get measures set up their caches in the main thread and avoid race condition
  CorrProducts dummy(1,0);
  dummy.itsBAT = 55000000000ull*86400ull;
  calculateUVW(dummy);
}

/// @brief calculate uvw for the given buffer
/// @param[in] buf products buffer
/// @note The calculation is bypassed if itsUVWValid flag is already set in the buffer
/// @return time epoch corresponding to the BAT of the buffer
casa::MEpoch FillerMSSink::calculateUVW(CorrProducts &buf) const
{
  // note, we need to specify 'ull' type for the constant as the value exceeds the capacity of long, 
  // which is assumed by default
  const uint64_t microsecondsPerDay = 86400000000ull;
  const casa::MVEpoch timeTAI(double(buf.itsBAT / microsecondsPerDay), double(buf.itsBAT % microsecondsPerDay)/double(microsecondsPerDay));
  const casa::MEpoch epoch = casa::MEpoch::Convert(casa::MEpoch(timeTAI, casa::MEpoch::Ref(casa::MEpoch::TAI)), 
                             casa::MEpoch::Ref(casa::MEpoch::UTC))();
  if (buf.itsUVWValid) {
      return epoch;
  }
  ASKAPLOG_DEBUG_STR(logger, "calculateUVW: BAT="<<buf.itsBAT<<" corresponds to UT epoch: "<<epoch.getValue());
  buf.itsUVWValid = true;
  // only 3 antennas are supported
  buf.itsUVW.resize(3,3);
  ASKAPDEBUGASSERT(itsAntXYZ.nrow() >= 3);
  ASKAPDEBUGASSERT(buf.itsBeam < int(itsBeamOffsets.nrow()));
  ASKAPDEBUGASSERT(itsBeamOffsets.ncolumn() == 2);
  casa::MDirection phaseCntr(itsDishPointing);
  // need to rotate beam offsets here if we dish rotation does not compensate parallactic angle rotation perfectly
  if (itsBeamOffsetUVW) {
      phaseCntr.shift(-itsBeamOffsets(buf.itsBeam,0), itsBeamOffsets(buf.itsBeam,1), casa::True);
  }
  const double ra = phaseCntr.getAngle().getValue()(0);
  const double dec = phaseCntr.getAngle().getValue()(1);
  const double gmstInDays = casa::MEpoch::Convert(epoch,casa::MEpoch::Ref(casa::MEpoch::GMST1))().get("d").getValue("d");
  const double gmst = (gmstInDays - casa::Int(gmstInDays)) * casa::C::_2pi; // in radians
  
  const double H0 = gmst - ra, sH0 = sin(H0), cH0 = cos(H0), sd = sin(dec), cd = cos(dec);
  // quick and dirty calculation without taking aberration and other fine effects into account
  // it should be fine for the sort of baselines we have with BETA3
  casa::Matrix<double> trans(3, 3, 0.);
  trans(0, 0) = -sH0; trans(0, 1) = -cH0;
  trans(1, 0) = sd * cH0; trans(1, 1) = -sd * sH0; trans(1, 2) = -cd;
  trans(2, 0) = -cd * cH0; trans(2, 1) = cd * sH0; trans(2, 2) = -sd;
  const casa::Matrix<double> antUVW = casa::product(trans,casa::transpose(itsAntXYZ));
  for (casa::uInt baseline = 0; baseline < buf.itsUVW.nrow(); ++baseline) {
       for (casa::uInt dim = 0; dim < buf.itsUVW.ncolumn(); ++dim) {
            buf.itsUVW(baseline,dim) = antUVW(dim,substituteAntId(theirAntIDs[baseline][1], buf.itsBeam)) - antUVW(dim,substituteAntId(theirAntIDs[baseline][0], buf.itsBeam));
       }
  }
  return epoch;
}

/// @brief antenna indicies for all 3 baselines in our standard order
const int FillerMSSink::theirAntIDs[3][2] = {{0, 1}, {1,2}, {0, 2}};

  
/// @brief write one buffer to the measurement set
/// @details Current fieldID and dataDescID are assumed
/// @param[in] buf products buffer
/// @note This method could've received a const reference to the buffer. However, more
/// workarounds would be required with casa arrays, so we don't bother doing this at the moment.
/// In addition, we could call calculateUVW inside this method (but we still need an option to
/// calculate uvw's ahead of writing the buffer if we implement some form of delay tracking).
void FillerMSSink::write(CorrProducts &buf) const
{
  const casa::MEpoch epoch = calculateUVW(buf);
  ASKAPDEBUGASSERT(itsMs);
  casa::MSColumns msc(*itsMs);
  const casa::uInt baseRow = msc.nrow();
  const casa::uInt newRows = buf.itsVisibility.nrow();
  ASKAPDEBUGASSERT(newRows == 3);
  itsMs->addRow(newRows);

  // First set the constant things outside the loop,
  // as they apply to all rows
  msc.scanNumber().put(baseRow, 0);
  msc.fieldId().put(baseRow, itsFieldID);
  msc.dataDescId().put(baseRow, itsDataDescID);

  msc.time().put(baseRow, epoch.getValue().getTime().getValue("s"));
  msc.timeCentroid().put(baseRow, epoch.getValue().getTime().getValue("s") + 0.5);

  msc.arrayId().put(baseRow, 0);
  msc.processorId().put(baseRow, 0);
  msc.exposure().put(baseRow, 1.);
  msc.interval().put(baseRow, 1.);
  msc.observationId().put(baseRow, 0);
  msc.stateId().put(baseRow, -1);
  for (casa::uInt i = 0; i < newRows; ++i) {
       const casa::uInt row = i + baseRow;
       msc.antenna1().put(row, substituteAntId(theirAntIDs[i][0], buf.itsBeam));
       msc.antenna2().put(row, substituteAntId(theirAntIDs[i][1], buf.itsBeam));
       msc.feed1().put(row, buf.itsBeam);
       msc.feed2().put(row, buf.itsBeam);
       msc.uvw().put(row, buf.itsUVW.row(i));

       const casa::uInt npol = 2;
       casa::Matrix<casa::Complex> visBuf(npol,buf.itsVisibility.ncolumn());
       casa::Matrix<casa::Bool> flagBuf(npol,buf.itsFlag.ncolumn());
       for (casa::uInt pol = 0; pol < npol; ++pol) {
            visBuf.row(pol) = buf.itsVisibility.row(i);
            flagBuf.row(pol) = buf.itsFlag.row(i);
       }
       msc.data().put(row, visBuf);
       msc.flag().put(row, flagBuf);
       msc.flagRow().put(row, casa::False);

       const casa::Vector<casa::Float> tmp(npol, 1.0);
       msc.weight().put(row, tmp);
       msc.sigma().put(row, tmp);
  }

  //
  // Update the observation table
  //
  // If this is the first integration cycle update the start time,
  // otherwise just update the end time.
  const casa::Double Tstart = epoch.getValue().getTime().getValue("s");

  casa::MSObservationColumns& obsc = msc.observation();
  casa::Vector<casa::Double> timeRange = obsc.timeRange()(0);
  if (timeRange(0) == 0) {
      timeRange(0) = Tstart; 
  }

  const casa::Double Tend = Tstart + 1;
  timeRange(1) = Tend;
  obsc.timeRange().put(0, timeRange);  
  // to avoid a corrupted MS if the process terminates abnormally outside write
  itsMs->flush();
}


/// @brief read beam information, populate itsBeamOffsets
void FillerMSSink::readBeamInfo()
{
   LOFAR::ParameterSet parset(itsParset);

   if (parset.isDefined("feeds.definition")) {
       parset = LOFAR::ParameterSet(itsParset.getString("feeds.definition"));
   }

    std::vector<std::string> feedNames(parset.getStringVector("feeds.names"));
    int nFeeds = int(feedNames.size());
    ASKAPCHECK(nFeeds > 0, "No feeds specified");
    const std::string mode = parset.getString("feeds.mode","perfect X Y");
    ASKAPCHECK(mode == "perfect X Y", "Unknown feed mode: "<<mode);

    double spacing = 1.;
    if (parset.isDefined("feeds.spacing")) {
        const casa::Quantity qspacing = asQuantity(parset.getString("feeds.spacing"));
        spacing = qspacing.getValue("rad");
        ASKAPLOG_INFO_STR(logger, "Scaling beam offsets by " << qspacing);
    }
    itsBeamOffsets.resize(nFeeds,2);
    for (int feed = 0; feed < nFeeds; ++feed) {
        std::ostringstream os;
        os << "feeds." << feedNames[feed];
        std::vector<double> xy(parset.getDoubleVector(os.str()));
        ASKAPCHECK(xy.size() == 2, "Expect two elements in the beam offset vector, you have: "<<xy);
        itsBeamOffsets(feed,0) = xy[0];
        itsBeamOffsets(feed,1) = xy[1];
    }
    ASKAPLOG_INFO_STR(logger, "Successfully defined " << nFeeds << " beams");
}

/// @brief Initialises ANTENNA and FEED tables
/// @details This method extracts configuration from the parset and fills in the 
/// compulsory ANTENNA and FEED tables. It also caches antenna positions and beam offsets 
/// in the form suitable for calculation of uvw's.
void FillerMSSink::initAntennasAndBeams()
{
  readBeamInfo();
  ASKAPDEBUGASSERT(itsBeamOffsets.nrow()>0);
  ASKAPDEBUGASSERT(itsBeamOffsets.ncolumn() == 2);  
  const casa::Vector<casa::String> polTypes(itsBeamOffsets.nrow(), "X Y");
  
  // read antenna layout
  LOFAR::ParameterSet parset(itsParset);
  if (parset.isDefined("antennas.definition")) {
      parset = LOFAR::ParameterSet(itsParset.getString("antennas.definition"));
  }
  
  const std::string telName = parset.getString("antennas.telescope");
  ASKAPLOG_INFO_STR(logger, "Defining array layout for " << telName);
  std::ostringstream oos;
  oos << "antennas." << telName << ".";
  LOFAR::ParameterSet antParset(parset.makeSubset(oos.str()));

  ASKAPCHECK(antParset.isDefined("names"), "Subset (antennas."<<telName<<") of the antenna definition parset does not have 'names' keyword.");
  std::vector<std::string> antNames(antParset.getStringVector("names"));
  const int nAnt = int(antNames.size());
  ASKAPCHECK(nAnt > 0, "No antennas defined in parset file");

  /// Csimulator.ASKAP.mount=equatorial
  const std::string mount = antParset.getString("mount", "equatorial");
  ASKAPCHECK((mount == "equatorial") || (mount == "alt-az"), "Antenna mount unknown: "<<mount);

  /// Csimulator.ASKAP.mount=equatorial
  const double diameter = asQuantity(antParset.getString("diameter", "12m")).getValue("m");
  ASKAPCHECK(diameter > 0.0, "Antenna diameter not positive, diam="<<diameter);
  const std::string coordinates = antParset.getString("coordinates", "local");
  ASKAPCHECK((coordinates == "global") || (coordinates == "local"), "Coordinates type unknown: "<<coordinates);

  const double scale = antParset.getDouble("scale", 1.0);

  /// Now we get the coordinates for each antenna in turn
  itsAntXYZ.resize(nAnt,3);

  /// antennas.ASKAP.location=[+115deg, -26deg, 192km, WGS84]
  casa::MPosition location;
  if (coordinates == "local") {
      location = asMPosition(antParset.getStringVector("location"));
  }

  /// Antenna information in the form:
  /// antennas.ASKAP.antenna0=[x,y,z]
  /// ...
  for (int iant = 0; iant < nAnt; ++iant) {
       const vector<double> xyz = antParset.getDoubleVector(antNames[iant]);
       itsAntXYZ(iant,0) = xyz[0] * scale;
       itsAntXYZ(iant,1) = xyz[1] * scale;
       itsAntXYZ(iant,2) = xyz[2] * scale;
       if (coordinates == "local") {
           casa::MPosition::Convert loc2(location, casa::MPosition::ITRF);
           const casa::MPosition locitrf(loc2());
           const casa::Vector<double> angRef = locitrf.getAngle("rad").getValue();
           const double cosLong = cos(angRef(0));
           const double sinLong = sin(angRef(0));
           const double cosLat = cos(angRef(1));
           const double sinLat = sin(angRef(1));
           
           const double xG1 = -sinLat * itsAntXYZ(iant,1) + cosLat * itsAntXYZ(iant,2);
           const double yG1 = itsAntXYZ(iant,0);

           casa::Vector<double> xyzNew = locitrf.get("m").getValue();
           xyzNew(0) += cosLong * xG1 - sinLong * yG1;
           xyzNew(1) += sinLong * xG1 + cosLong * yG1;
           xyzNew(2) += cosLat * itsAntXYZ(iant,1) + sinLat * itsAntXYZ(iant,2);
           itsAntXYZ.row(iant) = xyzNew;               
       }
       addAntenna(telName, itsAntXYZ.row(iant),antNames[iant], mount, diameter);
       
       // setup feeds corresponding to this antenna
       ASKAPDEBUGASSERT(iant>=0);
       addFeeds(casa::uInt(iant), itsBeamOffsets.column(0), itsBeamOffsets.column(1), polTypes);
   }
   ASKAPLOG_INFO_STR(logger, "Successfully defined " << nAnt
           << " antennas of " << telName);  
}


/// @brief initialises field information
void FillerMSSink::initFields()
{
   LOFAR::ParameterSet parset(itsParset);

   if (itsParset.isDefined("sources.definition")) {
       parset = LOFAR::ParameterSet(itsParset.getString("sources.definition"));
   }

   const std::vector<std::string> sources = parset.getStringVector("sources.names");
   ASKAPCHECK(sources.size()>0, "At least one field has to be defined in the parset!");
   const std::string defaultName = parset.getString("defaultfield",sources[0]);
   bool defaultNameSighted = false;
   for (size_t i = 0; i < sources.size(); ++i) {
        ASKAPLOG_INFO_STR(logger, "Defining FIELD table entry for " << sources[i]);
        const std::string dirPar = std::string("sources.") + sources[i] + ".direction";
        const casa::MDirection direction(asMDirection(parset.getStringVector(dirPar)));
        const std::string calCode = parset.getString("sources." + sources[i] + ".calcode", "");
        const casa::uInt fieldID = addField(sources[i], direction, calCode); 
        if (sources[i] == defaultName) {
            itsFieldID = fieldID;
            defaultNameSighted = true;
            itsDishPointing = direction;
        }
   }
   ASKAPCHECK(defaultNameSighted, "Default field name "<<defaultName<<" is not present in field names "<<sources);

   ASKAPLOG_INFO_STR(logger, "Successfully defined "<<sources.size()<<" sources (fields), default fieldID is "<<itsFieldID);   
}
  
/// @brief initialises spectral and polarisation info (data descriptor)
void FillerMSSink::initDataDesc()
{
   LOFAR::ParameterSet parset(itsParset);

   if (itsParset.isDefined("spws.definition")) {
       parset = LOFAR::ParameterSet(itsParset.getString("spws.definition"));
   }

   std::vector<std::string> names(parset.getStringVector("spws.names"));
   const size_t nSpw = names.size();
   ASKAPCHECK(nSpw > 0, "No spectral windows defined");
   const std::string defaultWindow = parset.getString("defaultwindow",names[0]);
   bool defaultWindowSighted = false;
   for (size_t spw = 0; spw < nSpw; ++spw) {
        std::vector<std::string> line = parset.getStringVector("spws." + names[spw]);
        ASKAPASSERT(line.size() >= 4);
        const casa::Quantity startFreq = asQuantity(line[1]);
        const casa::Quantity freqInc = asQuantity(line[2]);
        ASKAPCHECK(startFreq.isConform("Hz"), "start frequency for spectral window " << names[spw] << " is supposed to be in units convertible to Hz, you gave " <<
                   line[1]);
        ASKAPCHECK(freqInc.isConform("Hz"), "frequency increment for spectral window " << names[spw] << " is supposed to be in units convertible to Hz, you gave " <<
                   line[1]);
        const int numChan = askap::utility::fromString<int>(line[0]);           
        const casa::Int spWinID = addSpectralWindow(names[spw], numChan, startFreq, freqInc);
        const casa::Int polID = addPolarisation(scimath::PolConverter::fromString(line[3]));
        const casa::Int dataDescID = addDataDesc(spWinID, polID);
        if (names[spw] == defaultWindow) {
            defaultWindowSighted = true;
            itsDataDescID = dataDescID;
            itsNumberOfChannels = numChan;
        }
    }

    ASKAPLOG_INFO_STR(logger, "Successfully defined " << nSpw << " spectral windows");
   
}
  

/// @brief helper method to make a string out of an integer
/// @param[in] in unsigned integer number
/// @return a string padded with zero on the left size, if necessary
std::string FillerMSSink::makeString(const casa::uInt in)
{
  ASKAPASSERT(in<100);
  std::string result;
  if (in<10) {
      result += "0";
  }
  result+=utility::toString(in);
  return result;
}


/// @brief Create the measurement set
void FillerMSSink::create()
{
    // Get configuration first to ensure all parameters are present
    casa::uInt bucketSize = itsParset.getUint32("stman.bucketsize", 128 * 1024);
    casa::uInt tileNcorr = itsParset.getUint32("stman.tilencorr", 4);
    casa::uInt tileNchan = itsParset.getUint32("stman.tilenchan", 1);
    casa::String filename = itsParset.getString("filename","");
    if (filename == "") {
        casa::Time tm;
        tm.now();
        filename = utility::toString(tm.year())+"-"+makeString(tm.month())+"-"+makeString(tm.dayOfMonth())+"_"+
                   makeString(tm.hours())+makeString(tm.minutes())+makeString(tm.seconds())+".ms";        
    }
    casa::Path outPath(itsParset.getString("basepath",""));
    outPath.append(filename);
    filename = outPath.expandedName();

    if (bucketSize < 8192) {
        bucketSize = 8192;
    }
    if (tileNcorr < 1) {
        tileNcorr = 1;
    }
    if (tileNchan < 1) {
        tileNchan = 1;
    }

    ASKAPLOG_INFO_STR(logger, "Creating dataset " << filename);
    ASKAPCHECK(!casa::File(filename).exists(), "File or table "<<filename<<" already exists!");

    // Make MS with standard columns
    casa::TableDesc msDesc(casa::MS::requiredTableDesc());

    // Add the DATA column.
    casa::MS::addColumnToDesc(msDesc, casa::MS::DATA, 2);

    casa::SetupNewTable newMS(filename, msDesc, casa::Table::New);

    // Set the default Storage Manager to be the Incr one
    {
        casa::IncrementalStMan incrStMan("ismdata", bucketSize);
        newMS.bindAll(incrStMan, casa::True);
    }

    // Bind ANTENNA1, and ANTENNA2 to the standardStMan 
    // as they may change sufficiently frequently to make the
    // incremental storage manager inefficient for these columns.

    {
        casa::StandardStMan ssm("ssmdata", bucketSize);
        newMS.bindColumn(casa::MS::columnName(casa::MS::ANTENNA1), ssm);
        newMS.bindColumn(casa::MS::columnName(casa::MS::ANTENNA2), ssm);
        newMS.bindColumn(casa::MS::columnName(casa::MS::UVW), ssm);
    }

    // These columns contain the bulk of the data so save them in a tiled way
    {
        // Get nr of rows in a tile.
        const int nrowTile = std::max(1u, bucketSize / (8*tileNcorr*tileNchan));
        casa::TiledShapeStMan dataMan("TiledData",
                casa::IPosition(3, tileNcorr, tileNchan, nrowTile));
        newMS.bindColumn(casa::MeasurementSet::columnName(casa::MeasurementSet::DATA),
                dataMan);
        newMS.bindColumn(casa::MeasurementSet::columnName(casa::MeasurementSet::FLAG),
                dataMan);
    }
    {
        const int nrowTile = std::max(1u, bucketSize / (4*8));
        casa::TiledShapeStMan dataMan("TiledWeight",
                casa::IPosition(2, 4, nrowTile));
        newMS.bindColumn(casa::MeasurementSet::columnName(casa::MeasurementSet::SIGMA),
                dataMan);
        newMS.bindColumn(casa::MeasurementSet::columnName(casa::MeasurementSet::WEIGHT),
                dataMan);
    }

    // Now we can create the MeasurementSet and add the (empty) subtables
    itsMs.reset(new casa::MeasurementSet(newMS, 0));
    itsMs->createDefaultSubtables(casa::Table::New);
    itsMs->flush();

    // Set the TableInfo
    {
        casa::TableInfo& info(itsMs->tableInfo());
        info.setType(casa::TableInfo::type(casa::TableInfo::MEASUREMENTSET));
        info.setSubType(casa::String(""));
        info.readmeAddLine("This is a MeasurementSet Table holding astronomical observations obtained with ASKAP software correlator");
    }
}

// methods borrowed from Ben's MSSink class (see CP/ingest)
casa::Int FillerMSSink::addObs(const casa::String& telescope, 
        const casa::String& observer,
        const double obsStartTime,
        const double obsEndTime)
{
    casa::MSColumns msc(*itsMs);
    casa::MSObservation& obs = itsMs->observation();
    casa::MSObservationColumns& obsc = msc.observation();
    const casa::uInt row = obsc.nrow();
    obs.addRow();
    obsc.telescopeName().put(row, telescope);
    casa::Vector<double> timeRange(2);
    timeRange(0) = obsStartTime;
    timeRange(1) = obsEndTime;
    obsc.timeRange().put(row, timeRange);
    obsc.observer().put(row, observer);

    // Post-conditions
    ASKAPCHECK(obsc.nrow() == (row + 1), "Unexpected observation row count");

    return row;
}

casa::Int FillerMSSink::addField(const casa::String& fieldName,
        const casa::MDirection& fieldDirection,
        const casa::String& calCode)
{
    casa::MSColumns msc(*itsMs);
    casa::MSFieldColumns& fieldc = msc.field();
    const casa::uInt row = fieldc.nrow();

    ASKAPLOG_INFO_STR(logger, "Creating new field " << fieldName << ", ID "
            << row);

    itsMs->field().addRow();
    fieldc.name().put(row, fieldName);
    fieldc.code().put(row, calCode);
    fieldc.time().put(row, 0.0);
    fieldc.numPoly().put(row, 0);
    fieldc.sourceId().put(row, 0);
    casa::Vector<casa::MDirection> direction(1);
    direction(0) = fieldDirection;
    fieldc.delayDirMeasCol().put(row, direction);
    fieldc.phaseDirMeasCol().put(row, direction);
    fieldc.referenceDirMeasCol().put(row, direction);

    // Post-conditions
    ASKAPCHECK(fieldc.nrow() == (row + 1), "Unexpected field row count");

    return row;
}

void FillerMSSink::addFeeds(const casa::Int antennaID,
        const casa::Vector<double>& x,
        const casa::Vector<double>& y,
        const casa::Vector<casa::String>& polType)
{
    // Pre-conditions
    const casa::uInt nFeeds = x.size();
    ASKAPCHECK(nFeeds == y.size(), "X and Y vectors must be of equal length");
    ASKAPCHECK(nFeeds == polType.size(),
            "Pol type vector must have the same length as X and Y");

    // Add to the Feed table
    casa::MSColumns msc(*itsMs);
    casa::MSFeedColumns& feedc = msc.feed();
    const casa::uInt startRow = feedc.nrow();
    itsMs->feed().addRow(nFeeds);

    for (casa::uInt i = 0; i < nFeeds; ++i) {
        casa::uInt row = startRow + i;
        feedc.antennaId().put(row, antennaID);
        feedc.feedId().put(row, i);
        feedc.spectralWindowId().put(row, -1);
        feedc.beamId().put(row, 0);
        feedc.numReceptors().put(row, 2);

        // Feed position
        casa::Vector<double> feedXYZ(3, 0.0);
        feedc.position().put(row, feedXYZ);

        // Beam offset
        casa::Matrix<double> beamOffset(2, 2);
        beamOffset(0, 0) = x(i);
        beamOffset(1, 0) = y(i);
        beamOffset(0, 1) = x(i);
        beamOffset(1, 1) = y(i);
        feedc.beamOffset().put(row, beamOffset);

        // Polarisation type
        casa::Vector<casa::String> feedPol(2);
        if (polType(i).contains("X", 0)) {
            feedPol(0) = "X";
            feedPol(1) = "Y";
        } else {
            feedPol(0) = "L";
            feedPol(1) = "R";
        }
        feedc.polarizationType().put(row, feedPol);

        // Polarisation response
        casa::Matrix<casa::Complex> polResp(2, 2);
        polResp = casa::Complex(0.0, 0.0);
        polResp(1, 1) = casa::Complex(1.0, 0.0);
        polResp(0, 0) = casa::Complex(1.0, 0.0);
        feedc.polResponse().put(row, polResp);

        // Receptor angle
        casa::Vector<double> feedAngle(2, 0.0);
        feedc.receptorAngle().put(row, feedAngle);

        // Time
        feedc.time().put(row, 0.0);

        // Interval - 1.e30 is effectivly forever
        feedc.interval().put(row, 1.e30);
    };

    // Post-conditions
    ASKAPCHECK(feedc.nrow() == (startRow + nFeeds), "Unexpected feed row count");
    itsNumberOfBeams = int(nFeeds);
}

casa::Int FillerMSSink::addAntenna(const casa::String& station,
        const casa::Vector<double>& antXYZ,
        const casa::String& name,
        const casa::String& mount,
        const casa::Double& dishDiameter)
{
    // Pre-conditions
    ASKAPCHECK(antXYZ.size() == 3, "Antenna position vector must contain 3 elements");

    // Write the rows to the measurement set
    casa::MSColumns msc(*itsMs);
    casa::MSAntennaColumns& antc = msc.antenna();
    const casa::uInt row = antc.nrow();

    casa::MSAntenna& ant = itsMs->antenna();
    ant.addRow();

    antc.name().put(row, name);
    antc.station().put(row,station);
    antc.type().put(row, "GROUND-BASED");
    antc.mount().put(row, mount);
    antc.position().put(row, antXYZ);
    antc.dishDiameter().put(row, dishDiameter);
    antc.flagRow().put(row, false);

    // Post-conditions
    ASKAPCHECK(antc.nrow() == (row + 1), "Unexpected antenna row count");

    return row;
}

casa::Int FillerMSSink::addDataDesc(const casa::Int spwId, const casa::Int polId)
{
    // 1: Add new row and determine its offset
    casa::MSColumns msc(*itsMs);
    casa::MSDataDescColumns& ddc = msc.dataDescription();
    const casa::uInt row = ddc.nrow();
    itsMs->dataDescription().addRow();

    // 2: Populate DATA DESCRIPTION table
    ddc.flagRow().put(row, casa::False);
    ddc.spectralWindowId().put(row, spwId);
    ddc.polarizationId().put(row, polId);

    // 3: update number of data descriptors
    if (int(row) + 1 > itsNumberOfDataDesc) {
        itsNumberOfDataDesc = int(row) + 1;
    }
    return row;
}

casa::Int FillerMSSink::addSpectralWindow(const casa::String& spwName,
            const int nChan,
            const casa::Quantity& startFreq,
            const casa::Quantity& freqInc)
{
    casa::MSColumns msc(*itsMs);
    casa::MSSpWindowColumns& spwc = msc.spectralWindow();
    const casa::uInt row = spwc.nrow();
    ASKAPLOG_INFO_STR(logger, "Creating new spectral window " << spwName
            << ", ID " << row);

    itsMs->spectralWindow().addRow();

    spwc.numChan().put(row, nChan);
    spwc.name().put(row, spwName);
    spwc.netSideband().put(row, 1);
    spwc.ifConvChain().put(row, 0);
    spwc.freqGroup().put(row, 0);
    spwc.freqGroupName().put(row, "Group 1");
    spwc.flagRow().put(row, casa::False);
    spwc.measFreqRef().put(row, casa::MFrequency::TOPO);

    casa::Vector<double> freqs(nChan);
    casa::Vector<double> bandwidth(nChan, freqInc.getValue("Hz"));

    double vStartFreq(startFreq.getValue("Hz"));
    double vFreqInc(freqInc.getValue("Hz"));

    for (casa::Int chan = 0; chan < nChan; chan++) {
        freqs(chan) = vStartFreq + chan * vFreqInc;
    }

    spwc.refFrequency().put(row, vStartFreq);
    spwc.chanFreq().put(row, freqs);
    spwc.chanWidth().put(row, bandwidth);
    spwc.effectiveBW().put(row, bandwidth);
    spwc.resolution().put(row, bandwidth);
    spwc.totalBandwidth().put(row, nChan * vFreqInc);

    return row;
}

casa::Int FillerMSSink::addPolarisation(const casa::Vector<casa::Stokes::StokesTypes>& stokesTypes)
{
    const casa::Int nCorr = stokesTypes.size();

    casa::MSColumns msc(*itsMs);
    casa::MSPolarizationColumns& polc = msc.polarization();
    const casa::uInt row = polc.nrow();
    itsMs->polarization().addRow();

    polc.flagRow().put(row, casa::False);
    polc.numCorr().put(row, nCorr);

    // Translate stokesTypes into receptor products, catch invalid
    // fallibles.
    casa::Matrix<casa::Int> corrProduct(casa::uInt(2), casa::uInt(nCorr));
    casa::Fallible<casa::Int> fi;

    casa::Vector<casa::Int> stokesTypesInt(nCorr);
    for (casa::Int i = 0; i < nCorr; i++) {
        fi = casa::Stokes::receptor1(stokesTypes(i));
        corrProduct(0, i) = (fi.isValid() ? fi.value() : 0);
        fi = casa::Stokes::receptor2(stokesTypes(i));
        corrProduct(1, i) = (fi.isValid() ? fi.value() : 0);
        stokesTypesInt(i) = stokesTypes(i);
    }

    polc.corrType().put(row, stokesTypesInt);
    polc.corrProduct().put(row, corrProduct);

    return row;
}

/// @brief obtain the number of channels in the current setup
/// @details This method throws an exception if the number of channels has not been
/// set up (normally it takes place when MS is initialised)
/// @return the number of channels in the active configuration
int FillerMSSink::nChan() const
{
  ASKAPCHECK(itsNumberOfChannels > 0, "A positive number of channels is expected, you have "<<
             itsNumberOfChannels<<", check that it has been initialised");
  return itsNumberOfChannels;           
}

/// @brief obtain number of defined data descriptors
/// @return number of data descriptors
int FillerMSSink::numDataDescIDs() const
{
  return itsNumberOfDataDesc;
}
  
/// @brief set new default data descriptor
/// @details This will be used for all future write operations
/// @param[in] desc new data descriptor
void FillerMSSink::setDataDescID(const int desc)
{
  ASKAPCHECK((desc >= 0) && (desc < numDataDescIDs()), 
       "Data Descriptor ID is supposed to be a non-negative number not exceeding the number of spectral setups in your parset = "<<
       numDataDescIDs()<<" you have "<<desc);
  itsDataDescID = desc;
}

/// @brief obtain number of beams in the current setup
/// @details This method throws an exception if the number of beams has not been
/// set up  (normally it takes place when MS is initialised)
/// @return the number of beams
int FillerMSSink::nBeam() const
{
  ASKAPCHECK(itsNumberOfBeams > 0, "A positive number of beams is expected, you have "<<itsNumberOfBeams<<
             ", check that it has been initialised");
  return itsNumberOfBeams;           
}

/// @brief return baseline index for a given baseline
/// @details The data are passed in CorrProducts structure gathering all baselines in a single
/// matrix (for visibility data and for flags). There is a standard order (see also theirAntIDs)
/// of baselines. In the software correlator itself, the data are produced directly in the standard
/// order, but this method is handy for other uses of this class (i.e. format converter). It
/// returns an index for a given baseline
/// @param[in] ant1 zero-based indicex of the first antenna  
/// @param[in] ant2 zero-based indicex of the second antenna
/// @return index into visibility of flag matrix (row of the matrix)
/// @note a negative value is returned if the given baseline is not found
int FillerMSSink::baselineIndex(const casa::uInt ant1, const casa::uInt ant2)
{
   for (int i=0; i<3; ++i) {
        if ((theirAntIDs[i][0] == int(ant1)) && (theirAntIDs[i][1] == int(ant2))) {
             return i;
        }
   }
   return -1;
}

/// @brief helper method to substitute antenna index
/// @details This is required to be able to use 4th (or potentially even more) antennas 
/// connected through beamformer of another antenna. The correlator is still running in 3-antenna mode,
/// but records the given beam data as correlations with extra antennas (so a useful measurement set is
/// produced). The method substitutes an index in the range of 0-2 to an index > 2 if the appropriate
/// beam and antenna are selected
/// @param[in] antenna input antenna index
/// @param[in] beam beam index (controls whether and how the substitution is done)
/// @return output antenna index into itsAntXYZ
int FillerMSSink::substituteAntId(const int antenna, const int beam) const
{
  if (itsExtraAntennas.nRules() == 0) {
      // no extra antennas defined, just return the original index
      return antenna;
  }
  const int result = itsExtraAntennas(beam);
  if (result < 0) {
      // this particular beam is not mapped, return the original index
      return antenna;
  }
  ASKAPDEBUGASSERT(itsAntHandlingExtras >= 0);
  if (antenna != itsAntHandlingExtras) {
      // index is unchanged, as this is not the host antenna
      return antenna;
  }
  // result is the new antenna index, we substitute host antenna with an extra one
  ASKAPDEBUGASSERT(result < int(itsAntXYZ.nrow()));
  return result;
}


} // namespace swcorrelator

} // namespace askap

