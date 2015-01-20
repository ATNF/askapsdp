/// @file
///
/// XXX Notes on program XXX
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
/// @author XXX XXX <XXX.XXX@csiro.au>
///
#include <askap_analysis.h>
#include <patternmatching/CatalogueMatcher.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <patternmatching/Triangle.h>
#include <patternmatching/Point.h>
#include <patternmatching/PointCatalogue.h>
#include <patternmatching/MatchingUtilities.h>
#include <casainterface/CasaInterface.h>

#include <Common/ParameterSet.h>
#include <boost/shared_ptr.hpp>
#include <images/Images/ImageInterface.h>
#include <images/Images/ImageOpener.h>
#include <images/Images/FITSImage.h>
#include <images/Images/MIRIADImage.h>
#include <casa/Arrays/Vector.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <casa/Quanta.h>

#include <vector>

ASKAP_LOGGER(logger, ".cataloguematching");

namespace askap {

namespace analysis {

namespace matching {

CatalogueMatcher::CatalogueMatcher()
{
}

CatalogueMatcher::CatalogueMatcher(const LOFAR::ParameterSet& parset)
{
    itsReferenceImage = parset.getString("referenceImage", "");

    LOFAR::ParameterSet subset = parset.makeSubset("source.");
    subset.add("referenceImage", itsReferenceImage);
    itsSrcCatalogue = PointCatalogue(subset);

    subset = parset.makeSubset("reference.");
    subset.add("referenceImage", itsReferenceImage);
    itsRefCatalogue = PointCatalogue(subset);

    itsMatchFile = parset.getString("matchFile", "matches.txt");
    itsMissFile = parset.getString("missFile", "misses.txt");
    itsPositionUnits = casa::Unit(parset.getString("positionUnits", "deg"));
    if (!parset.isDefined("epsilon")) {
        ASKAPTHROW(AskapError, "The epsilon parameter must be provided.");
    }
    std::string epsilonString = parset.getString("epsilon");
    casa::Quantity q;
    casa::Quantity::read(q, epsilonString);
    itsEpsilon = q.getValue(itsPositionUnits);
    itsEpsilonUnits = q.getUnit();
    this->convertEpsilon();
    ASKAPLOG_DEBUG_STR(logger, "Requested epsilon value was " << epsilonString <<
                       ", which is " << itsEpsilon << " " << itsPositionUnits.getName());
    // itsEpsilon = parset.getDouble("epsilon");
    if (itsEpsilon < 0) {
        ASKAPTHROW(AskapError, "The epsilon parameter must be positive.");
    }
    itsMeanDx = itsMeanDy = 0.;
    itsSourceSummaryFile = parset.getString("srcSummaryFile", "match-summary-sources.txt");
    itsReferenceSummaryFile = parset.getString("refSummaryFile", "match-summary-reference.txt");
}

void CatalogueMatcher::convertEpsilon()
{
    if (itsReferenceImage != "") {
        const boost::shared_ptr<ImageInterface<Float> > imagePtr =
            analysisutilities::openImage(itsReferenceImage);

        int dirCooNum = imagePtr->coordinates().findCoordinate(casa::Coordinate::DIRECTION);
        casa::DirectionCoordinate dirCoo = imagePtr->coordinates().directionCoordinate(dirCooNum);

        ASKAPLOG_DEBUG_STR(logger, "Converting epsilon from " << itsEpsilon <<
                           " " << itsPositionUnits.getName());
        ASKAPASSERT(dirCoo.worldAxisUnits()[0] == dirCoo.worldAxisUnits()[1]);

        casa::Quantity eps(itsEpsilon, itsPositionUnits);
        double epsInWorldUnits = eps.getValue(dirCoo.worldAxisUnits()[0]);
        double pixScale = sqrt(fabs(dirCoo.increment()[0] *  dirCoo.increment()[1]));
        itsEpsilon = epsInWorldUnits / pixScale;
        itsEpsilonUnits.setValue(itsEpsilonUnits.getValue() / pixScale);
        ASKAPLOG_DEBUG_STR(logger, "Now have epsilon = " << itsEpsilon);
    }

}


bool CatalogueMatcher::read()
{

    bool filesOK = true;

    if (!itsSrcCatalogue.read()) {
        ASKAPLOG_FATAL_STR(logger, "Could not read Source Catalogue");
        return false;
    }
    if (!itsRefCatalogue.read()) {
        ASKAPLOG_FATAL_STR(logger, "Could not read Reference Catalogue");
        return false;
    }

    filesOK = itsSrcCatalogue.pointList().size() > 0 && itsRefCatalogue.pointList().size() > 0;
    if (itsSrcCatalogue.pointList().size() == 0) {
        ASKAPLOG_ERROR_STR(logger, "Could not read source catalogue from " <<
                           itsSrcCatalogue.filename());
    }
    if (itsRefCatalogue.pointList().size() == 0) {
        ASKAPLOG_ERROR_STR(logger, "Could not read source catalogue from " <<
                           itsRefCatalogue.filename());
    }
    if (filesOK) {
        ASKAPLOG_INFO_STR(logger, "Size of source pixel list = " <<
                          itsSrcCatalogue.pointList().size() <<
                          " and triangle list = " <<
                          itsSrcCatalogue.triangleList().size());
        ASKAPLOG_INFO_STR(logger, "Size of reference pixel list = " <<
                          itsRefCatalogue.pointList().size() <<
                          " and triangle list = " <<
                          itsRefCatalogue.triangleList().size());
    }

    // if(!itsRefCatalogue.crudeMatch(itsSrcCatalogue.fullPointList(), itsEpsilon))
    //      ASKAPLOG_WARN_STR(logger, "Crude matching failed! Using full reference point list");

    return filesOK;
}


//**************************************************************//

void CatalogueMatcher::findMatches()
{

    itsMatchingTriList = matchLists(itsSrcCatalogue.triangleList(),
                                    itsRefCatalogue.triangleList(),
                                    itsEpsilon);

    trimTriList(itsMatchingTriList);
    ASKAPLOG_INFO_STR(logger, "Found " << itsMatchingTriList.size() << " matches");
    itsNumInitialMatches = 0;

    if (itsMatchingTriList.size() > 0) {
        itsMatchingPixList = vote(itsMatchingTriList);
        itsNumInitialMatches = itsMatchingPixList.size();
        ASKAPLOG_INFO_STR(logger, "After voting, have found " <<
                          itsMatchingPixList.size() << " matching points");

        itsSenseMatch = (itsMatchingTriList[0].first.isClockwise() ==
                         itsMatchingTriList[0].second.isClockwise());

        if (itsSenseMatch) {
            ASKAPLOG_INFO_STR(logger, "The two lists have the same sense.");
        } else {
            ASKAPLOG_INFO_STR(logger, "The two lists have the opposite sense.");
        }
    }
}

//**************************************************************//

void CatalogueMatcher::zeroOffsetMatch()
{
    size_t srcSize = itsSrcCatalogue.pointList().size();
    size_t refSize = itsRefCatalogue.pointList().size();
    ASKAPLOG_DEBUG_STR(logger, "Performing zero-offset match of lists of size " <<
                       srcSize << " and " << refSize);

    std::vector<bool> srcMatched(srcSize, false);
    std::vector<bool> refMatched(refSize, false);
    int nmatch = 0;

    std::sort(itsSrcCatalogue.pointList().begin(), itsSrcCatalogue.pointList().end());
    std::sort(itsRefCatalogue.pointList().begin(), itsRefCatalogue.pointList().end());

    for (size_t s = 0; s < itsSrcCatalogue.pointList().size(); s++) {

        for (size_t r = 0; r < itsRefCatalogue.pointList().size() && !srcMatched[s]; r++) {

            if (!refMatched[r]) {
                if (itsSrcCatalogue.pointList()[s].sep(itsRefCatalogue.pointList()[r]) <
                        itsEpsilon) {

                    itsMatchingPixList.push_back(
                        std::pair<Point, Point>(itsSrcCatalogue.pointList()[s],
                                                itsRefCatalogue.pointList()[r]));

                    refMatched[r] = true;
                    srcMatched[s] = true;
                    nmatch++;
                }
            }
        }
    }

    ASKAPLOG_DEBUG_STR(logger, "Matched " << nmatch << " pairs of points");

}

//**************************************************************//

void CatalogueMatcher::findOffsets()
{
    std::vector<double> dx(itsMatchingPixList.size(), 0.);
    std::vector<double> dy(itsMatchingPixList.size(), 0.);

    double sensescale = itsSenseMatch ? -1. : 1.;
    for (size_t i = 0; i < itsMatchingPixList.size(); i++) {
        dx[i] = itsMatchingPixList[i].first.x() - itsMatchingPixList[i].second.x();
        dy[i] = itsMatchingPixList[i].first.y() + sensescale * itsMatchingPixList[i].second.y();
    }

    itsMeanDx = itsMeanDy = 0.;
    for (size_t i = 0; i < itsMatchingPixList.size(); i++) {
        itsMeanDx += dx[i];
        itsMeanDy += dy[i];
    }
    itsMeanDx /= double(itsMatchingPixList.size());
    itsMeanDy /= double(itsMatchingPixList.size());

    // Don't need these elsewhere, so make local only
    double rmsDx = 0., rmsDy = 0.;
    for (size_t i = 0; i < itsMatchingPixList.size(); i++) {
        rmsDx += (dx[i] - itsMeanDx) * (dx[i] - itsMeanDx);
        rmsDy += (dy[i] - itsMeanDy) * (dy[i] - itsMeanDy);
    }
    rmsDx = sqrt(rmsDx / (double(itsMatchingPixList.size() - 1)));
    rmsDy = sqrt(rmsDy / (double(itsMatchingPixList.size() - 1)));

    std::stringstream ss;
    ss << "Offsets between the two are dx = "
       << casa::Quantity(itsMeanDx, itsPositionUnits).getValue(itsEpsilonUnits)
       << " +- " << casa::Quantity(rmsDx, itsPositionUnits).getValue(itsEpsilonUnits)
       << " dy = "
       << casa::Quantity(itsMeanDy, itsPositionUnits).getValue(itsEpsilonUnits)
       << " +- " << casa::Quantity(rmsDy, itsPositionUnits).getValue(itsEpsilonUnits);
    ASKAPLOG_INFO_STR(logger, ss.str());
}

//**************************************************************//

void CatalogueMatcher::addNewMatches()
{

    if (itsNumInitialMatches > 0) {

        this->rejectMultipleMatches();
        const float matchRadius = 3.;
        std::vector<Point>::iterator src, ref;
        std::vector<std::pair<Point, Point> >::iterator match;

        for (src = itsSrcCatalogue.fullPointList().begin();
                src < itsSrcCatalogue.fullPointList().end();
                src++) {

            bool isMatch = false;
            match = itsMatchingPixList.begin();

            for (; match < itsMatchingPixList.end() && !isMatch; match++) {
                isMatch = (src->ID() == match->first.ID());
            }

            if (!isMatch) {
                float minOffset = 0.;
                int minRef = -1;

                for (ref = itsRefCatalogue.fullPointList().begin();
                        ref < itsRefCatalogue.fullPointList().end();
                        ref++) {

                    float offset = hypot(src->x() - ref->x() - itsMeanDx,
                                         src->y() - ref->y() - itsMeanDy);

                    if (offset < matchRadius * itsEpsilon) {
                        if ((minRef == -1) || (offset < minOffset)) {
                            minOffset = offset;
                            minRef = int(ref - itsRefCatalogue.fullPointList().begin());
                        }
                    }
                }

                if (minRef >= 0) { // there was a match within errors
                    ref = itsRefCatalogue.fullPointList().begin() + minRef;
                    std::pair<Point, Point> newMatch(*src, *ref);
                    itsMatchingPixList.push_back(newMatch);
                }
            }
        }

        this->rejectMultipleMatches();

        ASKAPLOG_INFO_STR(logger, "Total number of matches = " << itsMatchingPixList.size());
    }
}

//**************************************************************//

void CatalogueMatcher::rejectMultipleMatches()
{

    if (itsMatchingPixList.size() < 2) return;

    std::vector<std::pair<Point, Point> >::iterator alice, bob;
    alice = itsMatchingPixList.begin();

    while (alice < itsMatchingPixList.end() - 1) {
        bool aliceGone = false;
        bob = alice + 1;

        while (bob < itsMatchingPixList.end() && !aliceGone) {
            bool bobGone = false;
            if (alice->second.ID() == bob->second.ID()) {
                // alice & bob have the same reference source
                double df_alice = alice->first.flux() - alice->second.flux();
                double df_bob   = bob->first.flux() - bob->second.flux();

                if (fabs(df_alice) < fabs(df_bob)) {
                    itsMatchingPixList.erase(bob);
                    bobGone = true;
                } else {
                    itsMatchingPixList.erase(alice);
                    aliceGone = true;
                }
            }
            if (!bobGone) bob++;
        }

        if (!aliceGone) alice++;
    }
}

//**************************************************************//

void CatalogueMatcher::outputMatches()
{
    std::ofstream fout(itsMatchFile.c_str());
    if (fout.is_open()) {
        std::vector<std::pair<Point, Point> >::iterator match;
        int prec = 3;
        size_t width = 0;
        for (match = itsMatchingPixList.begin();
                match < itsMatchingPixList.end();
                match++) {
            prec = std::max(prec, int(ceil(log10(1. / match->first.flux()))) + 1);
            width = std::max(width, match->first.ID().size());
            width = std::max(width, match->second.ID().size());
        }

        fout.setf(std::ios::fixed);
        unsigned int ct = 0;
        char matchType;

        for (match = itsMatchingPixList.begin();
                match < itsMatchingPixList.end();
                match++) {

            if (ct++ < itsNumInitialMatches) matchType = '1';
            else matchType = '2';

            casa::Quantity sep(match->first.sep(match->second), itsPositionUnits);

            fout << std::setw(3) << matchType << " "
                 << std::setw(width) << match->first.ID() << " "
                 << std::setw(width) << match->second.ID() << " "
                 << std::setw(8)  << std::setprecision(6)
                 << sep.getValue(itsEpsilonUnits) << "\n";

        }

        fout.close();
    } else {
        ASKAPLOG_ERROR_STR(logger, "Could not open match file " << itsMatchFile);
    }

}

//**************************************************************//

void CatalogueMatcher::outputMisses()
{
    std::ofstream fout(itsMissFile.c_str());
    if (fout.is_open()) {
        fout.setf(std::ios::fixed);
        std::vector<Point>::iterator pt;
        std::vector<std::pair<Point, Point> >::iterator match;

        size_t width = 0;
        for (pt = itsRefCatalogue.fullPointList().begin();
                pt < itsRefCatalogue.fullPointList().end();
                pt++) {
            width = std::max(width, pt->ID().size());
        }

        for (pt = itsRefCatalogue.fullPointList().begin();
                pt < itsRefCatalogue.fullPointList().end();
                pt++) {

            bool isMatch = false;

            for (match = itsMatchingPixList.begin();
                    match < itsMatchingPixList.end() && !isMatch;
                    match++) {
                isMatch = (pt->ID() == match->second.ID());
            }

            if (!isMatch) {
                fout << "R "
                     << std::setw(width) << pt->ID() << " "
                     << std::setw(10) << std::setprecision(3) << pt->x()  << " "
                     << std::setw(10) << std::setprecision(3) << pt->y() << " "
                     << std::setw(10) << std::setprecision(8) << pt->flux()  << "\n";
            }
        }

        width = 0;
        for (pt = itsSrcCatalogue.fullPointList().begin();
                pt < itsSrcCatalogue.fullPointList().end();
                pt++) {
            width = std::max(width, pt->ID().size());
        }

        for (pt = itsSrcCatalogue.fullPointList().begin();
                pt < itsSrcCatalogue.fullPointList().end();
                pt++) {

            bool isMatch = false;

            for (match = itsMatchingPixList.begin();
                    match < itsMatchingPixList.end() && !isMatch;
                    match++) {
                isMatch = (pt->ID() == match->first.ID());
            }

            if (!isMatch) {
                fout << "S "
                     << std::setw(width) << pt->ID() << " "
                     << std::setw(10) << std::setprecision(3) << pt->x()  << " "
                     << std::setw(10) << std::setprecision(3) << pt->y()  << " "
                     << std::setw(10) << std::setprecision(8) << pt->flux() << "\n";
            }
        }
    } else {
        ASKAPLOG_ERROR_STR(logger, "Could not open miss file " << itsMissFile);
    }

}

//**************************************************************//

void CatalogueMatcher::outputSummary()
{

    if (itsSourceSummaryFile != "") {
        this->outputSummary(itsSrcCatalogue, itsSourceSummaryFile);
    }

    if (itsReferenceSummaryFile != "") {
        this->outputSummary(itsRefCatalogue, itsReferenceSummaryFile);
    }

}

void CatalogueMatcher::outputSummary(PointCatalogue &cat, std::string filename)
{
    size_t width = 0;
    std::string matchID;
    std::vector<Point>::iterator pt;
    std::vector<std::pair<Point, Point> >::iterator mpair;
    std::vector<std::pair<Point, Point> >::iterator match;

    for (match = itsMatchingPixList.begin();
            match < itsMatchingPixList.end();
            match++) {
        width = std::max(width, match->first.ID().size());
        width = std::max(width, match->second.ID().size());
    }

    std::ofstream fout(filename.c_str());
    if (fout.is_open()) {
        for (pt = cat.fullPointList().begin();
                pt < cat.fullPointList().end();
                pt++) {

            bool isMatch = false;
            for (mpair = itsMatchingPixList.begin();
                    mpair < itsMatchingPixList.end() && !isMatch;
                    mpair++) {
                isMatch = (pt->ID() == mpair->second.ID());
                matchID = isMatch ? mpair->first.ID() : "---";
            }
            fout << std::setw(width) << pt->ID() << " "
                 << std::setw(width) << matchID << " "
                 << std::setw(10) << std::setprecision(7) << pt->x()  << " "
                 << std::setw(10) << std::setprecision(7) << pt->y() << " "
                 << std::setw(10) << std::setprecision(8) << pt->flux()  << "\n";
        }
        fout.close();
    } else {
        ASKAPLOG_ERROR_STR(logger, "Could not open summary file " << filename);
    }

}



}
}
}
