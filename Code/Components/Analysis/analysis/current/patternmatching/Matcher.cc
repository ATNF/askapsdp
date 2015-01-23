/// @file
///
/// Provides base class for handling the matching of lists of points
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
/// @author Matthew Whiting <matthew.whiting@csiro.au>
///
#include <askap_analysis.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <patternmatching/Matcher.h>
#include <patternmatching/Triangle.h>
#include <patternmatching/Point.h>
#include <patternmatching/MatchingUtilities.h>

#include <Common/ParameterSet.h>

#include <duchamp/fitsHeader.hh>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <utility>
#include <string>
#include <math.h>

///@brief Where the log messages go.
ASKAP_LOGGER(logger, ".matching");

using namespace askap;

namespace askap {

namespace analysis {

namespace matching {

Matcher::Matcher()
{
    itsMeanDx = 0.;
    itsMeanDy = 0.;
    itsRmsDx = 0.;
    itsRmsDy = 0.;
}

Matcher::~Matcher()
{
}

Matcher::Matcher(const Matcher &m)
{
    operator=(m);
}

Matcher& Matcher::operator=(const Matcher &m)
{
    if (this == &m) return *this;

    itsFITShead = m.itsFITShead;
    itsSrcFile = m.itsSrcFile;
    itsRefFile = m.itsRefFile;
    itsRA = m.itsRA;
    itsDec = m.itsDec;
    itsSrcPosType = m.itsSrcPosType;
    itsRefPosType = m.itsRefPosType;
    itsRadius = m.itsRadius;
    itsFluxMethod = m.itsFluxMethod;
    itsFluxUseFit = m.itsFluxUseFit;
    itsSrcPixList = m.itsSrcPixList;
    itsRefPixList = m.itsRefPixList;
    itsSrcTriList = m.itsSrcTriList;
    itsRefTriList = m.itsRefTriList;
    itsMatchingTriList = m.itsMatchingTriList;
    itsMatchingPixList = m.itsMatchingPixList;
    itsEpsilon = m.itsEpsilon;
    itsTrimSize = m.itsTrimSize;
    itsMeanDx = m.itsMeanDx;
    itsMeanDy = m.itsMeanDy;
    itsRmsDx = m.itsRmsDx;
    itsRmsDy = m.itsRmsDy;
    itsNumMatch1 = m.itsNumMatch1;
    itsNumMatch2 = m.itsNumMatch2;
    itsSenseMatch = m.itsSenseMatch;
    itsOutputBestFile = m.itsOutputBestFile;
    itsOutputMissFile = m.itsOutputMissFile;
    return *this;
}

Matcher::Matcher(const LOFAR::ParameterSet& parset)
{
    itsSrcFile = parset.getString("srcFile", "");
    itsRefFile = parset.getString("refFile", "");
    itsFluxMethod = parset.getString("fluxMethod", "peak");
    itsFluxUseFit = parset.getString("fluxUseFit", "best");
    itsRA  = parset.getString("RA", "00:00:00");
    itsDec = parset.getString("Dec", "00:00:00");
    itsSrcPosType = parset.getString("srcPosType", "deg");
    itsRefPosType = parset.getString("refPosType", "deg");
    itsRadius = parset.getDouble("radius", -1.);
    itsEpsilon = parset.getDouble("epsilon", defaultEpsilon);
    itsTrimSize = parset.getInt16("trimsize", matching::maxSizePointList);
    itsMeanDx = 0.;
    itsMeanDy = 0.;
    itsRmsDx = 0.;
    itsRmsDy = 0.;
    itsOutputBestFile = parset.getString("matchFile", "matches.txt");
    itsOutputMissFile = parset.getString("missFile", "misses.txt");
}

//**************************************************************//

void Matcher::setHeader(duchamp::FitsHeader &head)
{
    itsFITShead = head;
}

//**************************************************************//

void Matcher::readLists()
{

    bool filesOK = true;

    if (itsSrcFile == "") {
        ASKAPTHROW(AskapError, "srcFile not defined. Cannot get pixel list!");
        filesOK = false;
    }

    if (itsRefFile == "") {
        ASKAPTHROW(AskapError, "refFile not defined. Cannot get pixel list!");
        filesOK = false;
    }

    if (filesOK) {
        std::ifstream fsrc(itsSrcFile.c_str());

        if (!fsrc.is_open()) {
            ASKAPTHROW(AskapError,
                       "srcFile (" << itsSrcFile << ") not valid. Error opening file.");
        }

        std::ifstream fref(itsRefFile.c_str());

        if (!fref.is_open()) {
            ASKAPTHROW(AskapError,
                       "refFile (" << itsRefFile << ") not valid. Error opening file.");
        }

        itsSrcPixList = getSrcPixList(fsrc, itsFITShead,
                                            itsRA, itsDec,
                                            itsSrcPosType, itsRadius,
                                            itsFluxMethod, itsFluxUseFit);
        ASKAPLOG_INFO_STR(logger, "Size of source pixel list = " << itsSrcPixList.size());

        itsRefPixList = getPixList(fref, itsFITShead,
                                         itsRA, itsDec,
                                         itsRefPosType, itsRadius);
        ASKAPLOG_INFO_STR(logger, "Size of reference pixel list = " << itsRefPixList.size());
    } else {
        ASKAPLOG_WARN_STR(logger, "Not reading any pixel lists!");
    }


}

//**************************************************************//

void Matcher::fixRefList(std::vector<float> beam)
{
    // ASKAPLOG_INFO_STR(logger, "Beam info being used: maj=" << beam[0]*3600.
    //                       << ", min=" << beam[1]*3600. << ", pa=" << beam[2]);
    // float a1 = std::max(beam[0] * 3600., beam[1] * 3600.);
    // float b1 = std::min(beam[0] * 3600., beam[1] * 3600.);
    // float pa1 = beam[2];
    // float d1 = a1 * a1 - b1 * b1;
    // std::vector<Point>::iterator pix = itsRefPixList.begin();

    // for (; pix < itsRefPixList.end(); pix++) {
    //     double a2 = std::max(pix->majorAxis(), pix->minorAxis());
    //     double b2 = std::min(pix->majorAxis(), pix->minorAxis());
    //     double pa2 = pix->PA();
    //     double d2 = a2 * a2 - b2 * b2;
    //     double d0sq = d1 * d1 + d2 * d2 + 2. * d1 * d2 * cos(2.*(pa1 - pa2));
    //     double d0 = sqrt(d0sq);
    //     double a0sq = 0.5 * (a1 * a1 + b1 * b1 + a2 * a2 + b2 * b2 + d0);
    //     double b0sq = 0.5 * (a1 * a1 + b1 * b1 + a2 * a2 + b2 * b2 - d0);
    //     pix->setMajorAxis(sqrt(a0sq));
    //     pix->setMinorAxis(sqrt(b0sq));

    //     if (d0sq > 0) {
    //         // leave out normalisation by d0, since we will take ratios to get tan2pa0
    //         double sin2pa0 = (d1 * sin(2.*pa1) + d2 * sin(2.*pa2));
    //         double cos2pa0 = (d1 * cos(2.*pa1) + d2 * cos(2.*pa2));
    //         double pa0 = atan(fabs(sin2pa0 / cos2pa0));

    //         // atan of the absolute value of the ratio returns a value between 0 and 90 degrees.
    //         // Need to correct the value of l according to the correct quandrant it is in.
    //         // This is worked out using the signs of sinl and cosl
    //         if (sin2pa0 > 0) {
    //             if (cos2pa0 > 0) pa0 = pa0;
    //             else          pa0 = M_PI - pa0;
    //         } else {
    //             if (cos2pa0 > 0) pa0 = 2.*M_PI - pa0;
    //             else          pa0 = M_PI + pa0;
    //         }

    //         pix->setPA(pa0 / 2.);
    //     } else  pix->setPA(0.);

    // }
}


//**************************************************************//

void Matcher::setTriangleLists()
{
    std::vector<Point> srclist = trimList(itsSrcPixList, itsTrimSize);
    ASKAPLOG_INFO_STR(logger, "Trimmed src list to " << srclist.size() << " points");
    // std::vector<Point> reflist = trimList(itsRefPixList, itsTrimSize);
    // ASKAPLOG_INFO_STR(logger, "Trimmed ref list to " << reflist.size() << " points");

    itsSrcTriList = getTriList(srclist);

    ASKAPLOG_INFO_STR(logger, "Performing crude match on reference list");
    std::vector<Point> newreflist = crudeMatchList(itsRefPixList, itsSrcPixList, 5);

    // ASKAPLOG_INFO_STR(logger, "Performing crude match on trimmed reference list");
    // std::vector<Point> newreflist = crudeMatchList(itsRefPixList, srclist,5);
    ASKAPLOG_INFO_STR(logger, "Now have reference list of size " <<
                      newreflist.size() << " points");
    newreflist = trimList(newreflist, itsTrimSize);
    ASKAPLOG_INFO_STR(logger, "Reference list trimmed to " <<
                      newreflist.size() << " points");

    //                itsRefTriList = getTriList(reflist);
    itsRefTriList = getTriList(newreflist);
    itsMatchingTriList = matchLists(itsSrcTriList,
                                          itsRefTriList,
                                          itsEpsilon);
    trimTriList(itsMatchingTriList);
    ASKAPLOG_INFO_STR(logger, "Found " << itsMatchingTriList.size() << " matches\n");
}

//**************************************************************//

void Matcher::findMatches()
{
    itsNumMatch1 = 0;

    if (itsMatchingTriList.size() > 0) {
        itsMatchingPixList = vote(itsMatchingTriList);
        itsNumMatch1 = itsMatchingPixList.size();
        ASKAPLOG_INFO_STR(logger, "After voting, have found " <<
                          itsMatchingPixList.size() << " matching points\n");

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

void Matcher::findOffsets()
{
    std::vector<double> dx(itsNumMatch1, 0.), dy(itsNumMatch1, 0.);

    for (int i = 0; i < itsNumMatch1; i++) {
        if (itsSenseMatch) {
            dx[i] = itsMatchingPixList[i].first.x() - itsMatchingPixList[i].second.x();
            dy[i] = itsMatchingPixList[i].first.y() - itsMatchingPixList[i].second.y();
        } else {
            dx[i] = itsMatchingPixList[i].first.x() - itsMatchingPixList[i].second.x();
            dy[i] = itsMatchingPixList[i].first.y() + itsMatchingPixList[i].second.y();
        }

    }

    itsMeanDx = itsMeanDy = 0.;

    for (int i = 0; i < itsNumMatch1; i++) {
        itsMeanDx += dx[i];
        itsMeanDy += dy[i];
    }

    itsMeanDx /= double(itsNumMatch1);
    itsMeanDy /= double(itsNumMatch1);

    itsRmsDx = itsRmsDy = 0.;

    for (int i = 0; i < itsNumMatch1; i++) {
        itsRmsDx += (dx[i] - itsMeanDx) * (dx[i] - itsMeanDx);
        itsRmsDy += (dy[i] - itsMeanDy) * (dy[i] - itsMeanDy);
    }

    itsRmsDx = sqrt(itsRmsDx / (double(itsNumMatch1 - 1)));
    itsRmsDy = sqrt(itsRmsDy / (double(itsNumMatch1 - 1)));
    std::stringstream ss;
    ss.setf(std::ios::fixed);
    ss << "Offsets between the two are dx = " << itsMeanDx << " +- " << itsRmsDx
       << " dy = " << itsMeanDy << " +- " << itsRmsDy;
    ASKAPLOG_INFO_STR(logger, ss.str());
}

//**************************************************************//

void Matcher::addNewMatches()
{

    if (itsNumMatch1 > 0) {

        this->rejectMultipleMatches();
        const float matchRadius = 3.;
        std::vector<Point>::iterator src, ref;
        std::vector<std::pair<Point, Point> >::iterator match;

        for (src = itsSrcPixList.begin(); src < itsSrcPixList.end(); src++) {
            bool isMatch = false;
            match = itsMatchingPixList.begin();

            for (; match < itsMatchingPixList.end() && !isMatch; match++) {
                isMatch = (src->ID() == match->first.ID());
            }

            if (!isMatch) {
                float minOffset = 0.;
                int minRef = -1;

                for (ref = itsRefPixList.begin(); ref < itsRefPixList.end(); ref++) {
                    float offset = hypot(src->x() - ref->x() - itsMeanDx,
                                         src->y() - ref->y() - itsMeanDy);

                    if (offset < matchRadius * itsEpsilon) {
                        if ((minRef == -1) || (offset < minOffset)) {
                            minOffset = offset;
                            minRef = int(ref - itsRefPixList.begin());
                        }
                    }
                }

                if (minRef >= 0) { // there was a match within errors
                    ref = itsRefPixList.begin() + minRef;
                    std::pair<Point, Point> newMatch(*src, *ref);
                    itsMatchingPixList.push_back(newMatch);
                }
            }
        }

        this->rejectMultipleMatches();
        itsNumMatch2 = itsMatchingPixList.size();
    }
}

//**************************************************************//

void Matcher::rejectMultipleMatches()
{
    if (itsMatchingPixList.size() < 2) return;

    std::vector<std::pair<Point, Point> >::iterator alice, bob;
    alice = itsMatchingPixList.begin();

    while (alice < itsMatchingPixList.end() - 1) {
        bool bobGone = false;
        bool aliceGone = false;
        bob = alice + 1;

        while (bob < itsMatchingPixList.end() && !aliceGone) {
            if (alice->second.ID() == bob->second.ID()) {
                // alice & bob have the same reference source
                float df_alice, df_bob;

                // if (itsFluxMethod == "integrated") {
                //     df_alice = alice->first.stuff().flux() - alice->second.flux();
                //     df_bob   = bob->first.stuff().flux() - bob->second.flux();
                // } else {
                df_alice = alice->first.flux() - alice->second.flux();
                df_bob   = bob->first.flux() - bob->second.flux();
                // }

                if (fabs(df_alice) < fabs(df_bob)) {
                    // delete bob
                    itsMatchingPixList.erase(bob);
                    bobGone = true;
                } else {
                    // delete alice
                    itsMatchingPixList.erase(alice);
                    aliceGone = true;
                }
            }

            if (!bobGone) bob++;
            else bobGone = false;
        }

        if (!aliceGone) alice++;
    }
}



//**************************************************************//

void Matcher::outputMatches()
{
    std::ofstream fout(itsOutputBestFile.c_str());
    std::vector<std::pair<Point, Point> >::iterator match;
    int prec = 3;

    for (match = itsMatchingPixList.begin();
            match < itsMatchingPixList.end();
            match++) {
        // if (itsFluxMethod == "integrated") { // need to swap around since we have initially stored peak flux in object.
        //     float tmpflux;
        //     tmpflux = match->first.stuff().flux();
        //     match->first.stuff().setFlux(match->first.flux());
        //     match->first.setFlux(tmpflux);
        // }

        int newprec = int(ceil(log10(1. / match->first.flux()))) + 1;
        prec = std::max(prec, newprec);
        newprec = int(ceil(log10(1. / match->first.flux()))) + 1;
        prec = std::max(prec, newprec);
    }

    fout.setf(std::ios::fixed);
    int ct = 0;
    char matchType;

    for (match = itsMatchingPixList.begin();
            match < itsMatchingPixList.end();
            match++) {

        if (ct++ < itsNumMatch1) matchType = '1';
        else matchType = '2';

        fout << matchType << "\t" << match->first.ID() << " " << match->second.ID() << " " <<
             std::setw(8)  << std::setprecision(3) << match->first.sep(match->second) << "\n";

        // fout << matchType << "\t"
        //     << "[" << match->first.ID() << "]\t"
        //     << std::setw(10) << std::setprecision(3) << match->first.x()  << " "
        //     << std::setw(10) << std::setprecision(3) << match->first.y()  << " "
        //     << std::setw(10) << std::setprecision(8) << match->first.flux() << " "
        //     << std::setw(10) << std::setprecision(3) << match->first.majorAxis() << " "
        //     << std::setw(10) << std::setprecision(3) << match->first.minorAxis() << " "
        //     << std::setw(10) << std::setprecision(3) << match->first.PA()  << " "
        //   << std::setw(10) << std::setprecision(3) << match->first.alpha() << " "
        //   << std::setw(10) << std::setprecision(3) << match->first.beta() << " "
        //     << std::setw(10) << match->first.stuff() << "\t"
        //     << "[" << match->second.ID() << "]\t"
        //     << std::setw(10) << std::setprecision(3) << match->second.x()  << " "
        //     << std::setw(10) << std::setprecision(3) << match->second.y()  << " "
        //     << std::setw(10) << std::setprecision(8) << match->second.flux() << " "
        //     << std::setw(10) << std::setprecision(3) << match->second.majorAxis() << " "
        //     << std::setw(10) << std::setprecision(3) << match->second.minorAxis() << " "
        //     << std::setw(10) << std::setprecision(3) << match->second.PA() << "\t"
        //   << std::setw(10) << std::setprecision(3) << match->second.alpha() << " "
        //   << std::setw(10) << std::setprecision(3) << match->second.beta() << " "
        //     << std::setw(8)  << std::setprecision(3) << match->first.sep(match->second) << " "
        //     << std::setw(8)  << std::setprecision(8) << match->first.flux() - match->second.flux() << " "
        //     << std::setw(8)  << std::setprecision(6) << (match->first.flux() - match->second.flux()) / match->second.flux() << "\n";
    }

    fout.close();
}

//**************************************************************//

void Matcher::outputMisses()
{
    std::ofstream fout(itsOutputMissFile.c_str());
    fout.setf(std::ios::fixed);
    std::vector<Point>::iterator pt;
    std::vector<std::pair<Point, Point> >::iterator match;
    //                Stuff nullstuff(0., 0., 0., 0, 0, 0, 0, 0.);

    for (pt = itsRefPixList.begin(); pt < itsRefPixList.end(); pt++) {
        bool isMatch = false;
        match = itsMatchingPixList.begin();

        for (; match < itsMatchingPixList.end() && !isMatch; match++) {
            isMatch = (pt->ID() == match->second.ID());
        }

        if (!isMatch) {
            fout << "R\t[" << pt->ID() << "]\t"
                 << std::setw(10) << std::setprecision(3) << pt->x()  << " "
                 << std::setw(10) << std::setprecision(3) << pt->y() << " "
                 << std::setw(10) << std::setprecision(8) << pt->flux()  << " "
                 // << std::setw(10) << std::setprecision(3) << pt->majorAxis() << " "
                 // << std::setw(10) << std::setprecision(3) << pt->minorAxis() << " "
                 // << std::setw(10) << std::setprecision(3) << pt->PA()  << " "
                 // << std::setw(10) << nullstuff
                 << "\n";
        }
    }

    for (pt = itsSrcPixList.begin(); pt < itsSrcPixList.end(); pt++) {
        bool isMatch = false;
        match = itsMatchingPixList.begin();

        for (; match < itsMatchingPixList.end() && !isMatch; match++) {
            isMatch = (pt->ID() == match->first.ID());
        }

        if (!isMatch) {
            fout << "S\t[" << pt->ID() << "]\t"
                 << std::setw(10) << std::setprecision(3) << pt->x()  << " "
                 << std::setw(10) << std::setprecision(3) << pt->y()  << " "
                 << std::setw(10) << std::setprecision(8) << pt->flux() << " "
                 // << std::setw(10) << std::setprecision(3) << pt->majorAxis() << " "
                 // << std::setw(10) << std::setprecision(3) << pt->minorAxis() << " "
                 // << std::setw(10) << std::setprecision(3) << pt->PA()  << " "
                 // << std::setw(10) << pt->stuff()
                 << "\n";
        }
    }

}


void Matcher::outputSummary()
{
    std::ofstream fout;
    //                Stuff nullstuff(0., 0., 0., 0, 0, 0, 0, 0.);

    std::vector<Point>::iterator pt;
    std::vector<std::pair<Point, Point> >::iterator mpair;
    Point match;

    fout.open("match-summary-sources.txt");
    for (pt = itsSrcPixList.begin(); pt < itsSrcPixList.end(); pt++) {
        bool isMatch = false;
        for (mpair = itsMatchingPixList.begin();
                mpair < itsMatchingPixList.end() && !isMatch;
                mpair++) {
            isMatch = (pt->ID() == mpair->first.ID());
            if (isMatch) {
                match = mpair->second;
            }
        }
        std::string matchID = isMatch ? match.ID() : "---";
        fout << pt->ID() << " " << matchID << "\t"
             << std::setw(10) << std::setprecision(3) << pt->x()  << " "
             << std::setw(10) << std::setprecision(3) << pt->y()  << " "
             << std::setw(10) << std::setprecision(8) << pt->flux() << " "
             // << std::setw(10) << std::setprecision(3) << pt->majorAxis() << " "
             // << std::setw(10) << std::setprecision(3) << pt->minorAxis() << " "
             // << std::setw(10) << std::setprecision(3) << pt->PA()  << " "
             // << std::setw(10) << pt->stuff()
             << "\n";
    }
    fout.close();

    fout.open("match-summary-reference.txt");
    for (pt = itsRefPixList.begin(); pt < itsRefPixList.end(); pt++) {
        bool isMatch = false;
        for (mpair = itsMatchingPixList.begin();
                mpair < itsMatchingPixList.end() && !isMatch;
                mpair++) {
            isMatch = (pt->ID() == mpair->second.ID());
            if (isMatch) {
                match = mpair->first;
            }
        }
        std::string matchID = isMatch ? match.ID() : "---";
        fout << pt->ID() << " " << matchID << "\t"
             << std::setw(10) << std::setprecision(3) << pt->x()  << " "
             << std::setw(10) << std::setprecision(3) << pt->y() << " "
             << std::setw(10) << std::setprecision(8) << pt->flux()  << " "
             // << std::setw(10) << std::setprecision(3) << pt->majorAxis() << " "
             // << std::setw(10) << std::setprecision(3) << pt->minorAxis() << " "
             // << std::setw(10) << std::setprecision(3) << pt->PA()  << " "
             // << std::setw(10) << nullstuff
             << "\n";
    }
    fout.close();

}



}

}

}

