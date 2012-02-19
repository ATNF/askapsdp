/// @file msmerge2.cc
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
#include <algorithm>
#include <iterator>

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "askap/AskapLogging.h"
#include "askap/Log4cxxLogSink.h"
#include "boost/shared_ptr.hpp"
#include "CommandLineParser.h"
#include "casa/OS/Timer.h"
#include "casa/OS/File.h"
#include "casa/aips.h"
#include "casa/Quanta.h"
#include "casa/Arrays/Vector.h"
#include "casa/Arrays/MatrixMath.h"
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

    // TODO: The first two left out because adding "target" hangs the merge (or at least
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

void appendToVector(const casa::Vector<double>& src, std::vector<double>& dest)
{
    std::copy(src.begin(), src.end(), std::back_inserter(dest));
}

void mergeSpectralWindow(const std::vector< boost::shared_ptr<const ROMSColumns> >& srcMscs,
        MSColumns& destMsc,
        casa::MeasurementSet& dest)
{
    MSSpWindowColumns& dc = destMsc.spectralWindow();
    const ROMSSpWindowColumns& sc = srcMscs[0]->spectralWindow();
    dest.spectralWindow().addRow(sc.nrow());

    // For each row
    const uInt nrows = sc.nrow();
    for (uInt row = 0; row < nrows; ++row) {

        // 1: Copy over the simple cells (i.e. those not needing merging)
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
        uInt nChan = 0;
        double totalBandwidth = 0.0;

        for (uInt i = 0; i < srcMscs.size(); ++i) {
            const ROMSSpWindowColumns& spwc = srcMscs[i]->spectralWindow();
            nChan += spwc.numChan()(row);
            totalBandwidth += spwc.totalBandwidth()(row);

            appendToVector(spwc.chanFreq()(row), chanFreq);
            appendToVector(spwc.chanWidth()(row), chanWidth);
            appendToVector(spwc.effectiveBW()(row), effectiveBW);
            appendToVector(spwc.resolution()(row), resolution);
        }

        // 3: Add those merged cells
        dc.numChan().put(row, nChan);
        dc.chanFreq().put(row, casa::Vector<double>(chanFreq));
        dc.chanWidth().put(row, casa::Vector<double>(chanWidth));
        dc.effectiveBW().put(row, casa::Vector<double>(effectiveBW));
        dc.resolution().put(row, casa::Vector<double>(resolution));
        dc.totalBandwidth().put(row, totalBandwidth);

    } // End for rows
}

void mergeMainTable(const std::vector< boost::shared_ptr<const ROMSColumns> >& srcMscs,
        MSColumns& dc,
        casa::MeasurementSet& dest)
{
    // Add rows upfront
    const ROMSColumns& sc = *(srcMscs[0]);
    const casa::uInt nRows = sc.nrow();
    dest.addRow(nRows);

    // For each row
    for (uInt row = 0; row < nRows; ++row) {
        if (row % 10000 == 0) {
            ASKAPLOG_INFO_STR(logger,  "Merging row " << row << " of " << nRows);
        }

        // 1: Copy over the simple cells (i.e. those not needing merging)
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
        const uInt nChan = sc.data()(row).shape()(1);
        const uInt nChanTotal = nChan * srcMscs.size();

        casa::Matrix<casa::Complex> data(nPol, nChanTotal);
        casa::Matrix<casa::Bool> flag(nPol, nChanTotal);

        // 3: Copy the data from each input into the output matrix
        for (uInt i = 0; i < srcMscs.size(); ++i) {
            const casa::Matrix<casa::Complex> srcData = srcMscs[i]->data()(row);
            const casa::Matrix<casa::Bool> srcFlag = srcMscs[i]->flag()(row);
            for (uInt pol = 0; pol < nPol; ++pol) {
                for (uInt chan = 0; chan < nChan; ++chan) {
                    data(pol, chan + (nChan * i)) = srcData(pol, chan);
                    flag(pol, chan + (nChan * i)) = srcFlag(pol, chan);
                }
            }
        }

        // 4: Add those merged cells
        dc.data().put(row, data);
        dc.flag().put(row, flag);
    }
}

void merge(const std::vector<std::string>& inFiles, const std::string& outFile)
{
    // Create the output measurement set
    ASKAPCHECK(!casa::File(outFile).exists(), "File or table "
            << outFile << " already exists!");
    boost::shared_ptr<casa::MeasurementSet> out(create(outFile));

    // Open the input measurement sets
    std::vector< boost::shared_ptr<const casa::MeasurementSet> > in;
    std::vector< boost::shared_ptr<const ROMSColumns> > inColumns;
    std::vector<std::string>::const_iterator it;
    for (it = inFiles.begin(); it != inFiles.end(); ++it) {
        const boost::shared_ptr<const casa::MeasurementSet> p(new casa::MeasurementSet(*it));
        in.push_back(p);
        inColumns.push_back(boost::shared_ptr<const ROMSColumns>(new ROMSColumns(*p)));
    }

    ASKAPLOG_INFO_STR(logger,  "First copy " << inFiles[0]<< " into " << outFile);

    // Copy ANTENNA
    ASKAPLOG_INFO_STR(logger,  "Copying ANTENNA table");
    copyAntenna(**(in.begin()), *out);

    // Copy DATA_DESCRIPTION
    ASKAPLOG_INFO_STR(logger,  "Copying DATA_DESCRIPTION table");
    copyDataDescription(**(in.begin()), *out);

    // Copy FEED
    ASKAPLOG_INFO_STR(logger,  "Copying FEED table");
    copyFeed(**(in.begin()), *out);

    // Copy FIELD
    ASKAPLOG_INFO_STR(logger,  "Copying FIELD table");
    copyField(**(in.begin()), *out);

    // Copy OBSERVATION
    ASKAPLOG_INFO_STR(logger,  "Copying OBSERVATION table");
    copyObservation(**(in.begin()), *out);

    // Copy POINTING
    ASKAPLOG_INFO_STR(logger,  "Copying POINTING table");
    copyPointing(**(in.begin()), *out);

    // Copy POLARIZATION
    ASKAPLOG_INFO_STR(logger,  "Copying POLARIZATION table");
    copyPolarization(**(in.begin()), *out);

    // Merge SPECTRAL_WINDOW
    ASKAPLOG_INFO_STR(logger,  "Merging SPECTRAL_WINDOW table");
    MSColumns destMsc(*out);
    mergeSpectralWindow(inColumns, destMsc, *out);

    // Merge main table
    ASKAPLOG_INFO_STR(logger,  "Merging main table");
    mergeMainTable(inColumns, destMsc, *out);
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

    try {
        casa::Timer timer;
        timer.mark();

        cmdlineparser::Parser parser; // a command line parser
        // command line parameter
        cmdlineparser::FlaggedParameter<std::string> outName("-o", "output.ms");
        // this parameter is required
        parser.add(outName, cmdlineparser::Parser::throw_exception);
        if (argc < 4) {
            throw cmdlineparser::XParser();
        }
        std::vector<cmdlineparser::GenericParameter<std::string> > inNames(argc-3);
        {
            std::vector<cmdlineparser::GenericParameter<std::string> >::iterator it;
            for (it = inNames.begin(); it < inNames.end(); ++it) {
                parser.add(*it);
            }
        }

        // Process command line options
        parser.process(argc, argv);
        if (!inNames.size()) {
            throw cmdlineparser::XParser();
        } 
        ASKAPLOG_INFO_STR(logger,
                "This program merges given measurement sets and writes the output into `"
                << outName.getValue() << "`");

        // Turns inNames into vector<string>
        std::vector<std::string> inNamesVec;
        std::vector<cmdlineparser::GenericParameter<std::string> >::iterator it;
        for (it = inNames.begin(); it < inNames.end(); ++it) {
            inNamesVec.push_back(it->getValue());
        }

        merge(inNamesVec, outName.getValue()); 

        ASKAPLOG_INFO_STR(logger,  "Total times - user:   " << timer.user() << " system: " << timer.system()
                << " real:   " << timer.real());
        ///==============================================================================
    } catch (const cmdlineparser::XParser &ex) {
        ASKAPLOG_FATAL_STR(logger, "Command line parser error, wrong arguments " << argv[0]);
        ASKAPLOG_FATAL_STR(logger, "Usage: " << argv[0] << " -o output.ms inMS1 ... inMSn");
        return 1;
    } catch (const askap::AskapError& x) {
        ASKAPLOG_FATAL_STR(logger, "Askap error in " << argv[0] << ": " << x.what());
        return 1;
    } catch (const std::exception& x) {
        ASKAPLOG_FATAL_STR(logger, "Unexpected exception in " << argv[0] << ": " << x.what());
        return 1;
    }

    return 0;
}
