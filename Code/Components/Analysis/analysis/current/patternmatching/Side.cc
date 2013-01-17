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
#include <askap_analysis.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <patternmatching/Side.h>
#include <patternmatching/Point.h>

#include <iostream>
#include <math.h>
#include <map>
#include <vector>
#include <utility>
#include <string>

///@brief Where the log messages go.
ASKAP_LOGGER(logger, ".matching");

namespace askap {

  namespace analysis {

    namespace matching {

      Side::Side():
	itsDX(0.),itsDY(0.)
      {
      }

      Side::Side(double run, double rise):
	itsDX(run), itsDY(rise)
      {
      }
      
      Side::Side(const Side& s)
      {
	this->operator=(s);
      }

      Side& Side::operator=(const Side& s)
      {
	if (this == &s) return *this;

	this->itsDX = s.itsDX;
	this->itsDY = s.itsDY;
	return *this;
      }
      
      void Side::define(double run, double rise)
      {
	this->itsDX = run; 
	this->itsDY = rise;
      }

      void Side::define(Point a, Point b)
      {
	this->itsDX = a.x() - b.x(); 
	this->itsDY = a.y() - b.y();
      }


    }
  }
}
