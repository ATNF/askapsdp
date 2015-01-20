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
#ifndef ASKAP_ANALYSIS_CATMATCHER_H_
#define ASKAP_ANALYSIS_CATMATCHER_H_

#include <patternmatching/Triangle.h>
#include <patternmatching/Point.h>
#include <patternmatching/PointCatalogue.h>

#include <Common/ParameterSet.h>
#include <casa/Quanta.h>

#include <vector>
#include <utility>
#include <string>
#include <math.h>

namespace askap {

namespace analysis {

namespace matching {

class CatalogueMatcher {
    public:
        CatalogueMatcher();
        CatalogueMatcher(const LOFAR::ParameterSet& parset);
        virtual ~CatalogueMatcher() {};

        /// @brief Change epsilon to pixel units
        void convertEpsilon();

        /// @brief Read in the lists of source and reference objects.
        /// @details This function reads the source and reference
        /// pixel lists from the files provided. Checks are made
        /// for the validity of the files.
        bool read();

        /// @brief Define the triangle lists from the (existing) point
        /// lists, and find matching triangles.
        void setTriangleLists();

        /// @brief Find the points in each list that match.
        /// @details Matching points are found via the Groth voting
        /// function vote(). The number of matches and their sense are
        /// recorded.
        void findMatches();

        /// @brief Find matching points assuming no significant net
        /// spatial offset
        /// @details Matches the lists on the assumption that
        /// there is no spatial offset between them, and we can
        /// just do the "crude" matching of points within the
        /// epsilon radius. Works down the lists starting with the
        /// brightest and identifies matches.
        void zeroOffsetMatch();

        /// @brief Find the linear offsets between the two lists of
        /// points.
        /// @details The mean and rms offsets in the x- and
        /// y-directions are measured for the matching points.
        // std::vector<double> dx(itsNumMatch1,0.),dy(itsNumMatch1,0.);
        void findOffsets();

        /// @brief Using the known offsets, find matches that were
        /// missed by the pattern matching.
        /// @details The source point list is scanned for points that
        /// were not initially matched, but have a reference
        /// counterpart within a certain number of epsilon values
        /// (currently set at 3). These points are added to the
        /// itsMatchingPixList, and the new total number of matches is
        /// recorded.
        void addNewMatches();

        /// @brief Remove multiple references to objects in the match
        /// list.
        /// @details Objects that appear twice in the match list are
        /// examined, and the one with the largest flux value is
        /// kept. All others are removed from the list.
        void rejectMultipleMatches();

        /// @brief Output lists of matching and isolated points.
        void outputLists()
        {
            outputMatches();
            outputMisses();
        };
        /// @brief Output the list of matching points
        /// @details The list of matching points is written to the
        /// designated output file. The format is: type of match --
        /// source ID -- source X -- source Y -- source Flux -- ref ID
        /// -- ref X -- ref Y -- ref Flux. The "type of match" is
        /// either 1 for points matched with the Groth algorithm, or 2
        /// for those subsequently matched.
        void outputMatches();

        /// @brief Output the list of points that were not matched.
        /// @details The points in the source and reference lists that
        /// were not matched are written to the designated output
        /// file. The format is: type of point -- ID -- X -- Y --
        /// Flux. The "type of point" is either R for reference point
        /// or S for source point.
        void outputMisses();
        /// @brief Output the list of sources with any matches from the other list
        void outputSummary();
        /// @brief Output a single catalogue showing matches from the other list
        void outputSummary(PointCatalogue &cat, std::string filename);

        unsigned int srcListSize() {return this->itsSrcCatalogue.pointList().size();}
        unsigned int refListSize() {return this->itsRefCatalogue.pointList().size();}

    protected:

        /// @brief Hold lists for the source catalogue
        PointCatalogue itsSrcCatalogue;
        /// @brief Hold lists for the reference catalogue
        PointCatalogue itsRefCatalogue;
        /// @brief Image to get world-pixel conversion from
        std::string itsReferenceImage;
        /// @brief The list of matching triangles
        std::vector<std::pair<Triangle, Triangle> > itsMatchingTriList;
        /// @brief The list of matching points
        std::vector<std::pair<Point, Point> > itsMatchingPixList;
        /// @brief The epsilon error parameter for matching
        double itsEpsilon;
        casa::Unit itsEpsilonUnits;
        casa::Unit itsPositionUnits;
        /// @brief The number of matches after the initial attempt
        unsigned int itsNumInitialMatches;
        /// @brief Do the two catalogues have the same sense?
        bool itsSenseMatch;

        /// @brief Mean x- and y-offsets for the matches
        /// @{
        double itsMeanDx;
        double itsMeanDy;
        /// @}

        /// @brief The output files for the matches and misses
        /// @{
        std::string itsMatchFile;
        std::string itsMissFile;
        std::string itsSourceSummaryFile;
        std::string itsReferenceSummaryFile;
        /// @}

};


}
}
}
#endif
