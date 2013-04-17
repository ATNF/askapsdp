/// @file MSSink.cc
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
#include "MSSink.h"

// Include package level header file
#include "askap_cpingest.h"

// System includes
#include <string>
#include <sstream>
#include <limits>
#include <mpi.h>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "cpcommon/VisChunk.h"

// Casecore includes
#include "casa/aips.h"
#include "casa/Quanta.h"
#include "casa/Arrays/Vector.h"
#include "casa/Arrays/Matrix.h"
#include "casa/Arrays/Cube.h"
#include "casa/Arrays/MatrixMath.h"
#include "tables/Tables/TableDesc.h"
#include "tables/Tables/SetupNewTab.h"
#include "tables/Tables/IncrementalStMan.h"
#include "tables/Tables/StandardStMan.h"
#include "tables/Tables/TiledShapeStMan.h"
#include "ms/MeasurementSets/MeasurementSet.h"
#include "ms/MeasurementSets/MSColumns.h"

// Local package includes
#include "configuration/Configuration.h" // Includes all configuration attributes too

ASKAP_LOGGER(logger, ".MSSink");

using namespace askap;
using namespace askap::cp::common;
using namespace askap::cp::ingest;
using namespace casa;

//////////////////////////////////
// Public methods
//////////////////////////////////

MSSink::MSSink(const LOFAR::ParameterSet& parset,
        const Configuration& config) :
    itsParset(parset), itsConfig(config), itsPreviousScanIndex(-1),
    itsFieldRow(-1), itsDataDescRow(-1)
{
    ASKAPLOG_DEBUG_STR(logger, "Constructor");
    create();
    initAntennas(); // Includes FEED table
    initObs();
}

MSSink::~MSSink()
{
    ASKAPLOG_DEBUG_STR(logger, "Destructor");
    itsMs.reset();
}

void MSSink::process(VisChunk::ShPtr chunk)
{
    ASKAPLOG_DEBUG_STR(logger, "process()");

    // Handle the details for when a new scan starts
    if (itsPreviousScanIndex != static_cast<casa::Int>(chunk->scan())) {
        itsFieldRow = findOrAddField(chunk->scan());
        itsDataDescRow = findOrAddDataDesc(chunk);
        itsPreviousScanIndex = chunk->scan();
    }

    MSColumns msc(*itsMs);
    const casa::uInt baseRow = msc.nrow();
    const casa::uInt newRows = chunk->nRow();
    itsMs->addRow(newRows);

    // First set the constant things outside the loop,
    // as they apply to all rows
    msc.scanNumber().put(baseRow, chunk->scan());
    msc.fieldId().put(baseRow, itsFieldRow);
    msc.dataDescId().put(baseRow, itsDataDescRow);

    const casa::Quantity chunkMidpoint = chunk->time().getTime();
    msc.time().put(baseRow, chunkMidpoint.getValue("s"));
    msc.time().put(baseRow, chunkMidpoint.getValue("s"));
    msc.timeCentroid().put(baseRow, chunkMidpoint.getValue("s"));

    msc.arrayId().put(baseRow, 0);
    msc.processorId().put(baseRow, 0);
    msc.exposure().put(baseRow, chunk->interval());
    msc.interval().put(baseRow, chunk->interval());
    msc.observationId().put(baseRow, 0);
    msc.stateId().put(baseRow, -1);

    for (casa::uInt i = 0; i < newRows; ++i) {
        const casa::uInt row = i + baseRow;
        msc.antenna1().put(row, chunk->antenna1()(i));
        msc.antenna2().put(row, chunk->antenna2()(i));
        msc.feed1().put(row, chunk->beam1()(i));
        msc.feed2().put(row, chunk->beam2()(i));
        msc.uvw().put(row, chunk->uvw()(i).vector());

        msc.data().put(row, casa::transpose(chunk->visibility().yzPlane(i)));
        msc.flag().put(row, casa::transpose(chunk->flag().yzPlane(i)));
        msc.flagRow().put(row, False);

        // TODO: Need to get this data from somewhere
        const Vector<Float> tmp(chunk->nPol(), 1.0);
        msc.weight().put(row, tmp);
        msc.sigma().put(row, tmp);
    }

    //
    // Update the observation table
    //
    // If this is the first integration cycle update the start time,
    // otherwise just update the end time.
    const casa::Double Tmid = chunkMidpoint.getValue("s");
    const casa::Double Tint = chunk->interval();

    MSObservationColumns& obsc = msc.observation();
    casa::Vector<casa::Double> timeRange = obsc.timeRange()(0);
    if (timeRange(0) == 0) {
        const casa::Double Tstart = Tmid - (Tint / 2);
        timeRange(0) = Tstart; 
    }

    const casa::Double Tend = Tmid + (Tint / 2);
    timeRange(1) = Tend;
    obsc.timeRange().put(0, timeRange);

    itsMs->flush();
}

//////////////////////////////////
// Private methods
//////////////////////////////////

void MSSink::create(void)
{
    // Get configuration first to ensure all parameters are present
    casa::uInt bucketSize = itsParset.getUint32("stman.bucketsize", 128 * 1024);
    casa::uInt tileNcorr = itsParset.getUint32("stman.tilencorr", 4);
    casa::uInt tileNchan = itsParset.getUint32("stman.tilenchan", 1);
    const casa::String filenamebase = itsParset.getString("filenamebase");

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); 
    std::ostringstream ss;
    ss << filenamebase << rank << ".ms";
    const casa::String filename = ss.str();

    if (bucketSize < 8192) {
        bucketSize = 8192;
    }
    if (tileNcorr < 1) {
        tileNcorr = 1;
    }
    if (tileNchan < 1) {
        tileNchan = 1;
    }

    ASKAPLOG_DEBUG_STR(logger, "Creating dataset " << filename);

    // Make MS with standard columns
    TableDesc msDesc(MS::requiredTableDesc());

    // Add the DATA column.
    MS::addColumnToDesc(msDesc, MS::DATA, 2);

    SetupNewTable newMS(filename, msDesc, Table::New);

    // Set the default Storage Manager to be the Incr one
    {
        IncrementalStMan incrStMan("ismdata", bucketSize);
        newMS.bindAll(incrStMan, True);
    }

    // Bind ANTENNA1, and ANTENNA2 to the standardStMan 
    // as they may change sufficiently frequently to make the
    // incremental storage manager inefficient for these columns.

    {
        StandardStMan ssm("ssmdata", bucketSize);
        newMS.bindColumn(MS::columnName(MS::ANTENNA1), ssm);
        newMS.bindColumn(MS::columnName(MS::ANTENNA2), ssm);
        newMS.bindColumn(MS::columnName(MS::UVW), ssm);
    }

    // These columns contain the bulk of the data so save them in a tiled way
    {
        // Get nr of rows in a tile.
        const int nrowTile = std::max(1u, bucketSize / (8*tileNcorr*tileNchan));
        TiledShapeStMan dataMan("TiledData",
                IPosition(3, tileNcorr, tileNchan, nrowTile));
        newMS.bindColumn(MeasurementSet::columnName(MeasurementSet::DATA),
                dataMan);
        newMS.bindColumn(MeasurementSet::columnName(MeasurementSet::FLAG),
                dataMan);
    }
    {
        const int nrowTile = std::max(1u, bucketSize / (4*8));
        TiledShapeStMan dataMan("TiledWeight",
                IPosition(2, 4, nrowTile));
        newMS.bindColumn(MeasurementSet::columnName(MeasurementSet::SIGMA),
                dataMan);
        newMS.bindColumn(MeasurementSet::columnName(MeasurementSet::WEIGHT),
                dataMan);
    }

    // Now we can create the MeasurementSet and add the (empty) subtables
    itsMs.reset(new MeasurementSet(newMS, 0));
    itsMs->createDefaultSubtables(Table::New);
    itsMs->flush();

    // Set the TableInfo
    {
        TableInfo& info(itsMs->tableInfo());
        info.setType(TableInfo::type(TableInfo::MEASUREMENTSET));
        info.setSubType(String(""));
        info.readmeAddLine("This is a MeasurementSet Table holding simulated astronomical observations");
    }
}

void MSSink::initAntennas(void)
{
    const std::vector<Antenna> antennas = itsConfig.antennas();
    std::vector<Antenna>::const_iterator it;
    for (it = antennas.begin(); it != antennas.end(); ++it) {
        casa::Int id = addAntenna(itsConfig.arrayName(),
                it->position(),
                it->name(),
                it->mount(),
                it->diameter().getValue("m"));

        // For each antenna one or more feed entries must be created
        const FeedConfig& feeds = it->feeds();
        initFeeds(feeds, id);
    }
}

void MSSink::initFeeds(const FeedConfig& feeds, const casa::Int antennaID)
{
    const uInt nFeeds = feeds.nFeeds();

    casa::Vector<double> x(nFeeds);
    casa::Vector<double> y(nFeeds);
    casa::Vector<casa::String> pol(nFeeds);

    for (size_t i = 0; i < nFeeds; ++i) {
        x(i) = feeds.offsetX(i).getValue("rad");
        y(i) = feeds.offsetY(i).getValue("rad");
        pol(i) = "X Y";
    }

    addFeeds(antennaID, x, y, pol);
}

void MSSink::initObs(void)
{
    addObs("ASKAP", "", 0, 0);
}

casa::Int MSSink::addObs(const casa::String& telescope, 
        const casa::String& observer,
        const double obsStartTime,
        const double obsEndTime)
{
    MSColumns msc(*itsMs);
    MSObservation& obs = itsMs->observation();
    MSObservationColumns& obsc = msc.observation();
    const uInt row = obsc.nrow();
    obs.addRow();
    obsc.telescopeName().put(row, telescope);
    Vector<double> timeRange(2);
    timeRange(0) = obsStartTime;
    timeRange(1) = obsEndTime;
    obsc.timeRange().put(row, timeRange);
    obsc.observer().put(row, observer);

    // Post-conditions
    ASKAPCHECK(obsc.nrow() == (row + 1), "Unexpected observation row count");

    return row;
}

casa::Int MSSink::addField(const casa::String& fieldName,
        const casa::MDirection& fieldDirection,
        const casa::String& calCode)
{
    MSColumns msc(*itsMs);
    MSFieldColumns& fieldc = msc.field();
    const uInt row = fieldc.nrow();

    ASKAPLOG_INFO_STR(logger, "Creating new field " << fieldName << ", ID "
            << row);

    itsMs->field().addRow();
    fieldc.name().put(row, fieldName);
    fieldc.code().put(row, calCode);
    fieldc.time().put(row, 0.0);
    fieldc.numPoly().put(row, 0);
    fieldc.sourceId().put(row, 0);
    Vector<MDirection> direction(1);
    direction(0) = fieldDirection;
    fieldc.delayDirMeasCol().put(row, direction);
    fieldc.phaseDirMeasCol().put(row, direction);
    fieldc.referenceDirMeasCol().put(row, direction);

    // Post-conditions
    ASKAPCHECK(fieldc.nrow() == (row + 1), "Unexpected field row count");

    return row;
}

void MSSink::addFeeds(const casa::Int antennaID,
        const casa::Vector<double>& x,
        const casa::Vector<double>& y,
        const casa::Vector<casa::String>& polType)
{
    // Pre-conditions
    const uInt nFeeds = x.size();
    ASKAPCHECK(nFeeds == y.size(), "X and Y vectors must be of equal length");
    ASKAPCHECK(nFeeds == polType.size(),
            "Pol type vector must have the same length as X and Y");

    // Add to the Feed table
    MSColumns msc(*itsMs);
    MSFeedColumns& feedc = msc.feed();
    const uInt startRow = feedc.nrow();
    itsMs->feed().addRow(nFeeds);

    for (casa::uInt i = 0; i < nFeeds; ++i) {
        casa::uInt row = startRow + i;
        feedc.antennaId().put(row, antennaID);
        feedc.feedId().put(row, i);
        feedc.spectralWindowId().put(row, -1);
        feedc.beamId().put(row, 0);
        feedc.numReceptors().put(row, 2);

        // Feed position
        Vector<double> feedXYZ(3, 0.0);
        feedc.position().put(row, feedXYZ);

        // Beam offset
        Matrix<double> beamOffset(2, 2);
        beamOffset(0, 0) = x(i);
        beamOffset(1, 0) = y(i);
        beamOffset(0, 1) = x(i);
        beamOffset(1, 1) = y(i);
        feedc.beamOffset().put(row, beamOffset);

        // Polarisation type
        Vector<String> feedPol(2);
        if (polType(i).contains("X", 0)) {
            feedPol(0) = "X";
            feedPol(1) = "Y";
        } else {
            feedPol(0) = "L";
            feedPol(1) = "R";
        }
        feedc.polarizationType().put(row, feedPol);

        // Polarisation response
        Matrix<Complex> polResp(2, 2);
        polResp = Complex(0.0, 0.0);
        polResp(1, 1) = Complex(1.0, 0.0);
        polResp(0, 0) = Complex(1.0, 0.0);
        feedc.polResponse().put(row, polResp);

        // Receptor angle
        Vector<double> feedAngle(2, 0.0);
        feedc.receptorAngle().put(row, feedAngle);

        // Time
        feedc.time().put(row, 0.0);

        // Interval - 1.e30 is effectivly forever
        feedc.interval().put(row, 1.e30);
    };

    // Post-conditions
    ASKAPCHECK(feedc.nrow() == (startRow + nFeeds), "Unexpected feed row count");
}

casa::Int MSSink::addAntenna(const casa::String& station,
        const casa::Vector<double>& antXYZ,
        const casa::String& name,
        const casa::String& mount,
        const casa::Double& dishDiameter)
{
    // Pre-conditions
    ASKAPCHECK(antXYZ.size() == 3, "Antenna position vector must contain 3 elements");

    // Write the rows to the measurement set
    MSColumns msc(*itsMs);
    MSAntennaColumns& antc = msc.antenna();
    const uInt row = antc.nrow();

    MSAntenna& ant = itsMs->antenna();
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

casa::Int MSSink::addDataDesc(const casa::Int spwId, const casa::Int polId)
{
    // 1: Add new row and determine its offset
    MSColumns msc(*itsMs);
    MSDataDescColumns& ddc = msc.dataDescription();
    const uInt row = ddc.nrow();
    itsMs->dataDescription().addRow();

    // 2: Populate DATA DESCRIPTION table
    ddc.flagRow().put(row, False);
    ddc.spectralWindowId().put(row, spwId);
    ddc.polarizationId().put(row, polId);

    return row;
}

/// @note The implementation of method isSpectralWindowRowEqual() is tightly
/// coupled to the implementation of this method. If this method is changed
/// it is likely isSpectralWindowRowEqual() should be too.
casa::Int MSSink::addSpectralWindow(const casa::String& spwName,
            const int nChan,
            const casa::Quantity& startFreq,
            const casa::Quantity& freqInc)
{
    MSColumns msc(*itsMs);
    MSSpWindowColumns& spwc = msc.spectralWindow();
    const uInt row = spwc.nrow();
    ASKAPLOG_INFO_STR(logger, "Creating new spectral window " << spwName
            << ", ID " << row);

    itsMs->spectralWindow().addRow();

    spwc.numChan().put(row, nChan);
    spwc.name().put(row, spwName);
    spwc.netSideband().put(row, 1);
    spwc.ifConvChain().put(row, 0);
    spwc.freqGroup().put(row, 0);
    spwc.freqGroupName().put(row, "Group 1");
    spwc.flagRow().put(row, False);
    spwc.measFreqRef().put(row, MFrequency::TOPO);

    Vector<double> freqs(nChan);
    Vector<double> bandwidth(nChan, freqInc.getValue("Hz"));

    double vStartFreq(startFreq.getValue("Hz"));
    double vFreqInc(freqInc.getValue("Hz"));

    for (Int chan = 0; chan < nChan; chan++) {
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

/// @note The implementation of method isPolarisationRowEqual() is tightly
/// coupled to the implementation of this method. If this method is changed
/// it is likely isPolarisationRowEqual() should be too.
casa::Int MSSink::addPolarisation(const casa::Vector<casa::Stokes::StokesTypes>& stokesTypes)
{
    const Int nCorr = stokesTypes.size();

    MSColumns msc(*itsMs);
    MSPolarizationColumns& polc = msc.polarization();
    const uInt row = polc.nrow();
    itsMs->polarization().addRow();

    polc.flagRow().put(row, False);
    polc.numCorr().put(row, nCorr);

    // Translate stokesTypes into receptor products, catch invalid
    // fallibles.
    Matrix<Int> corrProduct(uInt(2), uInt(nCorr));
    Fallible<Int> fi;

    casa::Vector<casa::Int> stokesTypesInt(nCorr);
    for (Int i = 0; i < nCorr; i++) {
        fi = Stokes::receptor1(stokesTypes(i));
        corrProduct(0, i) = (fi.isValid() ? fi.value() : 0);
        fi = Stokes::receptor2(stokesTypes(i));
        corrProduct(1, i) = (fi.isValid() ? fi.value() : 0);
        stokesTypesInt(i) = stokesTypes(i);
    }

    polc.corrType().put(row, stokesTypesInt);
    polc.corrProduct().put(row, corrProduct);

    return row;
}

casa::Int MSSink::findOrAddField(const casa::Int scanId)
{
    const Scan scan = itsConfig.observation().scans().at(scanId);
    const casa::String fieldName = scan.name();
    const casa::MDirection fieldDirection = scan.fieldDirection();
    const casa::String& calCode = "";

    MSColumns msc(*itsMs);
    ROMSFieldColumns& fieldc = msc.field();
    const uInt nRows = fieldc.nrow();

    for (uInt i = 0; i < nRows; ++i) {
        const Vector<MDirection> dirVec = fieldc.referenceDirMeasCol()(i);
        if ((fieldName.compare(fieldc.name()(i)) == 0)
                && (calCode.compare(fieldc.code()(i)) == 0)
                && equal(dirVec[0],  fieldDirection)) {
            return i;
        }
    }

    return addField(fieldName, fieldDirection, calCode);
}

casa::Int MSSink::findOrAddDataDesc(askap::cp::common::VisChunk::ShPtr chunk)
{
   casa::Int spwId;
   casa::Int polId;

   // 1: Try to find a data description that matches the scan
   MSColumns msc(*itsMs);
   ROMSDataDescColumns& ddc = msc.dataDescription();
   uInt nRows = ddc.nrow();
   for (uInt row = 0; row < nRows; ++row) {
       spwId = ddc.spectralWindowId()(row);
       polId = ddc.polarizationId()(row);
       if (isSpectralWindowRowEqual(chunk, spwId) &&
               isPolarisationRowEqual(chunk, polId)) {
           return row;
       }
   }

   // The value -1 indicates an entry that matches the scan has not
   // been found.
   spwId = -1;
   polId = -1;

   // 2: Try to find a spectral window row that matches
   nRows = msc.spectralWindow().nrow();
   for (uInt row = 0; row < nRows; ++row) {
       if (isSpectralWindowRowEqual(chunk, row)) {
           spwId = row;
           break;
       }
   }


   // 3: Try to find a polarisation row that matches
   nRows = msc.polarization().nrow();
   for (uInt row = 0; row < nRows; ++row) {
       if (isPolarisationRowEqual(chunk, row)) {
           polId = row;
           break;
       }
   }

   // 4: Create the missing entry and a data desc
   if (spwId == -1) {
       const casa::String spWindowName("NO_NAME"); // TODO: Add name
       spwId = addSpectralWindow(spWindowName,
               chunk->nChannel(),
               casa::Quantity(chunk->frequency()(0), "Hz"),
               casa::Quantity(chunk->channelWidth(), "Hz"));
   }
   if (polId == -1) {
       polId = addPolarisation(chunk->stokes());
   }

   return addDataDesc(spwId, polId);
}

// Compares the given row in the spectral window table with the spectral window
// setup as defined in the Scan.
//
// @note This is not an apples to apples comparison, and depends somewhat on
// how the infomration in the "Scan" object was translated to a spectral
// window setup. For this reason, the implementation of this method is
// tightly coupled to the addSpectralWindow() method in this class. If that
// method is modified, so should this.
//
// @return true if the two are effectivly equal, otherwise false.
bool MSSink::isSpectralWindowRowEqual(askap::cp::common::VisChunk::ShPtr chunk,
        const casa::uInt row) const
{
    MSColumns msc(*itsMs);
    ROMSSpWindowColumns& spwc = msc.spectralWindow();
    ASKAPCHECK(row < spwc.nrow(), "Row index out of bounds");

    if (spwc.numChan()(row) != static_cast<casa::Int>(chunk->nChannel())) {
        return false;
    }
    if (spwc.flagRow()(row) != false) {
        return false;
    }
    const casa::Vector<double> freqs = spwc.chanFreq()(row);
    const double dblEpsilon = std::numeric_limits<double>::epsilon();
    if (fabs(freqs(0) - chunk->frequency()(0)) > dblEpsilon) {
        return false;
    }
    const casa::Vector<double> bandwidth = spwc.chanWidth()(row);
    if (fabs(bandwidth(0) - chunk->channelWidth()) > dblEpsilon) {
        return false;
    }

    return true;
}

// Compares the given row in the polarisation table with the polarisation
// setup as defined in the Scan.
//
// @note This is not an apples to apples comparison, and depends somewhat on
// how the infomration in the "Scan" object was translated to a spectral
// window setup. For this reason, the implementation of this method is
// tightly coupled to the addPolarisation() method in this class. If that
// method is modified, so should this.
//
// @return true if the two are effectivly equal, otherwise false.
bool MSSink::isPolarisationRowEqual(askap::cp::common::VisChunk::ShPtr chunk,
        const casa::uInt row) const
{
    MSColumns msc(*itsMs);
    ROMSPolarizationColumns& polc = msc.polarization();
    ASKAPCHECK(row < polc.nrow(), "Row index out of bounds");

    if (polc.numCorr()(row) != static_cast<casa::Int>(chunk->stokes().size())) {
        return false;
    }
    if (polc.flagRow()(row) != false) {
        return false;
    }
    casa::Vector<casa::Int> stokesTypesInt = polc.corrType()(row);
    for (casa::uInt i = 0; i < stokesTypesInt.size(); ++i) {
        if (stokesTypesInt(i) != chunk->stokes()(i)) {
            return false;
        }
    }

    return true;
}

bool MSSink::equal(const casa::MDirection &dir1, const casa::MDirection &dir2)
{
    if (dir1.getRef().getType() != dir2.getRef().getType()) {
        return false;
    }
    return dir1.getValue().separation(dir2.getValue()) < std::numeric_limits<double>::epsilon();
}
