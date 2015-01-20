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
#ifndef ASKAP_ANALYSIS_MATCHER_H_
#define ASKAP_ANALYSIS_MATCHER_H_

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <patternmatching/Triangle.h>
#include <patternmatching/Point.h>

#include <Common/ParameterSet.h>

#include <duchamp/fitsHeader.hh>

#include <vector>
#include <utility>
#include <string>
#include <math.h>

namespace askap {

namespace analysis {

namespace matching {

/// @brief Default value of the error parameter used in pattern matching
const double defaultEpsilon = 1.0;

/// @brief Maximum size for list of points
const unsigned int maxSizePointList = 25;

/// @brief Class to handle matching of patterns of sources
/// @details This class uses Triangle and Point classes to match
/// lists of points. It handles the file input and output, as well as the
///actual matching.
class Matcher {
    public:
        Matcher();
        Matcher(const Matcher &m);
        Matcher& operator= (const Matcher &m);
        virtual ~Matcher();
        /// @brief Constructor, using an input parameter set
        /// @details The parameter file is examined for the relevant
        /// parameters to define the input and output files, the base
        /// positions for both lists, and the epsilon value. The input
        /// files are read to obtain the source and reference point
        /// lists.
        Matcher(const LOFAR::ParameterSet& parset);

        /// @brief Store the FITS header, including the WCS of the image
        /// @details This function takes a duchamp FitsHeader
        /// representation of the FITS header information and
        /// stores it for doing the coordinate conversions
        void setHeader(duchamp::FitsHeader &head);

        /// @brief Read in the lists of source and reference objects.
        /// @details This function reads the source and reference
        /// pixel lists from the files provided. Checks are made
        /// for the validity of the files.
        void readLists();

        int srcListSize() {return itsSrcPixList.size();};
        int refListSize() {return itsRefPixList.size();};

        /// @brief Manually set the list of source points
        void setSrcList(std::vector<Point> srclist) {itsSrcPixList = srclist;};

        /// @brief Manually set the list of reference points
        void setRefList(std::vector<Point> reflist) {itsRefPixList = reflist;};

        /// @brief Fix the sizes of reference objects to reflect the beam size used.
        /// @details This function takes a reference list and
        /// convolves the sizes of the sources with a given beam. The
        /// relationships discussed in Wild (1970), AustJPhys 23, 113
        /// are used to combine a gaussian source with a gaussian beam.
        ///
        /// @param beam A vector containing the beam major axis, beam
        /// minor axis and beam position angle, all in degrees.
        ///
        /// @todo This treatment only deals with Gaussian
        /// sources. What if we have discs as well?
        void fixRefList(std::vector<float> beam);

        /// @brief Define the triangle lists from the (existing) point
        /// lists, and find matching triangles.
        /// @details The point lists are first shortened to the
        /// appropriate size by trimList(). The shortened lists are then
        /// converted into triangle lists, which are matched and
        /// trimmed.
        void setTriangleLists();

        /// @brief Find the points in each list that match.
        /// @details Matching points are found via the Groth voting
        /// function vote(). The number of matches and their sense are
        /// recorded.
        void findMatches();

        /// @brief Find the linear offsets between the two lists of
        /// points.
        /// @details The mean and rms offsets in the x- and
        /// y-directions are measured for the matching points.
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
            outputMatches(); outputMisses();
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

    protected:
        /// @brief The file with the list of points to be matched
        std::string itsSrcFile;
        /// @brief The file with the list of reference points
        std::string itsRefFile;
        /// @brief The fiducial right ascension for the lists
        std::string itsRA;
        /// @brief The fiducial declination for the lists
        std::string itsDec;
        /// @brief The type of position (dms or deg) for the source list
        std::string itsSrcPosType;
        /// @brief The type of position (dms or deg) for the reference list
        std::string itsRefPosType;
        /// @brief The radius within which to compare points. Negative
        /// value means use all points.
        double itsRadius;
        /// @brief The FITS header, including the World Coordinate
        /// System of the image, for converting RA/DEC positions to
        /// pixel locations
        duchamp::FitsHeader itsFITShead;
        /// @brief Which flux measure to use: peak or integrated
        std::string itsFluxMethod;
        /// @brief Whether to use the fitted flux values ("yes"), the
        ///  measured values ("no"), or the fitted values where
        ///  available, else the measured ("best");
        std::string itsFluxUseFit;

        /// @brief The list of source points (those to be matched)
        std::vector<Point> itsSrcPixList;
        /// @brief The list of reference points
        std::vector<Point> itsRefPixList;
        /// @brief The list of triangles from the source list
        std::vector<Triangle> itsSrcTriList;
        /// @brief The list of triangles from the reference list
        std::vector<Triangle> itsRefTriList;

        /// @brief The size of the lists used to generate triangles
        int itsTrimSize;

        /// @brief The list of matching triangles
        std::vector<std::pair<Triangle, Triangle> > itsMatchingTriList;
        /// @brief The list of matching points
        std::vector<std::pair<Point, Point> > itsMatchingPixList;

        /// @brief The epsilon error parameter for matching
        double itsEpsilon;
        /// @brief The mean offset in the x-direction between the lists
        double itsMeanDx;
        /// @brief The mean offset in the y-direction between the lists
        double itsMeanDy;
        /// @brief The rms offset in the x-direction between the lists
        double itsRmsDx;
        /// @brief The rms offset in the y-direction between the lists
        double itsRmsDy;

        /// @brief The number of matches just from the pattern matching
        int itsNumMatch1;
        /// @brief The number of matches after the subsequent matching step
        int itsNumMatch2;

        /// @brief The sense of the matches
        bool itsSenseMatch; // true for same sense, false for opposite sense

        /// @brief The file to which the matched points are written
        std::string itsOutputBestFile;
        /// @brief The file to which the non-matched points are written
        std::string itsOutputMissFile;

};

}

}

}

#endif
