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

#include <patternmatching/MatchingUtilities.h>
#include <patternmatching/Triangle.h>
#include <patternmatching/Point.h>
#include <patternmatching/Matcher.h>

#include <coordutils/PositionUtilities.h>

#include <duchamp/fitsHeader.hh>
#include <duchamp/Utils/utils.hh>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <string>
#include <math.h>

///@brief Where the log messages go.
ASKAP_LOGGER(logger, ".matching");

namespace askap {

namespace analysis {

namespace matching {

std::vector<matching::Point>
getSrcPixList(std::ifstream &fin, duchamp::FitsHeader &header, std::string raBaseStr,
              std::string decBaseStr, std::string posType, double radius,
              std::string fluxMethod, std::string fluxUseFit)
{
    std::vector<matching::Point> pixlist;
    std::string raS, decS, sdud, id, name;
    double peakflux, iflux1, iflux2, pflux1, pflux2;
    double maj, min, pa, majD, minD, paD, chisq, rmsfit, noise, alpha, beta;
    int nfree, ndof, npixfit, npixobj, guess;

    // Convert the base position
    double ra, dec, zworld;
    ra = analysisutilities::dmsToDec(raBaseStr) * 15.;
    dec = analysisutilities::dmsToDec(decBaseStr);
    zworld = header.specToVel(0.);
    double xBase, yBase, zBase, x, y, z;
    header.wcsToPix(ra, dec, zworld, xBase, yBase, zBase);

    std::string line;

    ASKAPLOG_DEBUG_STR(logger, "About to read source pixel list");

    // now at start of object list
    while (getline(fin, line),
            !fin.eof()) {

        if (line[0] != '#') {

            std::stringstream inputline(line);

            inputline >> id >> name >> raS >> decS
                      >> iflux1 >> pflux1 >> iflux2 >> pflux2
                      >> maj >> min >> pa >> majD >> minD >> paD
                      >> alpha >> beta >> chisq >> noise >> rmsfit
                      >> nfree >> ndof >> npixfit >> npixobj >> guess;

            if (fluxUseFit == "no") {
                peakflux = pflux1;
            } else if (fluxUseFit == "yes") {
                peakflux = pflux2;
            } else {
                //NOTE: was if(fluxUseFit=="best") but taking "best" to be the default

                if (pflux2 > 0) {
                    peakflux = pflux2;
                } else {
                    peakflux = pflux1;
                }
            }

            id += "_" + name;

            std::stringstream ss;

            if (posType == "dms") {
                ra = analysisutilities::dmsToDec(raS) * 15.;
                dec = analysisutilities::dmsToDec(decS);
            } else if (posType == "deg") {
                ra = atof(raS.c_str());
                dec = atof(decS.c_str());
            } else {
                ASKAPTHROW(AskapError, "Unknown position type in getSrcPixList: " << posType);
            }

            if (header.wcsToPix(ra, dec, zworld, x, y, z)) {
                ASKAPLOG_ERROR_STR(logger,
                                   "getSrcPixList: Conversion error... source ID=" << id <<
                                   ": " << std::setprecision(6) << ra << " --> " << x <<
                                   " and " << std::setprecision(6) << dec << " --> " << y);
            }

            if (radius < 0 || (radius > 0 && hypot(x - xBase, y - yBase) < radius * 60.)) {
                matching::Point pt(xBase, yBase, peakflux, id);
                pixlist.push_back(pt);
            }

        }

    }

    stable_sort(pixlist.begin(), pixlist.end());
    reverse(pixlist.begin(), pixlist.end());
    return pixlist;
}

std::vector<matching::Point> getPixList(std::ifstream &fin, duchamp::FitsHeader &header,
                                        std::string raBaseStr, std::string decBaseStr,
                                        std::string posType, double radius)
{
    std::vector<matching::Point> pixlist;
    std::string line, raS, decS, id;
    double flux, maj, min, pa, alpha, beta;
    int ct = 1;

    // Convert the base position
    double ra, dec, zworld;
    ra = analysisutilities::dmsToDec(raBaseStr) * 15.;
    dec = analysisutilities::dmsToDec(decBaseStr);
    zworld = header.specToVel(0.);

    double xBase, yBase, zBase, x, y, z;
    header.wcsToPix(ra, dec, zworld, xBase, yBase, zBase);

    while (getline(fin, line),
            !fin.eof()) {
        if (line[0] != '#') {
            std::stringstream ss(line);
            ss >> raS >> decS >> flux >> alpha >> beta >> maj >> min >> pa;

            if (posType == "dms") {
                ra = analysisutilities::dmsToDec(raS) * 15.;
                dec = analysisutilities::dmsToDec(decS);
            } else if (posType == "deg") {
                ra = atof(raS.c_str());
                dec = atof(decS.c_str());
            } else
                ASKAPTHROW(AskapError, "Unknown position type in getRefPixList: " << posType);

            std::stringstream idString;
            idString << ct++ << "_"
                     << analysisutilities::decToDMS(ra, "RA") << "_"
                     << analysisutilities::decToDMS(dec, "DEC");

            if (header.wcsToPix(ra, dec, zworld, x, y, z)) {
                ASKAPLOG_ERROR_STR(logger, "getPixList: Conversion error... source ID=" <<
                                   idString.str() << ", wld=(" <<
                                   std::setprecision(6) << ra << "," <<
                                   std::setprecision(6) << dec <<
                                   "), line = " << line);
            }

            if (radius < 0 || (radius > 0 && hypot(x - xBase, y - yBase) < radius * 60.)) {
                matching::Point pt(x, y, flux, idString.str());
                pixlist.push_back(pt);
            }
        }
    }

    stable_sort(pixlist.begin(), pixlist.end());
    reverse(pixlist.begin(), pixlist.end());
    return pixlist;
}

std::vector<matching::Point>
getSrcPixList(std::ifstream &fin, std::string raBaseStr, std::string decBaseStr,
              std::string posType, double radius,
              std::string fluxMethod, std::string fluxUseFit)
{
    std::vector<matching::Point> pixlist;
    std::string raS, decS, sdud, id, name;
    double raBase = analysisutilities::dmsToDec(raBaseStr) * 15.;
    double decBase = analysisutilities::dmsToDec(decBaseStr);
    double xpt, ypt, ra, dec, peakflux, iflux1, iflux2, pflux1, pflux2;
    double maj, min, pa, majD, minD, paD, chisq, rmsfit, noise, alpha, beta;
    int nfree, ndof, npixfit, npixobj, guess;
    std::string line;

    ASKAPLOG_DEBUG_STR(logger, "About to read source pixel list");

    // now at start of object list
    while (getline(fin, line),
            !fin.eof()) {

        if (line[0] != '#') {

            std::stringstream inputline(line);

            inputline >> id >> name >> raS >> decS
                      >> iflux1 >> pflux1 >> iflux2 >> pflux2
                      >> maj >> min >> pa >> majD >> minD >> paD
                      >> alpha >> beta >> chisq >> noise >> rmsfit
                      >> nfree >> ndof >> npixfit >> npixobj >> guess;

            if (fluxUseFit == "no") {
                peakflux = pflux1;
            } else if (fluxUseFit == "yes") {
                peakflux = pflux2;
            } else {
                //NOTE: was if(fluxUseFit=="best") but taking "best" to be the default
                if (pflux2 > 0) peakflux = pflux2;
                else peakflux = pflux1;
            }

            id += "_" + name;
            ASKAPLOG_DEBUG_STR(logger, id << " " << peakflux);

            std::stringstream ss;

            if (posType == "dms") {
                ra = analysisutilities::dmsToDec(raS) * 15.;
                dec = analysisutilities::dmsToDec(decS);
            } else if (posType == "deg") {
                ra = atof(raS.c_str());
                dec = atof(decS.c_str());
            } else {
                ASKAPTHROW(AskapError, "Unknown position type in getSrcPixList: " << posType);
            }

            xpt = analysisutilities::angularSeparation(ra, decBase, raBase, decBase) * 3600.;

            if (ra > raBase) xpt *= -1.;

            ypt = (dec - decBase) * 3600.;

            if (radius < 0 || (radius > 0 && hypot(xpt, ypt) < radius * 60.)) {
                matching::Point pix(xpt, ypt, peakflux, id);
                pixlist.push_back(pix);
            }
        }

    }

    stable_sort(pixlist.begin(), pixlist.end());
    reverse(pixlist.begin(), pixlist.end());
    return pixlist;
}

std::vector<matching::Point>
getPixList(std::ifstream &fin, std::string raBaseStr, std::string decBaseStr,
           std::string posType, double radius)
{

    std::vector<matching::Point> pixlist;
    std::string line, raS, decS, id;
    double raBase = analysisutilities::dmsToDec(raBaseStr) * 15.;
    double decBase = analysisutilities::dmsToDec(decBaseStr);
    double ra, dec, xpt, ypt, flux, maj, min, pa, alpha, beta;
    int ct = 1;

    while (getline(fin, line),
            !fin.eof()) {
        if (line[0] != '#') {
            std::stringstream ss(line);
            ss >> raS >> decS >> flux >> alpha >> beta >> maj >> min >> pa;

            if (posType == "dms") {
                ra = analysisutilities::dmsToDec(raS) * 15.;
                dec = analysisutilities::dmsToDec(decS);
            } else if (posType == "deg") {
                ra = atof(raS.c_str());
                dec = atof(decS.c_str());
            } else {
                ASKAPTHROW(AskapError, "Unknown position type in getRefPixList: " << posType);
            }

            std::stringstream idString;
            idString << ct++ << "_"
                     << analysisutilities::decToDMS(ra, "RA") << "_"
                     << analysisutilities::decToDMS(dec, "DEC");

            xpt = analysisutilities::angularSeparation(ra, decBase, raBase, decBase) * 3600.;

            if (ra > raBase) xpt *= -1.;

            ypt = (dec - decBase) * 3600.;

            if (radius < 0 || (radius > 0 && hypot(xpt, ypt) < radius * 60.)) {
                matching::Point pix(xpt, ypt, flux, idString.str());
                pixlist.push_back(pix);
            }
        }
    }

    stable_sort(pixlist.begin(), pixlist.end());
    reverse(pixlist.begin(), pixlist.end());
    return pixlist;
}


std::vector<matching::Point>
trimList(std::vector<matching::Point> &inputList, const unsigned int maxSize)
{
    std::vector<matching::Point> outList = inputList;
    std::sort(outList.begin(), outList.end()); // sort by flux, ascending order
    std::reverse(outList.begin(), outList.end());

    if (outList.size() > maxSize) {
        std::vector<matching::Point>::iterator pt = outList.begin() + maxSize;

        while (pt != outList.end()) {
            outList.erase(pt);
        }
    }

    return outList;
}


std::vector<matching::Point>
crudeMatchList(std::vector<matching::Point> &reflist,
               std::vector<matching::Point> &srclist,
               float maxOffset)
{
    std::vector<matching::Point>::iterator ref, src;
    std::vector<matching::Point> newreflist;
    for (src = srclist.begin(); src < srclist.end(); src++) {

        for (ref = reflist.begin(); ref < reflist.end(); ref++) {
            if (src->sep(*ref) < maxOffset) newreflist.push_back(*ref);
        }

    }

    return newreflist;

}

std::vector<Triangle> getTriList(std::vector<Point> &pixlist)
{
    std::vector<Triangle> triList;
    int npix = pixlist.size();

    for (int i = 0; i < npix - 2; i++) {
        for (int j = i + 1; j < npix - 1; j++) {
            for (int k = j + 1; k < npix; k++) {
                Triangle tri(pixlist[i], pixlist[j], pixlist[k]);

                if (tri.ratio() < 10.) triList.push_back(tri);
            }
        }
    }

    ASKAPLOG_INFO_STR(logger, "Generated a list of " << triList.size() << " triangles");
    return triList;
}

//**************************************************************//

std::vector<std::pair<Triangle, Triangle> >
matchLists(std::vector<Triangle> &list1, std::vector<Triangle> &list2, double epsilon)
{

    ASKAPLOG_INFO_STR(logger, "Commencing match between lists of size " <<
                      list1.size() << " and " << list2.size());

    size_t size1 = list1.size(), size2 = list2.size();
    // sort in order of increasing ratio
    std::stable_sort(list1.begin(), list1.end());
    std::stable_sort(list2.begin(), list2.end());
    // find maximum ratio tolerances for each list
    double maxTol1 = 0., maxTol2 = 0.;

    for (size_t i = 0; i < size1; i++) {
        list1[i].defineTolerances(epsilon);

        if (i == 0 || list1[i].ratioTol() > maxTol1) maxTol1 = list1[i].ratioTol();
    }

    for (size_t i = 0; i < size2; i++) {
        list2[i].defineTolerances(epsilon);

        if (i == 0 || list2[i].ratioTol() > maxTol2) maxTol2 = list2[i].ratioTol();
    }

    // std::vector<bool> matches(size1*size2, false);
    int nmatch = 0;
    std::vector<std::pair<Triangle, Triangle> > matchList;

    // loop over the lists, finding matches
    for (size_t i = 0; i < size1; i++) {
        double maxRatioB = list1[i].ratio() + sqrt(maxTol1 + maxTol2);
        double minRatioB = list1[i].ratio() - sqrt(maxTol1 + maxTol2);

        std::vector<Triangle> matches;
        for (size_t j = 0; j < size2 && list2[j].ratio() < maxRatioB; j++) {
            if (list2[j].ratio() > minRatioB && list1[i].isMatch(list2[j], epsilon)) {
                nmatch++;
                std::pair<Triangle, Triangle> match(list1[i], list2[j]);
                matchList.push_back(match);

            }
        }

    }

    ASKAPLOG_INFO_STR(logger, "Number of matching triangles = " << nmatch);

    return matchList;
}

//**************************************************************//

void trimTriList(std::vector<std::pair<Triangle, Triangle> > &trilist)
{
    unsigned int nIter = 0;
    unsigned int nSame = 0, nOpp = 0;
    const unsigned int maxIter = 5;

    do {

        double mean = 0., rms = 0., mag, sumx = 0., sumxx = 0.;
        size_t size = trilist.size();

        for (unsigned int i = 0; i < size; i++) {
            mag = trilist[i].first.perimeter() - trilist[i].second.perimeter();
            sumx += mag;
            sumxx += (mag * mag);
            if (trilist[i].first.isClockwise() == trilist[i].second.isClockwise()) nSame++;
            else nOpp++;
        }

        mean = sumx / double(size);
        rms = sqrt(sumxx / double(size) - mean * mean);

        double trueOnFalse = abs(nSame - nOpp) / double(nSame + nOpp - abs(nSame - nOpp));
        double scale;

        if (trueOnFalse < 1.) scale = 1.;
        else if (trueOnFalse > 10.) scale = 3.;
        else scale = 2.;

        ASKAPLOG_DEBUG_STR(logger, "Iteration #" << nIter <<
                           ": meanMag=" << mean << ", rmsMag=" << rms << ", scale=" << scale);

        std::vector<std::pair<Triangle, Triangle> > newlist;
        for (size_t i = 0; i < trilist.size(); i++) {
            mag = trilist[i].first.perimeter() - trilist[i].second.perimeter();
            if (fabs((mag - mean) / rms) < scale) {
                newlist.push_back(trilist[i]);
            }
        }
        trilist = newlist;
        ASKAPLOG_DEBUG_STR(logger, "List size now " << trilist.size());

        nIter++;
    } while (nIter < maxIter && trilist.size() > 0);

    for (unsigned int i = 0; i < trilist.size(); i++) {
        if (trilist[i].first.isClockwise() == trilist[i].second.isClockwise()) {
            nSame++;
        } else {
            nOpp++;
        }
    }

    std::vector<std::pair<Triangle, Triangle> > newlist;
    for (size_t i = 0; i < trilist.size(); i++) {
        if (((nSame <= nOpp) ||
                (trilist[i].first.isClockwise() == trilist[i].second.isClockwise())) &&
                ((nOpp <= nSame) ||
                 (trilist[i].first.isClockwise() != trilist[i].second.isClockwise()))) {

            newlist.push_back(trilist[i]);

        }
    }
    trilist = newlist;

}

//**************************************************************//

std::vector<std::pair<Point, Point> > vote(std::vector<std::pair<Triangle, Triangle> > &trilist)
{

    std::multimap<int, std::pair<Point, Point> > voteList;
    std::vector<std::pair<Point, Point> > pts;
    std::vector<int> votes;
    std::multimap<int, std::pair<Point, Point> >::iterator vote;
    std::multimap<int, std::pair<Point, Point> >::reverse_iterator rvote;

    for (unsigned int i = 0; i < trilist.size(); i++) {
        std::vector<Point> ptlist1 = trilist[i].first.getPtList();
        std::vector<Point> ptlist2 = trilist[i].second.getPtList();

        for (int p = 0; p < 3; p++) { // for each of the three points:
            bool foundMatch = false;
            if (votes.size() > 0) {

                for (size_t i = 0; i < votes.size() && !foundMatch; i++) {
                    if ((pts[i].first.ID() == ptlist1[p].ID()) &&
                            (pts[i].second.ID() == ptlist2[p].ID())) {
                        votes[i]++;
                        foundMatch = true;
                    }
                }

            }

            if (!foundMatch) {
                votes.push_back(1);
                pts.push_back(std::pair<Point, Point>(ptlist1[p], ptlist2[p]));
            }

        }
    }

    for (size_t i = 0; i < votes.size(); i++) {
        voteList.insert(std::pair<int, std::pair<Point, Point> >(votes[i], pts[i]));
    }

    std::vector<std::pair<Point, Point> > outlist;

    if (voteList.rbegin()->first == 1) {
        // largest vote was 1 -- no match;
        return outlist;
    }

    bool stop = false;
    int prevVote = voteList.rbegin()->first;

    for (rvote = voteList.rbegin(); rvote != voteList.rend() && !stop; rvote++) {

        for (unsigned int i = 0; i < outlist.size() && !stop; i++) {
            stop = ((rvote->second.first.ID() == outlist[i].first.ID()));
        }

        if (rvote != voteList.rbegin()) {
            stop = stop || (rvote->first < 0.5 * prevVote);
        }

        if (!stop) {
            outlist.push_back(rvote->second);
        }

        prevVote = rvote->first;
    }

    return outlist;
}


}

}

}
