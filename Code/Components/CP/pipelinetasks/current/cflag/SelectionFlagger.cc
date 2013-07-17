/// @file SelectionFlagger.cc
///
/// @copyright (c) 2011 CSIRO
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
#include "SelectionFlagger.h"

// Include package level header file
#include "askap_pipelinetasks.h"

// System includes
#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <stdint.h>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "boost/shared_ptr.hpp"
#include "Common/ParameterSet.h"
#include "casa/aipstype.h"
#include "ms/MeasurementSets/MeasurementSet.h"
#include "ms/MeasurementSets/MSColumns.h"
#include "ms/MeasurementSets/MSSelection.h"
#include "casa/Arrays/Vector.h"
#include "casa/Arrays/Matrix.h"

// Local package includes
#include "cflag/FlaggingStats.h"

ASKAP_LOGGER(logger, ".SelectionFlagger");

using namespace askap;
using namespace casa;
using namespace askap::cp::pipelinetasks;

vector< boost::shared_ptr<IFlagger> > SelectionFlagger::build(
        const LOFAR::ParameterSet& parset,
        const casa::MeasurementSet& ms)
{
    vector< boost::shared_ptr<IFlagger> > flaggers;
    const string key = "selection_flagger.rules";
    if (parset.isDefined(key)) {
        const vector<string> rules = parset.getStringVector(key);
        vector<string>::const_iterator it;
        for (it = rules.begin(); it != rules.end(); ++it) {
            ASKAPLOG_DEBUG_STR(logger, "Processing rule: " << *it);
            stringstream ss;
            ss << "selection_flagger." << *it << ".";
            const LOFAR::ParameterSet subset = parset.makeSubset(ss.str());
            flaggers.push_back(boost::shared_ptr<IFlagger>(new SelectionFlagger(subset, ms)));
        }
    }
    return flaggers;
}

SelectionFlagger:: SelectionFlagger(const LOFAR::ParameterSet& parset,
                                      const casa::MeasurementSet& ms)
        : itsStats("SelectionFlagger"), itsFlagAutoCorr(false),
        itsDetailedCriteriaExists(false)
{
    itsSelection.resetMS(ms);

    if (parset.isDefined("field")) {
        itsSelection.setFieldExpr(parset.getString("field"));
        itsRowCriteria.push_back(FIELD);
    }

    if (parset.isDefined("spw")) {
        itsSelection.setSpwExpr(parset.getString("spw"));
        itsDetailedCriteriaExists = true;
    }

    if (parset.isDefined("antenna")) {
        itsSelection.setAntennaExpr(parset.getString("antenna"));
        itsRowCriteria.push_back(BASELINE);
    }

    if (parset.isDefined("timerange")) {
        itsSelection.setTimeExpr(parset.getString("timerange"));
        itsRowCriteria.push_back(TIMERANGE);
    }

    if (parset.isDefined("correlation")) {
        itsSelection.setPolnExpr(parset.getString("correlation"));
        ASKAPTHROW(AskapError, "Correlation selection not yet implemented");
        itsDetailedCriteriaExists = true;
    }

    if (parset.isDefined("scan")) {
        itsSelection.setScanExpr(parset.getString("scan"));
        itsRowCriteria.push_back(SCAN);
    }

    if (parset.isDefined("feed")) {
        const std::vector<uint32_t> v = parset.getUint32Vector("feed");
        itsFeedsFlagged.insert(v.begin(), v.end());
        itsRowCriteria.push_back(FEED);
    }

    if (parset.isDefined("uvrange")) {
        itsSelection.setUvDistExpr(parset.getString("uvrange"));
        // Specifying a uvrange does results in flagging of baselines
        itsRowCriteria.push_back(BASELINE);
    }

    if (parset.isDefined("autocorr")) {
        itsFlagAutoCorr = parset.getBool("autocorr");
        if (itsFlagAutoCorr) {
            itsRowCriteria.push_back(AUTOCORR);
        }
    }

    if (itsRowCriteria.empty() && !itsDetailedCriteriaExists) {
        ASKAPTHROW(AskapError, "No selection criteria for rule specified");
    }
}

FlaggingStats SelectionFlagger::stats(void) const
{
    return itsStats;
}

void SelectionFlagger::processRow(casa::MSColumns& msc, const casa::uInt row,
                                   const bool dryRun)
{
    const bool rowCriteriaMatches = dispatch(itsRowCriteria, msc, row);

    // 1: Handle the case where all row criteria match and no detailed criteria
    // exists
    if (rowCriteriaMatches && !itsDetailedCriteriaExists) {
        flagRow(msc, row, dryRun);
    }

    // 2: Handle the case where there is no row criteria, but there is detailed
    // criteria. Or, where the row criteria exists and match.
    if ((itsRowCriteria.empty() && itsDetailedCriteriaExists)
            || (rowCriteriaMatches && itsDetailedCriteriaExists)) {
        checkDetailed(msc, row, dryRun);
    }
}

bool SelectionFlagger::checkBaseline(casa::MSColumns& msc, const casa::uInt row)
{
    const Matrix<casa::Int> m = itsSelection.getBaselineList();
    if (m.empty()) {
        return false;
    }
    ASKAPCHECK(m.ncolumn() == 2, "Expected two columns");

    const casa::Int ant1 = msc.antenna1()(row);
    const casa::Int ant2 = msc.antenna2()(row);
    for (size_t i = 0; i < m.nrow(); ++i) {
        if ((m(i, 0) == ant1 && m(i, 1) == ant2)
                || (m(i, 0) == ant2 && m(i, 1) == ant1)) {
            return true;
        }
    }

    return false;
}

bool SelectionFlagger::checkField(casa::MSColumns& msc, const casa::uInt row)
{
    const casa::Int fieldId = msc.fieldId()(row);
    const Vector<casa::Int> v = itsSelection.getFieldList();
    for (size_t i = 0; i < v.size(); ++i) {
        if (v[i] == fieldId) {
            return true;
        }
    }
    return false;
}

bool SelectionFlagger::checkTimerange(casa::MSColumns& msc, const casa::uInt row)
{
    const Matrix<casa::Double> timeList = itsSelection.getTimeList();
    if (timeList.empty()) {
        ASKAPLOG_DEBUG_STR(logger, "Time list is EMPTY");
        return false;
    }
    ASKAPCHECK(timeList.nrow() == 2, "Expected two rows");
    ASKAPCHECK(timeList.ncolumn() == 1,
            "Only a single time range specification is supported");
    const casa::Double t = msc.time()(row);
    if (t > timeList(0, 0) && t < timeList(1, 0)) {
        return true;
    } else {
        return false;
    }
}

bool SelectionFlagger::checkScan(casa::MSColumns& msc, const casa::uInt row)
{
    const casa::Int scanNum = msc.scanNumber()(row);
    const Vector<casa::Int> v = itsSelection.getScanList();
    for (size_t i = 0; i < v.size(); ++i) {
        if (v[i] == scanNum) {
            return true;
        }
    }
    return false;
}

bool SelectionFlagger::checkFeed(casa::MSColumns& msc, const casa::uInt row)
{
    const casa::Int feed1 = msc.feed1()(row);
    const casa::Int feed2 = msc.feed2()(row);

    if ((itsFeedsFlagged.find(feed1) != itsFeedsFlagged.end())
            || (itsFeedsFlagged.find(feed2) != itsFeedsFlagged.end())) {
        return true;
    } else {
        return false;
    }
}

bool SelectionFlagger::checkAutocorr(casa::MSColumns& msc, const casa::uInt row)
{
    ASKAPDEBUGASSERT(itsFlagAutoCorr);

    const casa::Int ant1 = msc.antenna1()(row);
    const casa::Int ant2 = msc.antenna2()(row);
    return (ant1 == ant2);
}

bool SelectionFlagger::dispatch(const std::vector<SelectionCriteria>& v,
                                 casa::MSColumns& msc, const casa::uInt row)
{
    std::vector<SelectionCriteria>::const_iterator it;
    for (it = v.begin(); it != v.end(); ++it) {
        switch (*it) {
            case SelectionFlagger::BASELINE:
                if (!checkBaseline(msc, row)) return false;
                break;
            case SelectionFlagger::FIELD:
                if (!checkField(msc, row)) return false;
                break;
            case SelectionFlagger::TIMERANGE:
                if (!checkTimerange(msc, row)) return false;
                break;
            case SelectionFlagger::SCAN:
                if (!checkScan(msc, row)) return false;
                break;
            case SelectionFlagger::FEED:
                if (!checkFeed(msc, row)) return false;
                break;
            case SelectionFlagger::AUTOCORR:
                if (!checkAutocorr(msc, row)) return false;
                break;
            default:
                break;
        }
    }
    return true;
}

void SelectionFlagger::checkDetailed(casa::MSColumns& msc, const casa::uInt row, const bool dryRun)
{
    const Matrix<casa::Int> chanList = itsSelection.getChanList();
    if (chanList.empty()) {
        ASKAPLOG_DEBUG_STR(logger, "Channel flagging list is EMPTY");
        return;
    }
    ASKAPCHECK(chanList.ncolumn() == 4, "Expected four columns");
    Matrix<casa::Bool> flags = msc.flag()(row);

    const casa::ROMSDataDescColumns& ddc = msc.dataDescription();

    //ASKAPLOG_DEBUG_STR(logger, "Channel flagging list size: " << chanList.nrow());
    for (size_t i = 0; i < chanList.nrow(); ++i) {
        const casa::Int spwID = chanList(i, 0);
        const casa::Int startCh = chanList(i, 1);
        const casa::Int stopCh = chanList(i, 2);
        const casa::Int step = chanList(i, 3);
        //ASKAPLOG_DEBUG_STR(logger, "spwID: " << spwID
        //                       << ", startCh: " << startCh
        //                       << ", stopCh: " << stopCh
        //                       << ", step: " << step);
        ASKAPCHECK(step > 0, "Step must be greater than zero to avoid infinite loop");
        const casa::Int dataDescId = msc.dataDescId()(row);
        const casa::Int descSpwId = ddc.spectralWindowId()(dataDescId);
        if (descSpwId != spwID) {
            continue;
        }

        for (casa::Int chan = startCh; chan <= stopCh; chan += step) {
            for (casa::uInt pol = 0; pol < flags.nrow(); ++pol) {
                flags(pol, chan) = true;
                itsStats.visFlagged++;
            }
        }
    }

    if (!dryRun) {
        msc.flag().put(row, flags);
    }
}

void SelectionFlagger::flagRow(casa::MSColumns& msc, const casa::uInt row, const bool dryRun)
{
    Matrix<casa::Bool> flags = msc.flag()(row);
    flags = true;

    itsStats.visFlagged += flags.size();
    itsStats.rowsFlagged++;

    if (!dryRun) {
        msc.flagRow().put(row, true);
        msc.flag().put(row, flags);
    }
}
