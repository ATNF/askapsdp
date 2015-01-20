/// @file
///
/// Provides generic methods for pattern matching
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
#ifndef ASKAP_ANALYSIS_GROTHTRIANGLES_H_
#define ASKAP_ANALYSIS_GROTHTRIANGLES_H_

#include <patternmatching/Point.h>

#include <iostream>
#include <iomanip>
#include <vector>
#include <utility>
#include <string>
#include <math.h>

namespace askap {

namespace analysis {

namespace matching {


/// @brief The default tolerance in the position for triangle matching
const double posTolerance = 0.001;
/// @brief The default elimination threshold for culling lists prior to triangle matching
const double elimThreshold = 0.003;


/// @brief Support class for matching patterns of sources
///
/// @details This class holds all the information to specify a
/// triangle of points from a list of sources that can be matched to
/// another list. The formulation follows Groth 1986 (AJ 91, 1244-1248).

class Triangle {
    public:
        /// @brief Default constructor
        Triangle();
        /// @brief Constructor from three Points
        Triangle(Point &pt1, Point &pt2, Point &pt3);
        /// @brief Constructor from three positions
        Triangle(double x1, double y1, double x2, double y2, double x3, double y3);
        /// @brief Copy constructor
        Triangle(const Triangle& t);
        /// @brief Copy function
        Triangle& operator= (const Triangle& t);
        /// @brief Destructor
        ~Triangle() {};
        /// @brief Definition function using three Points
        /// @details Define a triangle from three points. The key part
        ///  of this function is to order the sides by their
        ///  length. The triangle is defined on the basis of the ratio
        ///  of the longest to smallest sides, and the angle between
        ///  them. The given points are used to define sides, which
        ///  are then ordered according to their length. The triangle
        ///  parameters are then calculated from the known side
        ///  parameters.
        void define(Point &pt1, Point &pt2, Point &pt3);

        /// @brief Calculate tolerances for triangle parameters.
        /// @details The tolerances for the triangle parameters are
        /// calculated. These require the angle and ratio parameters
        /// to have been calculated, so this should be done after the
        /// triangle is defined.
        /// @param epsilon The parameter governing the size of the
        /// acceptable error in matching. This defaults to the value
        /// of posTolerance
        void defineTolerances(double epsilon = posTolerance);

        /// @brief Does this triangle match another?
        /// @details Does the triangle match another. Compares the
        /// ratios and angles to see whether they match to within the
        /// respective tolerances. Triangle::defineTolerances is
        /// called prior to testing, using the value of epsilon.
        /// @param comp The comparison triangle
        /// @param epsilon The error parameter used to define the
        /// tolerances. Defaults to posTolerance.
        /// @return True if triangles match
        bool isMatch(Triangle &comp, double epsilon = posTolerance);

        /// @brief Return the ratio of longest to shortest sides
        double ratio() {return itsRatio;};
        /// @brief Return the tolerance for the ratio value
        double ratioTol() {return itsRatioTolerance;};
        /// @brief Return the angle
        double angle() {return itsAngle;};
        /// @brief Return the tolerance for the angle value
        double angleTol() {return itsAngleTolerance;};
        /// @brief Is the sense of the triangle clockwise?
        double isClockwise() {return itIsClockwise;};
        /// @brief Return the log of the perimeter
        double perimeter() {return itsLogPerimeter;};

        /// @brief Return the first point
        Point one() {return itsPts[0];};
        /// @brief Return the second point
        Point two() {return itsPts[1];};
        /// @brief Return the third point
        Point three() {return itsPts[2];};
        /// @brief Return the list of Points
        std::vector<Point> getPtList() {return itsPts;};

        /// @brief Less-than function, working on ratio values.
        friend bool operator< (Triangle lhs, Triangle rhs) {return lhs.ratio() < rhs.ratio();};

        friend std::ostream &operator<< (std::ostream &stream, Triangle &tri)
        {
            stream << tri.itsPts[0] << "|" << tri.itsPts[1] << "|" << tri.itsPts[2] <<
                   "||" << tri.itsRatio << "/" << tri.itsRatioTolerance <<
                   "|" << tri.itsAngle << "/" << tri.itsAngleTolerance;
            return stream;
        };

    protected:

        void initialise();

        /// @brief The log of the perimeter of the triangle
        double itsLogPerimeter;
        /// @brief Whether the sides increase in size in a clockwise fashion
        bool   itIsClockwise;
        /// @brief The ratio between the largest and smallest sides
        double itsRatio;
        /// @brief The tolerance in the ratio value
        double itsRatioTolerance;
        /// @brief The angle between the largest and smallest sides (actually cos(angle))
        double itsAngle;
        /// @brief The tolerance in the angle value
        double itsAngleTolerance;

        /// @brief The list of points making up the triangle
        std::vector<Point> itsPts;

};

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

