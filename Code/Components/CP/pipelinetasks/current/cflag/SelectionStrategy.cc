/// @file SelectionStrategy.cc
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
#include "SelectionStrategy.h"

// Include package level header file
#include "askap_pipelinetasks.h"

// System includes

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "Common/ParameterSet.h"
#include "casa/aipstype.h"
#include "ms/MeasurementSets/MeasurementSet.h"
#include "ms/MeasurementSets/MSColumns.h"
#include "ms/MeasurementSets/MSSelection.h"
#include "casa/Arrays/Vector.h"
#include "casa/Arrays/Matrix.h"

// Local package includes
#include "cflag/FlaggingStats.h"

ASKAP_LOGGER(logger, ".SelectionStrategy");

using namespace askap;
using namespace casa;
using namespace askap::cp::pipelinetasks;

SelectionStrategy:: SelectionStrategy(const LOFAR::ParameterSet& parset,
                                      const casa::MeasurementSet& ms)
    : itsStats("SelectionStrategy"), itsFlagAutoCorr(false),
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
        ASKAPTHROW(AskapError, "Timerange selection not yet implemented");
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
        //itsSelection.setFeedExpr(parset.getString("feed"));
        ASKAPTHROW(AskapError, "Feed selection not yet implemented");
        itsRowCriteria.push_back(FEED);
    }

    if (parset.isDefined("uvrange")) {
        itsSelection.setUvDistExpr(parset.getString("uvrange"));
        ASKAPTHROW(AskapError, "UVRange selection not yet implemented");
        itsRowCriteria.push_back(UVRANGE);
    }

    if (parset.isDefined("autocorr")) {
        itsFlagAutoCorr = parset.getBool("autocorr");
        itsRowCriteria.push_back(AUTOCORR);
    }
}

FlaggingStats SelectionStrategy::stats(void) const
{
    return itsStats;
}

void SelectionStrategy::processRow(casa::MSColumns& msc, const casa::uInt row)
{
    const bool rowCriteriaMatches = dispatch(itsRowCriteria, msc, row);

    // 1: Handle the case where all row criteria match and no detailed criteria
    // exists
    if (rowCriteriaMatches && !itsDetailedCriteriaExists) {
        flagRow(msc, row);
    }

    // 2: Handle the case where there is no row criteria, but there is detailed
    // criteria. Or, where the row criteria exists and match.
    if ((itsRowCriteria.empty() && itsDetailedCriteriaExists)
            || (rowCriteriaMatches && itsDetailedCriteriaExists)) {
        ASKAPLOG_DEBUG_STR(logger, "########### checkDetailed()");
        itsStats.visflagged += checkDetailed(msc, row, true);
    }
}

bool SelectionStrategy::checkBaseline(casa::MSColumns& msc, const casa::uInt row)
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

bool SelectionStrategy::checkField(casa::MSColumns& msc, const casa::uInt row)
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

bool SelectionStrategy::checkTimerange(casa::MSColumns& msc, const casa::uInt row)
{
    return false;
}

bool SelectionStrategy::checkScan(casa::MSColumns& msc, const casa::uInt row)
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

bool SelectionStrategy::checkFeed(casa::MSColumns& msc, const casa::uInt row)
{
    return false;
}

bool SelectionStrategy::checkUVRange(casa::MSColumns& msc, const casa::uInt row)
{
    return false;
}

bool SelectionStrategy::checkAutocorr(casa::MSColumns& msc, const casa::uInt row)
{
    ASKAPDEBUGASSERT(itsFlagAutoCorr);

    const casa::Int ant1 = msc.antenna1()(row);
    const casa::Int ant2 = msc.antenna2()(row);
    return (ant1 == ant2);
}

bool SelectionStrategy::dispatch(const std::vector<SelectionCriteria>& v,
        casa::MSColumns& msc, const casa::uInt row)
{
    std::vector<SelectionCriteria>::const_iterator it;
    for (it = v.begin(); it != v.end(); ++it) {
        switch (*it) {
            case SelectionStrategy::BASELINE:
                if (!checkBaseline(msc, row)) return false;
                break;
            case SelectionStrategy::FIELD:
                if (!checkField(msc, row)) return false;
                break;
            case SelectionStrategy::TIMERANGE:
                if (!checkTimerange(msc, row)) return false;
                break;
            case SelectionStrategy::SCAN:
                if (!checkScan(msc, row)) return false;
                break;
            case SelectionStrategy::FEED:
                if (!checkFeed(msc, row)) return false;
                break;
            case SelectionStrategy::UVRANGE:
                if (!checkUVRange(msc, row)) return false;
                break;
            case SelectionStrategy::AUTOCORR:
                if (!checkAutocorr(msc, row)) return false;
                break;
            default:
                break;
        }
    }
    return true;
}

unsigned long SelectionStrategy::checkDetailed(casa::MSColumns& msc, const casa::uInt row, const bool doFlag)
{
    // Count the number of visibilities flagged (or that would have been
    // flagged)
    unsigned long flagCount = 0;

    const Matrix<casa::Int> chanList = itsSelection.getChanList();
    if (chanList.empty()) {
        ASKAPLOG_DEBUG_STR(logger, "Channel flagging list is EMPTY");
        return false;
    }
    ASKAPCHECK(chanList.ncolumn() == 4, "Expected four columns");
    Matrix<casa::Bool> flags = msc.flag()(row);

    const casa::ROMSDataDescColumns& ddc = msc.dataDescription();

    ASKAPLOG_DEBUG_STR(logger, "Channel flagging list size: " << chanList.size());
    for (size_t i = 0; i < chanList.size(); ++i) {
        const casa::Int spwID = flags(i, 0);
        const casa::Int startCh = flags(i, 1);
        const casa::Int stopCh = flags(i, 2);
        const casa::Int step = flags(i, 3);
        ASKAPLOG_DEBUG_STR(logger, "spwID: " << spwID
                << ", startCh: " << startCh
                << ", stopCh: " << stopCh
                << ", step: " << step);
        ASKAPCHECK(step > 0, "Step must be greater than zero to avoid infinite loop");
        const casa::Int dataDescId = msc.dataDescId()(row);
        const casa::Int descSpwId = ddc.spectralWindowId()(dataDescId);
        if (descSpwId != spwID) {
            continue;
        }

        for (casa::Int chan = startCh; chan <= stopCh; chan += step) {
            for (casa::uInt pol = 0; pol < flags.nrow(); ++pol) {
                flags(pol, chan) = true;
                flagCount++;
            }
        }
    }

    if (doFlag) {
        msc.flag().put(row, flags);
    }
    return flagCount;
}

void SelectionStrategy::flagRow(casa::MSColumns& msc, const casa::uInt row)
{
    itsStats.rowsflagged++;
    msc.flagRow().put(row, true);
}
