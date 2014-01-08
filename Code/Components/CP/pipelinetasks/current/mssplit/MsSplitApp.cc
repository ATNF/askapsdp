/// @file MsSplitApp.cc
///
/// @copyright (c) 2012 CSIRO
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

// Package level header file
#include "askap_pipelinetasks.h"

// Include own header file
#include "mssplit/MsSplitApp.h"

// System includes
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <utility>
#include <algorithm>
#include <iterator>
#include <stdint.h>

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "askap/AskapLogging.h"
#include "askap/Application.h"
#include "askap/AskapUtil.h"
#include "askap/StatReporter.h"
#include "askap/Log4cxxLogSink.h"
#include "boost/shared_ptr.hpp"
#include "Common/ParameterSet.h"
#include "casa/OS/File.h"
#include "casa/aips.h"
#include "casa/Arrays/IPosition.h"
#include "casa/Arrays/Slicer.h"
#include "casa/Arrays/Array.h"
#include "casa/Arrays/Vector.h"
#include "casa/Arrays/Cube.h"
#include "tables/Tables/TableDesc.h"
#include "tables/Tables/SetupNewTab.h"
#include "tables/Tables/IncrementalStMan.h"
#include "tables/Tables/StandardStMan.h"
#include "tables/Tables/TiledShapeStMan.h"
#include "ms/MeasurementSets/MeasurementSet.h"
#include "ms/MeasurementSets/MSColumns.h"

// Local package includes
#include "mssplit/ParsetUtils.h"

ASKAP_LOGGER(logger, ".mssplitapp");

using namespace askap;
using namespace askap::cp::pipelinetasks;
using namespace casa;
using namespace std;

boost::shared_ptr<casa::MeasurementSet> MsSplitApp::create(
    const std::string& filename, casa::uInt bucketSize,
    casa::uInt tileNcorr, casa::uInt tileNchan)
{
    if (bucketSize < 8192) bucketSize = 8192;

    if (tileNcorr < 1) tileNcorr = 1;

    if (tileNchan < 1) tileNchan = 1;

    ASKAPLOG_INFO_STR(logger, "Creating dataset " << filename);

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
        // NOTE: The addition of the FEED columns here is a bit unusual.
        // While the FEED columns are perfect candidates for the incremental
        // storage manager, for some reason doing so results in a huge
        // increase in I/O to the file (see ticket: 4094 for details).
        StandardStMan ssm("ssmdata", bucketSize);
        newMS.bindColumn(MS::columnName(MS::ANTENNA1), ssm);
        newMS.bindColumn(MS::columnName(MS::ANTENNA2), ssm);
        newMS.bindColumn(MS::columnName(MS::FEED1), ssm);
        newMS.bindColumn(MS::columnName(MS::FEED2), ssm);
        newMS.bindColumn(MS::columnName(MS::UVW), ssm);
    }

    // These columns contain the bulk of the data so save them in a tiled way
    {
        // Get nr of rows in a tile.
        const int nrowTile = std::max(1u, bucketSize / (8 * tileNcorr * tileNchan));
        TiledShapeStMan dataMan("TiledData",
                                IPosition(3, tileNcorr, tileNchan, nrowTile));
        newMS.bindColumn(MeasurementSet::columnName(MeasurementSet::DATA),
                         dataMan);
        newMS.bindColumn(MeasurementSet::columnName(MeasurementSet::FLAG),
                         dataMan);
    }
    {
        const int nrowTile = std::max(1u, bucketSize / (4 * 8));
        TiledShapeStMan dataMan("TiledWeight",
                                IPosition(2, 4, nrowTile));
        newMS.bindColumn(MeasurementSet::columnName(MeasurementSet::SIGMA),
                         dataMan);
        newMS.bindColumn(MeasurementSet::columnName(MeasurementSet::WEIGHT),
                         dataMan);
    }

    // Now we can create the MeasurementSet and add the (empty) subtables
    boost::shared_ptr<casa::MeasurementSet> ms(new MeasurementSet(newMS, 0));
    ms->createDefaultSubtables(Table::New);
    ms->flush();

    // Set the TableInfo
    {
        TableInfo& info(ms->tableInfo());
        info.setType(TableInfo::type(TableInfo::MEASUREMENTSET));
        info.setSubType(String(""));
        info.readmeAddLine("This is a MeasurementSet Table holding simulated astronomical observations");
    }
    return ms;
}

void MsSplitApp::copyAntenna(const casa::MeasurementSet& source, casa::MeasurementSet& dest)
{
    const ROMSColumns srcMsc(source);
    const ROMSAntennaColumns& sc = srcMsc.antenna();

    MSColumns destMsc(dest);
    MSAntennaColumns& dc = destMsc.antenna();

    // Add new rows to the destination and copy the data
    dest.antenna().addRow(sc.nrow());

    dc.name().putColumn(sc.name());
    dc.station().putColumn(sc.station());
    dc.type().putColumn(sc.type());
    dc.mount().putColumn(sc.mount());
    dc.position().putColumn(sc.position());
    dc.dishDiameter().putColumn(sc.dishDiameter());
    dc.flagRow().putColumn(sc.flagRow());
}

void MsSplitApp::copyDataDescription(const casa::MeasurementSet& source, casa::MeasurementSet& dest)
{
    const ROMSColumns srcMsc(source);
    const ROMSDataDescColumns& sc = srcMsc.dataDescription();

    MSColumns destMsc(dest);
    MSDataDescColumns& dc = destMsc.dataDescription();

    // Add new rows to the destination and copy the data
    dest.dataDescription().addRow(sc.nrow());

    dc.flagRow().putColumn(sc.flagRow());
    dc.spectralWindowId().putColumn(sc.spectralWindowId());
    dc.polarizationId().putColumn(sc.polarizationId());
}

void MsSplitApp::copyFeed(const casa::MeasurementSet& source, casa::MeasurementSet& dest)
{
    const ROMSColumns srcMsc(source);
    const ROMSFeedColumns& sc = srcMsc.feed();

    MSColumns destMsc(dest);
    MSFeedColumns& dc = destMsc.feed();

    // Add new rows to the destination and copy the data
    dest.feed().addRow(sc.nrow());

    dc.antennaId().putColumn(sc.antennaId());
    dc.feedId().putColumn(sc.feedId());
    dc.spectralWindowId().putColumn(sc.spectralWindowId());
    dc.beamId().putColumn(sc.beamId());
    dc.numReceptors().putColumn(sc.numReceptors());
    dc.position().putColumn(sc.position());
    dc.beamOffset().putColumn(sc.beamOffset());
    dc.polarizationType().putColumn(sc.polarizationType());
    dc.polResponse().putColumn(sc.polResponse());
    dc.receptorAngle().putColumn(sc.receptorAngle());
    dc.time().putColumn(sc.time());
    dc.interval().putColumn(sc.interval());
}

void MsSplitApp::copyField(const casa::MeasurementSet& source, casa::MeasurementSet& dest)
{
    const ROMSColumns srcMsc(source);
    const ROMSFieldColumns& sc = srcMsc.field();

    MSColumns destMsc(dest);
    MSFieldColumns& dc = destMsc.field();

    // Add new rows to the destination and copy the data
    dest.field().addRow(sc.nrow());

    dc.name().putColumn(sc.name());
    dc.code().putColumn(sc.code());
    dc.time().putColumn(sc.time());
    dc.numPoly().putColumn(sc.numPoly());
    dc.sourceId().putColumn(sc.sourceId());
    dc.delayDir().putColumn(sc.delayDir());
    dc.phaseDir().putColumn(sc.phaseDir());
    dc.referenceDir().putColumn(sc.referenceDir());
}

void MsSplitApp::copyObservation(const casa::MeasurementSet& source, casa::MeasurementSet& dest)
{
    const ROMSColumns srcMsc(source);
    const ROMSObservationColumns& sc = srcMsc.observation();

    MSColumns destMsc(dest);
    MSObservationColumns& dc = destMsc.observation();

    // Add new rows to the destination and copy the data
    dest.observation().addRow(sc.nrow());

    dc.timeRange().putColumn(sc.timeRange());
    //dc.log().putColumn(sc.log());
    //dc.schedule().putColumn(sc.schedule());
    dc.flagRow().putColumn(sc.flagRow());
    dc.observer().putColumn(sc.observer());
    dc.telescopeName().putColumn(sc.telescopeName());
    dc.project().putColumn(sc.project());
    dc.releaseDate().putColumn(sc.releaseDate());
    dc.scheduleType().putColumn(sc.scheduleType());
}

void MsSplitApp::copyPointing(const casa::MeasurementSet& source, casa::MeasurementSet& dest)
{
    const ROMSColumns srcMsc(source);
    const ROMSPointingColumns& sc = srcMsc.pointing();

    MSColumns destMsc(dest);
    MSPointingColumns& dc = destMsc.pointing();

    // Add new rows to the destination and copy the data
    dest.pointing().addRow(sc.nrow());

    // TODO: The first two left out because adding "target" hangs the split (or
    // at least gets it stuch in some long/infinite loop). Maybe need to handle
    // these MeasCol differently
    //dc.direction().putColumn(sc.direction());
    //dc.target().putColumn(sc.target());
    dc.antennaId().putColumn(sc.antennaId());
    dc.interval().putColumn(sc.interval());
    dc.name().putColumn(sc.name());
    dc.numPoly().putColumn(sc.numPoly());
    dc.time().putColumn(sc.time());
    dc.timeOrigin().putColumn(sc.timeOrigin());
    dc.tracking().putColumn(sc.tracking());
}

void MsSplitApp::copyPolarization(const casa::MeasurementSet& source, casa::MeasurementSet& dest)
{
    const ROMSColumns srcMsc(source);
    const ROMSPolarizationColumns& sc = srcMsc.polarization();

    MSColumns destMsc(dest);
    MSPolarizationColumns& dc = destMsc.polarization();

    // Add new rows to the destination and copy the data
    dest.polarization().addRow(sc.nrow());

    dc.flagRow().putColumn(sc.flagRow());
    dc.numCorr().putColumn(sc.numCorr());
    dc.corrType().putColumn(sc.corrType());
    dc.corrProduct().putColumn(sc.corrProduct());
}

void MsSplitApp::splitSpectralWindow(const casa::MeasurementSet& source,
                                     casa::MeasurementSet& dest,
                                     const uint32_t startChan,
                                     const uint32_t endChan,
                                     const uint32_t width)
{
    MSColumns destCols(dest);
    const ROMSColumns srcCols(source);

    MSSpWindowColumns& dc = destCols.spectralWindow();
    const ROMSSpWindowColumns& sc = srcCols.spectralWindow();
    const uInt nrows = sc.nrow();
    ASKAPCHECK(nrows == 1, "Only single spectral window is supported");
    dest.spectralWindow().addRow(nrows);

    // For each row
    for (uInt row = 0; row < nrows; ++row) {

        // 1: Copy over the simple cells (i.e. those not needing splitting/averaging)
        dc.measFreqRef().put(row, sc.measFreqRef()(row));
        dc.refFrequency().put(row, sc.refFrequency()(row));
        dc.flagRow().put(row, sc.flagRow()(row));
        dc.freqGroup().put(row, sc.freqGroup()(row));
        dc.freqGroupName().put(row, sc.freqGroupName()(row));
        dc.ifConvChain().put(row, sc.ifConvChain()(row));
        dc.name().put(row, sc.name()(row));
        dc.netSideband().put(row, sc.netSideband()(row));

        // 2: Now process each source measurement set, building up the arrays
        const uInt nChanIn = endChan - startChan + 1;
        const uInt nChanOut = nChanIn / width;
        vector<double> chanFreq;
        vector<double> chanWidth;
        vector<double> effectiveBW;
        vector<double> resolution;
        chanFreq.resize(nChanOut);
        chanWidth.resize(nChanOut);
        effectiveBW.resize(nChanOut);
        resolution.resize(nChanOut);
        double totalBandwidth = 0.0;

        for (uInt destChan = 0; destChan < nChanOut; ++destChan) {
            chanFreq[destChan] = 0.0;
            chanWidth[destChan] = 0.0;
            effectiveBW[destChan] = 0.0;
            resolution[destChan] = 0.0;

            // The offset for the first input channel for this destination channel
            const uInt chanOffset = startChan - 1 + (destChan * width);

            for (uInt i = chanOffset; i < chanOffset + width; ++i) {
                chanFreq[destChan] += sc.chanFreq()(row)(casa::IPosition(1, i));
                chanWidth[destChan] += sc.chanWidth()(row)(casa::IPosition(1, i));
                effectiveBW[destChan] += sc.effectiveBW()(row)(casa::IPosition(1, i));
                resolution[destChan] += sc.resolution()(row)(casa::IPosition(1, i));
                totalBandwidth += sc.chanWidth()(row)(casa::IPosition(1, i));
            }

            // Finally average chanFreq
            chanFreq[destChan] = chanFreq[destChan] / width;
        }

        // 3: Add those splitting/averaging cells
        dc.numChan().put(row, nChanOut);
        dc.chanFreq().put(row, casa::Vector<double>(chanFreq));
        dc.chanWidth().put(row, casa::Vector<double>(chanWidth));
        dc.effectiveBW().put(row, casa::Vector<double>(effectiveBW));
        dc.resolution().put(row, casa::Vector<double>(resolution));
        dc.totalBandwidth().put(row, totalBandwidth);

    } // End for rows
}

bool MsSplitApp::rowFiltersExist() const
{
    return !itsBeams.empty() || !itsScans.empty();
}

bool MsSplitApp::rowIsFiltered(uint32_t scanid, uint32_t feed1, uint32_t feed2) const
{
    // Include all rows if no filters exist
    if (!rowFiltersExist()) return false;

    if (!itsScans.empty() && itsScans.find(scanid) == itsScans.end()) return true;

    if (!itsBeams.empty() &&
            itsBeams.find(feed1) == itsBeams.end() &&
            itsBeams.find(feed2) == itsBeams.end()) {
        return true;
    }

    return false;
}

void MsSplitApp::splitMainTable(const casa::MeasurementSet& source,
                                casa::MeasurementSet& dest,
                                const uint32_t startChan,
                                const uint32_t endChan,
                                const uint32_t width)
{
    // Pre-conditions
    ASKAPDEBUGASSERT(endChan >= startChan);
    ASKAPDEBUGASSERT((endChan - startChan + 1) % width == 0);

    const ROMSColumns sc(source);
    MSColumns dc(dest);

    // Add rows upfront is no row based filters exist
    const casa::uInt nRows = sc.nrow();
    if (!rowFiltersExist()) dest.addRow(nRows);

    // Work out how many channels are to be actual input and which output
    // and how many polarisations are involved.
    const uInt nChanIn = endChan - startChan + 1;
    const uInt nChanOut = nChanIn / width;
    const uInt nPol = sc.data()(0).shape()(0);
    ASKAPDEBUGASSERT(nPol > 0);

    // Decide how many rows to process simultaneously. This needs to fit within
    // a reasonable amount of memory, because all visibilities will be read
    // in for possible averaging. Assumes 32MB working space.
    uInt maxSimultaneousRows = (32 * 1024 * 1024) / (nChanIn + nChanOut) / nPol
            / (sizeof(casa::Complex) + sizeof(casa::Bool));

    // However, if there is row-based filtering only one row can be copied
    // at a time.
    if (rowFiltersExist()) maxSimultaneousRows = 1;

    // Set a 64MB maximum cache size for the large columns
    const casa::uInt cacheSize = 64 * 1024 * 1024;
    sc.data().setMaximumCacheSize(cacheSize);
    dc.data().setMaximumCacheSize(cacheSize);
    sc.flag().setMaximumCacheSize(cacheSize);
    dc.flag().setMaximumCacheSize(cacheSize);

    uInt progressCounter = 0; // Used for progress reporting
    const uInt PROGRESS_INTERVAL_IN_ROWS = nRows / 100;

    // Row in destination table may differ from source table if row based
    // filtering is used
    uInt dstRow = 0;
    uInt row = 0;
    while (row < nRows) {
        // Number of rows to process for this iteration of the loop; either
        // maxSimultaneousRows or the remaining rows.
        const uInt nRowsThisIteration = min(maxSimultaneousRows, nRows - row);
        const Slicer srcrowslicer(IPosition(1, row), IPosition(1, nRowsThisIteration),
                Slicer::endIsLength);
        Slicer dstrowslicer = srcrowslicer;

        // Report progress at intervals and on completion
        progressCounter += nRowsThisIteration;
        if (progressCounter >= PROGRESS_INTERVAL_IN_ROWS ||
                (row >= nRows - 1)) {
            ASKAPLOG_INFO_STR(logger,  "Processed row " << row + 1 << " of " << nRows);
            progressCounter = 0;
        }
        
        // Debugging for chunk copying only
        if (nRowsThisIteration > 1) {
            ASKAPLOG_DEBUG_STR(logger,  "Processing " << nRowsThisIteration
                    << " rows this iteration");
        }

        // Skip this row if it is filtered out
        if (rowIsFiltered(sc.scanNumber()(row),
                    sc.feed1()(row),
                    sc.feed2()(row))) {
            row += nRowsThisIteration;
            continue;
        }

        // Rows have been pre-added if no row based filtering is done 
        if (rowFiltersExist()) {
            dest.addRow();
            dstrowslicer = Slicer(IPosition(1, dstRow), IPosition(1, nRowsThisIteration),
                    Slicer::endIsLength);
        }

        // Copy over the simple cells (i.e. those not needing averaging/merging)
        dc.scanNumber().putColumnRange(dstrowslicer, sc.scanNumber().getColumnRange(srcrowslicer));
        dc.fieldId().putColumnRange(dstrowslicer, sc.fieldId().getColumnRange(srcrowslicer));
        dc.dataDescId().putColumnRange(dstrowslicer, sc.dataDescId().getColumnRange(srcrowslicer));
        dc.time().putColumnRange(dstrowslicer, sc.time().getColumnRange(srcrowslicer));
        dc.timeCentroid().putColumnRange(dstrowslicer, sc.timeCentroid().getColumnRange(srcrowslicer));
        dc.arrayId().putColumnRange(dstrowslicer, sc.arrayId().getColumnRange(srcrowslicer));
        dc.processorId().putColumnRange(dstrowslicer, sc.processorId().getColumnRange(srcrowslicer));
        dc.exposure().putColumnRange(dstrowslicer, sc.exposure().getColumnRange(srcrowslicer));
        dc.interval().putColumnRange(dstrowslicer, sc.interval().getColumnRange(srcrowslicer));
        dc.observationId().putColumnRange(dstrowslicer, sc.observationId().getColumnRange(srcrowslicer));
        dc.antenna1().putColumnRange(dstrowslicer, sc.antenna1().getColumnRange(srcrowslicer));
        dc.antenna2().putColumnRange(dstrowslicer, sc.antenna2().getColumnRange(srcrowslicer));
        dc.feed1().putColumnRange(dstrowslicer, sc.feed1().getColumnRange(srcrowslicer));
        dc.feed2().putColumnRange(dstrowslicer, sc.feed2().getColumnRange(srcrowslicer));
        dc.uvw().putColumnRange(dstrowslicer, sc.uvw().getColumnRange(srcrowslicer));
        dc.flagRow().putColumnRange(dstrowslicer, sc.flagRow().getColumnRange(srcrowslicer));
        dc.weight().putColumnRange(dstrowslicer, sc.weight().getColumnRange(srcrowslicer));
        dc.sigma().putColumnRange(dstrowslicer, sc.sigma().getColumnRange(srcrowslicer));

        // Set the shape of the destination arrays
        for (uInt i = dstRow; i < dstRow + nRowsThisIteration; ++i) {
            dc.data().setShape(i, IPosition(2, nPol, nChanOut));
            dc.flag().setShape(i, IPosition(2, nPol, nChanOut));
        }

        //  Average (if applicable) then write data into the output MS
        const Slicer srcarrslicer(IPosition(2, 0, startChan - 1),
                                  IPosition(2, nPol, nChanIn), Slicer::endIsLength);
        const Slicer destarrslicer(IPosition(2, 0, 0),
                                   IPosition(2, nPol, nChanOut), Slicer::endIsLength);

        if (width == 1) {
            dc.data().putColumnRange(dstrowslicer, destarrslicer,
                                     sc.data().getColumnRange(srcrowslicer, srcarrslicer));
            dc.flag().putColumnRange(dstrowslicer, destarrslicer,
                                     sc.flag().getColumnRange(srcrowslicer, srcarrslicer));
        } else {
            // Get (read) the input data/flag
            const casa::Cube<casa::Complex> indata = sc.data().getColumnRange(srcrowslicer, srcarrslicer);
            const casa::Cube<casa::Bool> inflag = sc.flag().getColumnRange(srcrowslicer, srcarrslicer);

            // Create the output data/flag
            casa::Cube<casa::Complex> outdata(nPol, nChanOut, nRowsThisIteration);
            casa::Cube<casa::Bool> outflag(nPol, nChanOut, nRowsThisIteration);

            // Average data and combine flag information
            for (uInt pol = 0; pol < nPol; ++pol) {
                for (uInt destChan = 0; destChan < nChanOut; ++destChan) {
                    for (uInt r = 0; r < nRowsThisIteration; ++r) {
                        casa::Complex sum(0.0, 0.0);
                        casa::Bool outputFlag = false;

                        // Starting at the appropriate offset into the source data, average "width"
                        // channels together
                        for (uInt i = (destChan * width); i < (destChan * width) + width; ++i) {
                            ASKAPDEBUGASSERT(i < nChanIn);
                            sum += indata(pol, i, r);

                            if (outputFlag == false && inflag(pol, i, r)) {
                                outputFlag = true;
                            }
                        }

                        // Now the input channels have been averaged, write the data to
                        // the output cubes
                        outdata(pol, destChan, r) = casa::Complex(sum.real() / width,
                                                    sum.imag() / width);
                        outflag(pol, destChan, r) = outputFlag;
                    }
                }
            }

            // Put (write) the output data/flag
            dc.data().putColumnRange(dstrowslicer, destarrslicer, outdata);
            dc.flag().putColumnRange(dstrowslicer, destarrslicer, outflag);
        }

        row += nRowsThisIteration;
        dstRow += nRowsThisIteration;
    }
}

int MsSplitApp::split(const std::string& invis, const std::string& outvis,
                      const uint32_t startChan,
                      const uint32_t endChan,
                      const uint32_t width,
                      const LOFAR::ParameterSet& parset)
{
    ASKAPLOG_INFO_STR(logger,  "Splitting out channel range " << startChan << " to "
                          << endChan << " (inclusive)");

    if (width > 1) {
        ASKAPLOG_INFO_STR(logger,  "Averaging " << width << " channels to form 1");
    } else {
        ASKAPLOG_INFO_STR(logger,  "No averaging");
    }

    // Verify split parameters
    const uInt nChanIn = endChan - startChan + 1;

    if ((width < 1) || (nChanIn % width != 0)) {
        ASKAPLOG_ERROR_STR(logger, "Width must equally divide the channel range");
        return 1;
    }

    // Open the input measurement set
    const casa::MeasurementSet in(invis);

    // Create the output measurement set
    if (casa::File(outvis).exists()) {
        ASKAPLOG_ERROR_STR(logger, "File or table " << outvis << " already exists!");
        return 1;
    }

    const casa::uInt bucketSize = parset.getUint32("stman.bucketsize", 64 * 1024);
    const casa::uInt tileNcorr = parset.getUint32("stman.tilencorr", 4);
    const casa::uInt tileNchan = parset.getUint32("stman.tilenchan", 1);

    boost::shared_ptr<casa::MeasurementSet> out(create(outvis, bucketSize, tileNcorr, tileNchan));

    // Copy ANTENNA
    ASKAPLOG_INFO_STR(logger,  "Copying ANTENNA table");
    copyAntenna(in, *out);

    // Copy DATA_DESCRIPTION
    ASKAPLOG_INFO_STR(logger,  "Copying DATA_DESCRIPTION table");
    copyDataDescription(in, *out);

    // Copy FEED
    ASKAPLOG_INFO_STR(logger,  "Copying FEED table");
    copyFeed(in, *out);

    // Copy FIELD
    ASKAPLOG_INFO_STR(logger,  "Copying FIELD table");
    copyField(in, *out);

    // Copy OBSERVATION
    ASKAPLOG_INFO_STR(logger,  "Copying OBSERVATION table");
    copyObservation(in, *out);

    // Copy POINTING
    ASKAPLOG_INFO_STR(logger,  "Copying POINTING table");
    copyPointing(in, *out);

    // Copy POLARIZATION
    ASKAPLOG_INFO_STR(logger,  "Copying POLARIZATION table");
    copyPolarization(in, *out);

    // Split SPECTRAL_WINDOW
    ASKAPLOG_INFO_STR(logger,  "Splitting SPECTRAL_WINDOW table");
    splitSpectralWindow(in, *out, startChan, endChan, width);

    // Split main table
    ASKAPLOG_INFO_STR(logger,  "Splitting main table");
    splitMainTable(in, *out, startChan, endChan, width);

    return 0;
}

int MsSplitApp::run(int argc, char* argv[])
{
    StatReporter stats;

    // Get the parameters to split
    const string invis = config().getString("vis");
    const string outvis = config().getString("outputvis");

    // Read channel selection parameters
    const pair<uint32_t, uint32_t> range = ParsetUtils::parseIntRange(config(), "channel");
    const uint32_t width = config().getUint32("width", 1);

    // Read beam selection parameters
    if (config().isDefined("beams")) {
        const vector<uint32_t> v = config().getUint32Vector("beams", true);
        itsBeams.insert(v.begin(), v.end());
        ASKAPLOG_INFO_STR(logger,  "Including ONLY beams: " << v);
    }

    // Read scan id selection parameters
    if (config().isDefined("scans")) {
        const vector<uint32_t> v = config().getUint32Vector("scans", true);
        itsScans.insert(v.begin(), v.end());
        ASKAPLOG_INFO_STR(logger,  "Including ONLY scan numbers: " << v);
    }


    const int error = split(invis, outvis, range.first, range.second, width, config());
    stats.logSummary();
    return error;
}
