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
#include "ingestutils/ParsetConfiguration.h"
#include "ingestutils/AntennaPositions.h"

ASKAP_LOGGER(logger, ".MSSink");

using namespace askap;
using namespace askap::cp::common;
using namespace askap::cp::ingest;
using namespace casa;

//////////////////////////////////
// Public methods
//////////////////////////////////

MSSink::MSSink(const LOFAR::ParameterSet& parset) :
    itsParset(parset)
{
    ASKAPLOG_DEBUG_STR(logger, "Constructor");
    const LOFAR::ParameterSet configSubset = parset.makeSubset("config.");
    itsConfig.reset(new ParsetConfiguration(configSubset));
    create();
    initAntennas();
    initFeeds();
    initSpws();
    initFields();
    initObs();
}

MSSink::~MSSink()
{
    ASKAPLOG_DEBUG_STR(logger, "Destructor");
    itsMs.reset();
    itsConfig.reset();
}

void MSSink::process(VisChunk::ShPtr chunk)
{
    ASKAPLOG_DEBUG_STR(logger, "process()");

    MSColumns msc(*itsMs);
    const casa::uInt baseRow = msc.nrow();
    const casa::uInt newRows = chunk->nRow();
    itsMs->addRow(newRows);

    // First set the constant things outside the loop,
    // as they apply to all rows
    msc.scanNumber().put(baseRow, 0);
    msc.fieldId().put(baseRow, 0);
    msc.dataDescId().put(baseRow, 0);

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

    const casa::Double Tend = Tmid - (Tint / 2);
    timeRange(1) = Tend;
    obsc.timeRange().put(0, timeRange);
}

//////////////////////////////////
// Private methods
//////////////////////////////////

void MSSink::create(void)
{
    // Get configuration first to ensure all parameters are present
    casa::uInt bucketSize = itsParset.getUint32("stman.bucketsize", 1024 * 1024);
    casa::uInt tileNcorr = itsParset.getUint32("stman.tilencorr", 4);
    casa::uInt tileNchan = itsParset.getUint32("stman.tilenchan", 1);
    const std::string filenamebase = itsParset.getString("filenamebase");

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); 
    std::ostringstream ss;
    ss << filenamebase << rank << ".ms";
    const std::string filename = ss.str();

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
        info.setSubType(String("simulator"));
        info.readmeAddLine("This is a MeasurementSet Table holding simulated astronomical observations");
    }
}

void MSSink::initAntennas(void)
{
    std::string station;
    casa::Vector<std::string> name;
    casa::Matrix<double> antXYZ;
    casa::Matrix<double> offset;
    casa::Vector<casa::Double> dishDiameter;
    casa::Vector<std::string> mount;

    itsConfig->getAntennas(name, station, antXYZ, offset, dishDiameter, mount);
    casa::uInt nAnt = name.size();

    // Write the rows to the measurement set
    MSColumns msc(*itsMs);
    MSAntennaColumns& antc = msc.antenna();
    ASKAPCHECK(antc.nrow() == 0, "Antenna table already has data");

    MSAntenna& ant = itsMs->antenna();
    ant.addRow(nAnt);

    antc.type().fillColumn("GROUND-BASED");
    antc.station().fillColumn(station);
    antc.flagRow().fillColumn(false);
    antc.position().putColumn(antXYZ);

    for (unsigned int row = 0; row < nAnt; ++row) {
        antc.name().put(row, name(row));
        antc.mount().put(row, mount(row));
        antc.dishDiameter().put(row, dishDiameter(row));
    }
}

void MSSink::initFeeds(void)
{
    casa::String mode;
    casa::Vector<double> x;
    casa::Vector<double> y;
    casa::Vector<casa::String> pol;

    itsConfig->getFeeds(mode, x, y, pol);

    MSColumns msc(*itsMs);
    MSAntennaColumns& antc = msc.antenna();
    const uInt nAnt = antc.nrow();

    if (nAnt <= 0) {
        ASKAPLOG_INFO_STR(logger, "initFeeds: must call initAntenna() first");
    }

    uInt nFeed = x.nelements();

    String feedPol0 = "R", feedPol1 = "L";
    Bool isList = False;

    if (nFeed > 0) {
        isList = True;

        if (x.nelements() != y.nelements()) {
            ASKAPLOG_ERROR_STR(logger, "Feed x and y must be the same length");
        }

        ASKAPCHECK(pol.nelements() == x.nelements(),
                   "Feed polarization list must be same length as the number of positions");
    } else {
        nFeed = 1;

        // mode == "perfect R L" OR "perfect X Y"
        if (mode.contains("X", 0)) {
            feedPol0 = "X";
            feedPol1 = "Y";
        }
    }

    const uInt nRow = nFeed * nAnt;
    Vector<Int> feedAntId(nRow);
    Vector<Int> feedId(nRow);
    Vector<Int> feedSpWId(nRow);
    Vector<Int> feedBeamId(nRow);

    Vector<Int> feedNumRec(nRow);
    Cube<double> beamOffset(2, 2, nRow);

    Matrix<String> feedPol(2, nRow);
    Matrix<double> feedXYZ(3, nRow);
    Matrix<double> feedAngle(2, nRow);
    Cube<Complex> polResp(2, 2, nRow);

    Int iRow = 0;

    if (isList) {
        polResp = Complex(0.0, 0.0);

        for (uInt i = 0; i < nAnt; i++) {
            for (uInt j = 0; j < nFeed; j++) {
                feedAntId(iRow) = i;
                feedId(iRow) = j;
                feedSpWId(iRow) = -1;
                feedBeamId(iRow) = 0;
                feedNumRec(iRow) = 2;
                beamOffset(0, 0, iRow) = x(j);
                beamOffset(1, 0, iRow) = y(j);
                beamOffset(0, 1, iRow) = x(j);
                beamOffset(1, 1, iRow) = y(j);
                feedXYZ(0, iRow) = 0.0;
                feedXYZ(1, iRow) = 0.0;
                feedXYZ(2, iRow) = 0.0;
                feedAngle(0, iRow) = 0.0;
                feedAngle(1, iRow) = 0.0;

                if (pol(j).contains("X", 0)) {
                    feedPol(0, iRow) = "X";
                    feedPol(1, iRow) = "Y";
                } else {
                    feedPol(0, iRow) = "L";
                    feedPol(1, iRow) = "R";
                }

                polResp(0, 0, iRow) = polResp(1, 1, iRow) = Complex(1.0, 0.0);
                iRow++;
            }
        }
    } else {
        polResp = Complex(0.0, 0.0);

        for (uInt i = 0; i < nAnt; i++) {
            feedAntId(iRow) = i;
            feedId(iRow) = 0;
            feedSpWId(iRow) = -1;
            feedBeamId(iRow) = 0;
            feedNumRec(iRow) = 2;
            beamOffset(0, 0, iRow) = 0.0;
            beamOffset(1, 0, iRow) = 0.0;
            beamOffset(0, 1, iRow) = 0.0;
            beamOffset(1, 1, iRow) = 0.0;
            feedXYZ(0, iRow) = 0.0;
            feedXYZ(1, iRow) = 0.0;
            feedXYZ(2, iRow) = 0.0;
            feedAngle(0, iRow) = 0.0;
            feedAngle(1, iRow) = 0.0;
            feedPol(0, iRow) = feedPol0;
            feedPol(1, iRow) = feedPol1;
            polResp(0, 0, iRow) = polResp(1, 1, iRow) = Complex(1.0, 0.0);
            iRow++;
        }
    }

    // fill Feed table - don't check to see if any of the positions match
    MSFeedColumns& feedc = msc.feed();
    const uInt numFeeds = feedc.nrow();
    Slicer feedSlice(IPosition(1, numFeeds), IPosition(1, nRow + numFeeds - 1),
                     IPosition(1, 1), Slicer::endIsLast);
    itsMs->feed().addRow(nRow);
    feedc.antennaId().putColumnRange(feedSlice, feedAntId);
    feedc.feedId().putColumnRange(feedSlice, feedId);
    feedc.spectralWindowId().putColumnRange(feedSlice, feedSpWId);
    feedc.beamId().putColumnRange(feedSlice, feedBeamId);
    feedc.numReceptors().putColumnRange(feedSlice, feedNumRec);
    feedc.position().putColumnRange(feedSlice, feedXYZ);
    const double forever = 1.e30;

    for (uInt i = numFeeds; i < (nRow + numFeeds); i++) {
        feedc.beamOffset().put(i, beamOffset.xyPlane(i - numFeeds));
        feedc.polarizationType().put(i, feedPol.column(i - numFeeds));
        feedc.polResponse().put(i, polResp.xyPlane(i - numFeeds));
        feedc.receptorAngle().put(i, feedAngle.column(i - numFeeds));
        feedc.time().put(i, 0.0);
        feedc.interval().put(i, forever);
    }
}

void MSSink::initSpws(void)
{
    casa::String spWindowName;
    int nChan;
    casa::Quantity startFreq;
    casa::Quantity freqInc;
    casa::String stokesString;

    itsConfig->getSpWindows(spWindowName, nChan, startFreq, freqInc, stokesString);

    Vector<Int> stokesTypes(4);
    stokesTypes = Stokes::Undefined;
    String myStokesString = stokesString;
    Int nCorr = 0;

    for (Int j = 0; j < 4; j++) {
        while (myStokesString.at(0, 1) == " ") {
            myStokesString.del(0, 1);
        }

        if (myStokesString.length() == 0)
            break;

        stokesTypes(j) = Stokes::type(myStokesString.at(0, 2));
        myStokesString.del(0, 2);
        nCorr = j + 1;

        if (stokesTypes(j) == Stokes::Undefined) {
            ASKAPLOG_INFO_STR(logger, " Undefined polarization type in input");
        }
    }

    MSColumns msc(*itsMs);
    MSSpWindowColumns& spwc = msc.spectralWindow();
    MSDataDescColumns& ddc = msc.dataDescription();
    MSPolarizationColumns& polc = msc.polarization();
    const uInt baseSpWID = spwc.nrow();
    ASKAPLOG_INFO_STR(logger, "Creating new spectral window " << spWindowName << ", ID "
                          << baseSpWID + 1);
    // fill spectralWindow table
    itsMs->spectralWindow().addRow(1);
    itsMs->polarization().addRow(1);
    itsMs->dataDescription().addRow(1);
    spwc.numChan().put(baseSpWID, nChan);
    spwc.name().put(baseSpWID, spWindowName);
    spwc.netSideband().fillColumn(1);
    spwc.ifConvChain().fillColumn(0);
    spwc.freqGroup().fillColumn(0);
    spwc.freqGroupName().fillColumn("Group 1");
    spwc.flagRow().fillColumn(False);
    spwc.measFreqRef().fillColumn(MFrequency::TOPO);
    polc.flagRow().fillColumn(False);
    ddc.flagRow().fillColumn(False);
    polc.numCorr().put(baseSpWID, nCorr);
    Vector <double> freqs(nChan), bandwidth(nChan);
    bandwidth = freqInc.getValue("Hz");
    ddc.spectralWindowId().put(baseSpWID, baseSpWID);
    ddc.polarizationId().put(baseSpWID, baseSpWID);
    double vStartFreq(startFreq.getValue("Hz"));
    double vFreqInc(freqInc.getValue("Hz"));

    for (Int chan = 0; chan < nChan; chan++) {
        freqs(chan) = vStartFreq + chan * vFreqInc;
    }

    // translate stokesTypes into receptor products, catch invalid
    // fallibles.
    Matrix<Int> corrProduct(uInt(2), uInt(nCorr));
    Fallible<Int> fi;
    stokesTypes.resize(nCorr, True);

    for (Int j = 0; j < nCorr; j++) {
        fi = Stokes::receptor1(Stokes::type(stokesTypes(j)));
        corrProduct(0, j) = (fi.isValid() ? fi.value() : 0);
        fi = Stokes::receptor2(Stokes::type(stokesTypes(j)));
        corrProduct(1, j) = (fi.isValid() ? fi.value() : 0);
    }

    spwc.refFrequency().put(baseSpWID, vStartFreq);
    spwc.chanFreq().put(baseSpWID, freqs);
    spwc.chanWidth().put(baseSpWID, bandwidth);
    spwc.effectiveBW().put(baseSpWID, bandwidth);
    spwc.resolution().put(baseSpWID, bandwidth);
    spwc.totalBandwidth().put(baseSpWID, nChan*vFreqInc);
    polc.corrType().put(baseSpWID, stokesTypes);
    polc.corrProduct().put(baseSpWID, corrProduct);
}

void MSSink::initFields(void)
{
    casa::String fieldName;
    casa::MDirection fieldDirection;
    casa::String calCode;

    itsConfig->getFields(fieldName, fieldDirection, calCode);

    MSColumns msc(*itsMs);
    MSFieldColumns& fieldc = msc.field();
    const uInt baseFieldID = fieldc.nrow();

    ASKAPLOG_INFO_STR(logger, "Creating new field " << fieldName << ", ID " << baseFieldID
            + 1);

    itsMs->field().addRow(1);
    fieldc.name().put(baseFieldID, fieldName);
    fieldc.code().put(baseFieldID, calCode);
    fieldc.time().put(baseFieldID, 0.0);
    fieldc.numPoly().put(baseFieldID, 0);
    fieldc.sourceId().put(baseFieldID, 0);
    Vector<MDirection> direction(1);
    direction(0) = fieldDirection;
    fieldc.delayDirMeasCol().put(baseFieldID, direction);
    fieldc.phaseDirMeasCol().put(baseFieldID, direction);
    fieldc.referenceDirMeasCol().put(baseFieldID, direction);
}

void MSSink::initObs(void)
{
    MSColumns msc(*itsMs);
    MSObservation& obs = itsMs->observation();
    MSObservationColumns& obsc = msc.observation();
    const uInt nobsrow = obsc.nrow();
    obs.addRow();
    obsc.telescopeName().put(nobsrow, "ASKAP");
    Vector<double> timeRange(2);
    timeRange(0) = 0;
    timeRange(1) = 0;
    obsc.timeRange().put(nobsrow, timeRange);
    obsc.observer().put(nobsrow, "");
}
