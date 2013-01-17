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

#include <patternmatching/Point.h>
#include <modelcomponents/Spectrum.h>
#include <coordutils/PositionUtilities.h>

#include <iostream>
#include <math.h>
#include <map>
#include <vector>
#include <utility>
#include <string>

///@brief Where the log messages go.
ASKAP_LOGGER(logger, ".matching.Point");

namespace askap {

  namespace analysis {

    namespace matching {

      Point::Point():
	itsX(0.),itsY(0.),itsFlux(0.),itsID("")
      {
      }

      Point::Point(double x, double y):
	itsX(x), itsY(y),itsFlux(0.),itsID("")
      {
      }
      
      Point::Point(double x, double y, double f):
	itsX(x), itsY(y),itsFlux(f),itsID("")
      {
      }
      
      Point::Point(double x, double y, double f, std::string id):
	itsX(x), itsY(y),itsFlux(0.),itsID(id)
      {
      }

      Point::Point(analysisutilities::Spectrum *spec):
	itsFlux(spec->fluxZero()),itsID(spec->id())
      {
	std::string ra=spec->ra();
	size_t pos = ra.find(':');
	if(pos==std::string::npos) // RA doesn't have : in it => it is a position in decimal degrees.
	  this->itsX = atof(ra.c_str());
	else // need to convert from dms to dec
	  this->itsX = analysisutilities::dmsToDec(ra) * 15.;
	std::string dec=spec->dec();
	pos = dec.find(':');
	if(pos==std::string::npos) // DEC doesn't have : in it => it is a position in decimal degrees.
	  this->itsX = atof(dec.c_str());
	else // need to convert from dms to dec
	  this->itsX = analysisutilities::dmsToDec(dec);
      }

      Point::Point(const Point& p)
      {
	this->operator=(p);
      }

      Point& Point::operator=(const Point& p)
      {
	if (this == &p) return *this;

	this->itsX = p.itsX;
	this->itsY = p.itsY;
	this->itsFlux = p.itsFlux;
	this->itsID = p.itsID;
	return *this;
      }

    }
  }
}
