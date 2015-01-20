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
#ifndef ASKAP_ANALYSIS_MATCHUTIL_H_
#define ASKAP_ANALYSIS_MATCHUTIL_H_

#include <patternmatching/Triangle.h>
#include <patternmatching/Point.h>
#include <patternmatching/Matcher.h>

#include <duchamp/fitsHeader.hh>

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

namespace askap {

namespace analysis {

namespace matching {

/// @brief Read in the list of points to be matched
/// @details Read in a list of points from a duchamp-Summary.txt file
/// (that is, a summary file produced by cduchamp). The base positions
/// are used to convert each point's position into an offset in
/// arcsec. The ID of each point is generated from the object number
/// in the list, plus the ra and dec, e.g. 2_12:34:56.78_-45:34:23.12
/// @param fin The file stream to read from
/// @param header The FitsHeader object defining the WCS
/// transformations
/// @param raBaseStr The base right ascension, in string form,
/// e.g. 12:23:34.5
/// @param decBaseStr The base right ascension, in string form,
/// e.g. -12:23:34.57
/// @param posType The type of position given in the stream: "dms"
/// (12:23:45) or "deg" (12.3958333)
/// @param radius The maximum radius from the base position within
/// which to keep objects. If negative, everything is kept.
/// @param fluxMethod Which flux value to use: either "peak" or
/// "integrated". Not used at the moment.
/// @param fluxUseFit Whether to use the fitted value of the flux. Can
/// be either "yes", "no", or "best". If "best", we use the fitted
/// flux whenever that value is >0 (taken to mean a fit was made
/// successfully).
/// @return A list of sources from the file
std::vector<matching::Point>
getSrcPixList(std::ifstream &fin, duchamp::FitsHeader &header,
              std::string raBaseStr, std::string decBaseStr,
              std::string posType, double radius,
              std::string fluxMethod = "peak", std::string fluxUseFit = "best");

/// @brief Read in the reference list
/// @details Reads in a list of points from a file, to serve as a
/// reference list. The file should have six columns: ra, dec, flux,
/// major axis, minor axis, position angle. The ra and dec should be
/// in string form: 12:23:34.43 etc.The base positions are used to
/// convert each point's position into an offset in arcsec. The ID of
/// each point is generated from the object number in the list, plus
/// the ra and dec, e.g. 2_12:34:56.78_-45:34:23.12
///
/// @param fin The file stream to read from
/// @param header The FitsHeader object containing information on the
/// WCS system (for conversion from WCS to pixel positions)
/// @param raBaseStr The base right ascension, in string form,
/// e.g. 12:23:34.5
/// @param decBaseStr The base right ascension, in string form,
/// e.g. -12:23:34.57
/// @param posType The type of position, either "dms" (12:34:56
/// format) or "deg" (decimal degrees)
/// @param radius The search radius within which points are
/// kept.
/// @return A list of sources from the file
std::vector<matching::Point>
getPixList(std::ifstream &fin, duchamp::FitsHeader &header,
           std::string raBaseStr, std::string decBaseStr,
           std::string posType, double radius);

/// @brief Read in the list of points to be matched
/// @details Read in a list of points from a duchamp-Summary.txt
/// file (that is, a summary file produced by cduchamp). The
/// base positions are used to convert each point's position
/// into an offset in arcsec. The ID of each point is generated
/// from the object number in the list, plus the ra and dec,
/// e.g. 2_12:34:56.78_-45:34:23.12
/// @param fin The file stream to read from
/// @param raBaseStr The base right ascension, in string form,
/// e.g. 12:23:34.5
/// @param decBaseStr The base right ascension, in string form,
/// e.g. -12:23:34.57
/// @param posType The type of position given in the stream: "dms"
/// (12:23:45) or "deg" (12.3958333)
/// @param radius The maximum radius from the base position within
/// which to keep objects. If negative, everything is kept.
/// @param fluxMethod Which flux value to use: either "peak" or
/// "integrated". Not used at the moment.
/// @param fluxUseFit Whether to use the fitted value of the flux. Can
/// be either "yes", "no", or "best". If "best", we use the fitted
/// flux whenever that value is >0 (taken to mean a fit was made
/// successfully).
/// @return A list of sources from the file
std::vector<matching::Point>
getSrcPixList(std::ifstream &fin, std::string raBaseStr, std::string decBaseStr,
              std::string posType, double radius,
              std::string fluxMethod = "peak", std::string fluxUseFit = "best");

/// @brief Read in the reference list
/// @details Reads in a list of points from a file, to serve as
/// a reference list. The file should have six columns: ra, dec,
/// flux, major axis, minor axis, position angle. The ra and dec
/// should be in string form: 12:23:34.43 etc.The base positions
/// are used to convert each point's position into an offset in
/// arcsec. The ID of each point is generated from the object
/// number in the list, plus the ra and dec,
/// e.g. 2_12:34:56.78_-45:34:23.12
/// @param fin The file stream to read from
/// @param raBaseStr The base right ascension, in string form,
/// e.g. 12:23:34.5
/// @param decBaseStr The base right ascension, in string form,
/// e.g. -12:23:34.57
/// @param posType The type of position, either "dms" (12:34:56
/// format) or "deg" (decimal degrees)
/// @param radius The search radius within which points are kept.
/// @return A list of sources from the file
std::vector<matching::Point>
getPixList(std::ifstream &fin, std::string raBaseStr, std::string decBaseStr,
           std::string posType, double radius);

/// @brief Shorten the list of points to a given length
/// @details The list of points is sorted by flux, and only the
/// maxSize highest-flux points are returned.
/// @param inputList List of points to be trimmed
/// @param maxSize Number of points to be returned. Defaults to matching::maxSizePointList
/// @return The maxSize highest-flux points, in flux order.
std::vector<matching::Point>
trimList(std::vector<matching::Point> &inputList, const unsigned int maxSize);

std::vector<matching::Point>
crudeMatchList(std::vector<matching::Point> &reflist,
               std::vector<matching::Point> &srclist,
               float maxOffset);

/// @brief Create a list of triangles from a list of points
std::vector<Triangle> getTriList(std::vector<Point> &pixlist);

/// @brief Match two lists of triangles
/// @details Finds a list of matching triangles from two
/// lists. The lists are both sorted in order of increasing
/// ratio, and the maximum ratio tolerance is found for each
/// list. Triangles from list1 are compared with a range from
/// list2, where the ratio of the comparison triangle falls
/// between the maximum acceptable range using the maximum
/// ratio tolerances (so that we don't look at every possible
/// triangle pair). The matching triangles are returned as a
/// vector of pairs of triangles.
/// @param list1 The first list of triangles
/// @param list2 The other list of triangles
/// @param epsilon The error parameter used to define the tolerances. Defaults to posTolerance.
/// @return A list of matching pairs of triangles.
std::vector<std::pair<Triangle, Triangle> >
matchLists(std::vector<Triangle> &list1, std::vector<Triangle> &list2, double epsilon);

/// @brief Eliminate likely false matches from a triangle list
/// @details A list of triangle matches is trimmed of false
/// matches. First, the magnifications (the difference in the
/// log(perimeter) values of the two matching triangles) are
/// examined: the true matches will have mags in a small range
/// of values, while false matches will have a broader
/// distribution. Only those matches in a narrow range of mags
/// will be accepted: those with mean_mag +- rms_mag*scale,
/// where scale is determined based on the number of same- and
/// opposite-sense matches.
///
/// If n_same and n_opp are the numbers of matches with the
/// same sense (both clockwise or both anticlockwise) or
/// opposite sense, then we get estimates of the number of
/// true & false matches by m_t=|n_same-n_opp| and m_f =
/// n_same + n_opp - m_t. Then scale is :
/// @li 1 if m_f > m_t
/// @li 3 if 0.1 m_t > m_f
/// @li 2 otherwise
///
/// Finally, all matches should have the same sense, so if
/// n_same > n_opp, all opposite sense matches are discarded,
/// and vice versa.
void trimTriList(std::vector<std::pair<Triangle, Triangle> > &trilist);

/// @brief Make the final assignment of matching points
/// @details The final step in removing false matches is the
/// voting. Each matched triangle votes for matched
/// points. The array of votes is ordered from max vote to min
/// vote. If no pair of points received more than one vote,
/// the lists don't match. Otherwise, successive points are
/// accepted until one of :
/// @li The vote drops by a factor of 2
/// @li We try to accept a point already accepted
/// @li The vote drops to zero.
std::vector<std::pair<Point, Point> >
vote(std::vector<std::pair<Triangle, Triangle> > &trilist);


}

}

}

#endif
