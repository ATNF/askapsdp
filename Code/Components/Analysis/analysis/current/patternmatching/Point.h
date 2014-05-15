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
#ifndef ASKAP_ANALYSIS_POINT_H_
#define ASKAP_ANALYSIS_POINT_H_

#include <iostream>
#include <iomanip>
#include <vector>
#include <utility>
#include <string>
#include <math.h>

#include <modelcomponents/Spectrum.h>

namespace askap {

  namespace analysis {

    namespace matching {

      /// @brief A class to hold information about a 2D point
      /// @details This class holds positional information that will
      ///  be used by the pattern matching algorithms. It holds information about
      ///  a single point in the plane: its X and Y coordinates, its flux (a
      ///  measure of its brightness or importance), and an identification
      ///  string.
      class Point {
      public:
	/// @brief Default constructor
	Point();
	/// @brief Constructor from position
	Point(double x, double y);
	/// @brief Constructor from position & flux
	Point(double x, double y, double f);
	/// @brief Constructor from position, flux, ID
	Point(double x, double y, double f, std::string id);
	/// @brief Constructor from a pointer to a Spectrum
	Point(analysisutilities::Spectrum *spec);
	/// @brief Copy Constructor
	Point(const Point& p);
	/// @brief Copy function
	Point& operator= (const Point& p);
	/// @brief Destructor
	virtual ~Point() {};

	/// @brief Set the x coordinate
	void setX(double x) {itsX = x;};
	/// @brief Return the x coordinate
	double x() {return itsX;};
	/// @brief Set the y coordinate
	void setY(double y) {itsY = y;};
	/// @brief Return the y coordinate
	double y() {return itsY;};
	/// @brief Set the flux
	void setFlux(double f) {itsFlux = f;};
	/// @brief Return the flux
	double flux() {return itsFlux;};
	/// @brief Set the ID string
	void setID(std::string id) {itsID = id;};
	/// @brief Return the ID string
	std::string ID() {return itsID;};

	/// @brief Less-than function, operating on the flux
	friend bool operator<(Point lhs, Point rhs) {return lhs.flux() < rhs.flux();};

	/// @brief Return the separation from another Point.
	double sep(Point pt) {return hypot(itsX - pt.x(), itsY - pt.y());};

      protected:
	///@brief The X coordinate
	double itsX;
	///@brief The Y coordinate
	double itsY;
	///@brief The peak flux of the point
	double itsFlux;
	///@brief The identification string
	std::string itsID;
      };

    }
  }
}

#endif

