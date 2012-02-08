/// @file mssplit.cc
///
/// @brief
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
#include "askap_synthesis.h"

// System includes
#include <sstream>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <iterator>

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "askap/AskapLogging.h"
#include "askap/AskapUtil.h"
#include "askap/Log4cxxLogSink.h"
#include "boost/shared_ptr.hpp"
#include "boost/regex.hpp"
#include "Common/ParameterSet.h"
#include "CommandLineParser.h"
#include "casa/OS/Timer.h"
#include "casa/OS/File.h"
#include "casa/aips.h"
#include "casa/Arrays/Vector.h"
#include "casa/Arrays/Matrix.h"
#include "tables/Tables/TableDesc.h"
#include "tables/Tables/SetupNewTab.h"
#include "tables/Tables/IncrementalStMan.h"
#include "tables/Tables/StandardStMan.h"
#include "tables/Tables/TiledShapeStMan.h"
#include "ms/MeasurementSets/MeasurementSet.h"
#include "ms/MeasurementSets/MSColumns.h"

ASKAP_LOGGER(logger, ".msmerge2");

using namespace askap;
using namespace casa;

boost::shared_ptr<casa::MeasurementSet> create(const std::string& filename)
{
    // Get configuration first to ensure all parameters are present
    casa::uInt bucketSize =  128 * 1024;
    casa::uInt tileNcorr = 4;
    casa::uInt tileNchan = 1;

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

void copyAntenna(const casa::MeasurementSet& source, casa::MeasurementSet& dest)
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

void copyDataDescription(const casa::MeasurementSet& source, casa::MeasurementSet& dest)
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

void copyFeed(const casa::MeasurementSet& source, casa::MeasurementSet& dest)
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

void copyField(const casa::MeasurementSet& source, casa::MeasurementSet& dest)
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

void copyObservation(const casa::MeasurementSet& source, casa::MeasurementSet& dest)
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

void copyPointing(const casa::MeasurementSet& source, casa::MeasurementSet& dest)
{
    const ROMSColumns srcMsc(source);
    const ROMSPointingColumns& sc = srcMsc.pointing();

    MSColumns destMsc(dest);
    MSPointingColumns& dc = destMsc.pointing();

    // Add new rows to the destination and copy the data
    dest.pointing().addRow(sc.nrow());

    // TODO: The first two left out because adding "target" hangs the split (or at least
    // gets it stuch in some long/infinite loop). Maybe need to handle these
    // MeasCol differently
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

void copyPolarization(const casa::MeasurementSet& source, casa::MeasurementSet& dest)
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

void splitSpectralWindow(const casa::MeasurementSet& source,
                         casa::MeasurementSet& dest,
                         const unsigned int startChan,
                         const unsigned int endChan,
                         const unsigned int width)
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
        std::vector<double> chanFreq;
        std::vector<double> chanWidth;
        std::vector<double> effectiveBW;
        std::vector<double> resolution;
        const uInt nChanIn = endChan - startChan + 1;
        const uInt nChanOut = nChanIn / width;
        double totalBandwidth = 0.0;

        for (uInt chan = startChan - 1; chan < nChanOut; ++chan) {
            // Chan is zero-based inside this loop
            totalBandwidth += sc.chanWidth()(row)(casa::IPosition(1, chan));

            chanFreq.push_back(sc.chanFreq()(row)(casa::IPosition(1, chan)));
            chanWidth.push_back(sc.chanWidth()(row)(casa::IPosition(1, chan)));
            effectiveBW.push_back(sc.effectiveBW()(row)(casa::IPosition(1, chan)));
            resolution.push_back(sc.resolution()(row)(casa::IPosition(1, chan)));
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

void splitMainTable(const casa::MeasurementSet& source,
                    casa::MeasurementSet& dest,
                    const unsigned int startChan,
                    const unsigned int endChan,
                    const unsigned int width)
{
    const ROMSColumns sc(source);
    MSColumns dc(dest);

    // Add rows upfront
    const casa::uInt nRows = sc.nrow();
    dest.addRow(nRows);

    // For each row
    for (uInt row = 0; row < nRows; ++row) {
        if (row % 10000 == 0) {
            ASKAPLOG_INFO_STR(logger,  "Splitting and/or averaging row " << row << " of " << nRows);
        }

        // 1: Copy over the simple cells (i.e. those not needing averaging/merging)
        dc.scanNumber().put(row, sc.scanNumber()(row));
        dc.fieldId().put(row, sc.fieldId()(row));
        dc.dataDescId().put(row, sc.dataDescId()(row));
        dc.time().put(row, sc.time()(row));
        dc.timeCentroid().put(row, sc.timeCentroid()(row));
        dc.arrayId().put(row, sc.arrayId()(row));
        dc.processorId().put(row, sc.processorId()(row));
        dc.exposure().put(row, sc.exposure()(row));
        dc.interval().put(row, sc.interval()(row));
        dc.observationId().put(row, sc.observationId()(row));
        dc.antenna1().put(row, sc.antenna1()(row));
        dc.antenna2().put(row, sc.antenna2()(row));
        dc.feed1().put(row, sc.feed1()(row));
        dc.feed2().put(row, sc.feed2()(row));
        dc.uvw().put(row, sc.uvw()(row));
        dc.flagRow().put(row, sc.flagRow()(row));
        dc.weight().put(row, sc.weight()(row));
        dc.sigma().put(row, sc.sigma()(row));

        // 2: Size the matrix for data and flag
        const uInt nPol = sc.data()(row).shape()(0);
        const uInt nChanIn = endChan - startChan + 1;
        const uInt nChanOut = nChanIn / width;

        casa::Matrix<casa::Complex> data(nPol, nChanOut);
        casa::Matrix<casa::Bool> flag(nPol, nChanOut);

        // 3: Copy the data from each input into the output matrix
        const casa::Matrix<casa::Complex>& srcData = sc.data()(row);
        const casa::Matrix<casa::Bool>& srcFlag = sc.flag()(row);
        for (uInt pol = 0; pol < nPol; ++pol) {
            unsigned int destChan = 0;
            for (uInt chan = startChan - 1; chan < nChanOut; ++chan) {
                // Chan is zero-based inside this loop
                data(pol, destChan) = srcData(pol, chan);
                flag(pol, destChan) = srcFlag(pol, chan);
                destChan++;
            }
        }

        // 4: Add those split/merged cells
        dc.data().put(row, data);
        dc.flag().put(row, flag);
    }
}

int split(const std::string& invis, const std::string& outvis,
           const unsigned int startChan,
           const unsigned int endChan,
           const unsigned int width)
{
    // Verify split parameters
    if (((endChan - startChan) % width != 0) && (width > 0)) {
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
    boost::shared_ptr<casa::MeasurementSet> out(create(outvis));

    ASKAPLOG_INFO_STR(logger,  "Splitting out channel range " << startChan << " to " << endChan << " (inclusive)");
    if (width != 1) {
        ASKAPLOG_INFO_STR(logger,  "Averaging " << width << " channels to form one");
    } else {
        ASKAPLOG_INFO_STR(logger,  "No averaging");
    }

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

std::pair<unsigned int, unsigned int> parseRange(const LOFAR::ParameterSet parset)
{
    std::pair<unsigned int, unsigned int> result;
    const std::string raw = parset.getString("channel");

    // These are the two possible patterns, either a single integer, or an
    // integer separated by a dash (potentially surrounded by whitespace)
    const boost::regex e1("[\\d]+");
    const boost::regex e2("([\\d]+)\\s*-\\s*([\\d]+)");

    boost::smatch what;
    if (regex_match(raw, what, e1)) {
        result.first = utility::fromString<unsigned int>(raw);
        result.second = result.first;
    } else if (regex_match(raw, e2)) {
        // Now extract the first and second integer
        const boost::regex digits("[\\d]+");
        boost::sregex_iterator it(raw.begin(), raw.end(), digits);
        boost::sregex_iterator end;

        result.first = utility::fromString<unsigned int>(it->str());
        ++it;
        result.second = utility::fromString<unsigned int>(it->str());
    } else {
        ASKAPLOG_ERROR_STR(logger, "Invalid format 'channel' parameter");
    }

    return result;
}

// Main function
int main(int argc, const char** argv)
{
    std::ostringstream ss;
    ss << argv[0] << ".log_cfg";
    ASKAPLOG_INIT(ss.str().c_str());

    // Ensure that CASA log messages are captured
    casa::LogSinkInterface* globalSink = new Log4cxxLogSink();
    casa::LogSink::globalSink(globalSink);

    int error = 0;
    try {
        casa::Timer timer;
        timer.mark();

        // Command line parser
        cmdlineparser::Parser parser;

        // Command line parameter
        cmdlineparser::FlaggedParameter<std::string> inputsPar("-inputs", "mssplit.in");

        // Throw an exception if the parameter is not present
        parser.add(inputsPar, cmdlineparser::Parser::throw_exception);
        parser.process(argc, const_cast<char**>(argv));

        // Create a parset
        LOFAR::ParameterSet parset(inputsPar);

        // Get the parameters to split
        const std::string invis = parset.getString("vis");
        const std::string outvis = parset.getString("outputvis");
        const std::pair<unsigned int, unsigned int> range = parseRange(parset);

        const unsigned int width = parset.getUint32("width", 1);
        error = split(invis, outvis, range.first, range.second, width);

        ASKAPLOG_INFO_STR(logger,  "Total times - user:   " << timer.user() << " system: " << timer.system()
                              << " real:   " << timer.real());
    } catch (const cmdlineparser::XParser &ex) {
        ASKAPLOG_FATAL_STR(logger, "Command line parser error, wrong arguments " << argv[0]);
        ASKAPLOG_FATAL_STR(logger, "Usage: " << argv[0] << " -o output.ms inMS1 ... inMSn");
        error = 1;
    } catch (const askap::AskapError& x) {
        ASKAPLOG_FATAL_STR(logger, "Askap error in " << argv[0] << ": " << x.what());
        error = 1;
    } catch (const std::exception& x) {
        ASKAPLOG_FATAL_STR(logger, "Unexpected exception in " << argv[0] << ": " << x.what());
        error = 1;
    }

    return error;
}
