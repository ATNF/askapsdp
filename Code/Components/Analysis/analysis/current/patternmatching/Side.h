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
#ifndef ASKAP_ANALYSIS_SIDE_H_
#define ASKAP_ANALYSIS_SIDE_H_

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

            /// @brief A class to hold info on a triangle side (a straight line)
            /// @details This class holds the necessary information on a
            ///  line connecting two points, providing functions to access its length,
            ///  dx and dy.
            class Side {
                public:
                    /// @brief Default constructor
                    Side();
                    /// @brief Constructor using rise over run
                    Side(double run, double rise);
                    Side(Point &a, Point &b);
		    Side(const Side& s);
		    Side& operator=(const Side& s);
                   /// @brief Destructor
                    virtual ~Side() {};

                    /// @brief Definition function, using slope (defined by rise and run)
                    void define(double run, double rise);
                    /// @brief Definition function, using two points
                    void define(Point &a, Point &b);

                    /// @brief Return the rise (delta-y)
                    double rise() {return itsDY;};
                    /// @brief Return the run (delta-x)
                    double run() {return itsDX;};
                    /// @brief Return the length of the side
                    double length() {return hypot(itsDX, itsDY);};
                    /// @brief Less-than function, working on length of the sides.
                    friend bool operator<(Side lhs, Side rhs) {return lhs.length() < rhs.length();};

                protected:
                    /// @brief Length in x-direction
                    double itsDX;
                    /// @brief Length in y-direction
                    double itsDY;
            };


        }
    }
}

#endif

