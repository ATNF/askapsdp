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
#ifndef ASKAP_EVALUATION_EVALUTIL_H_
#define ASKAP_EVALUATION_EVALUTIL_H_

#include <patternmatching/GrothTriangles.h>
#include <patternmatching/Matcher.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

namespace askap {

    namespace evaluation {

        /// @brief Read in the list of points to be matched
        std::vector<matching::Point> getSrcPixList(std::ifstream &fin, std::string raBaseStr, std::string decBaseStr, double radius, std::string fluxMethod = "peak", std::string fluxUseFit = "best");

        /// @brief Read in the reference list
        std::vector<matching::Point> getPixList(std::ifstream &fin, std::string raBaseStr, std::string decBaseStr, double radius);

        /// @brief Shorten the list of points to a given length
        std::vector<matching::Point> trimList(std::vector<matching::Point> &inputList,
                                              const unsigned int maxSize = matching::maxSizePointList);



    }

}

#endif
