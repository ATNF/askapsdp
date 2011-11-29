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

// casa includes
#include <casa/OS/File.h>
#include <ms/MeasurementSets/MSColumns.h>
#include "tables/Tables/TableDesc.h"
#include "tables/Tables/SetupNewTab.h"
#include "tables/Tables/IncrementalStMan.h"
#include "tables/Tables/StandardStMan.h"
#include "tables/Tables/TiledShapeStMan.h"

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
   itsFieldID(0)
{
  create();
  initAntennasAndBeams(); 
  addObs("ASKAP", "team", 0, 0);
  initFields();
  initDataDesc();
}


/// @brief Initialises ANTENNA and FEED tables
/// @details This method extracts configuration from the parset and fills in the 
/// compulsory ANTENNA and FEED tables. It also caches antenna positions and beam offsets 
/// in the form suitable for calculation of uvw's.
void FillerMSSink::initAntennasAndBeams()
{
  LOFAR::ParameterSet parset(itsParset);
  if (parset.isDefined("antennas.definition")) {
      parset = LOFAR::ParameterSet(itsParset.getString("antennas.definition"));
  }
  
  const std::string telName = parset.getString("antennas.telescope");
  ASKAPLOG_INFO_STR(logger, "Simulating " << telName);
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
   }
   ASKAPLOG_INFO_STR(logger, "Successfully defined " << nAnt
           << " antennas of " << telName);  
}


/// @brief initialises field information
void FillerMSSink::initFields()
{
}
  
/// @brief initialises spectral and polarisation info (data descriptor)
void FillerMSSink::initDataDesc()
{
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


} // namespace swcorrelator

} // namespace askap
