/// @file
///
/// Utility functions relating to transformations of sky position
///
/// @copyright (c) 2010 CSIRO
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
/// @author Matthew Whiting <Matthew.Whiting@csiro.au>
///
#ifndef ASKAP_ANALYSISUTILS_POSUTILS_H_
#define ASKAP_ANALYSISUTILS_POSUTILS_H_

#include <iostream>
#include <string>
namespace askap {

namespace analysisutilities {

/// @brief Remove blank spaces from the beginning of a string.
/// All blank spaces from the start of the string to the first
/// non-blank-space character are deleted.
const std::string removeLeadingBlanks(const std::string s);

/// @brief Converts a string in the format +12:23:34.45 to a decimal
/// angle in degrees.
/// @details Assumes the angle given is in degrees, so if passing
/// RA as the argument, need to multiply by 15 to get the result
/// in degrees rather than hours.  The sign of the angle is
/// preserved, if present.
const double dmsToDec(const std::string input);

/// @brief Converts a decimal into a dd:mm:ss.ss format (or similar).
/// @details This is the general form, where one can specify the
///  degree of precision of the seconds, and the separating character.
///  The format reflects the axis type:
///  @li RA   (right ascension):     hh:mm:ss.ss, with dec modulo 360. (24hrs)
///  @li DEC  (declination):        sdd:mm:ss.ss  (with sign, either + or -)
///  @li GLON (galactic longitude): ddd:mm:ss.ss, with dec made modulo 360.
///  @li GLAT (galactic latitude):  sdd:mm:ss.ss  (with sign, either + or -)
///    Any other type defaults to RA, and prints warning.
/// @param input Angle in decimal degrees.
/// @param type Axis type to be used
/// @param secondPrecision How many decimal places to quote the
/// seconds to.
/// @param separator The character (or string) to use as a separator
/// between hh and mm, and mm and ss.sss. A special value of 'parset'
/// for separator will output RA in the format 19h39m25.03 and Dec as
/// -63.42.45.63
const std::string decToDMS(const double input, const std::string type = "DEC",
                           const int secondPrecision = 2, const std::string separator = ":");

/// Convert a string position to a decimal value. The string can
/// either be HMS/DMS formatted, or a decimal value (if HMS/DMS, it
/// needs to use a ':' as the separator).
const double positionToDouble(const std::string position);
/// Convert a RA string to decimal degrees. This will convert from hours to degrees if the string is in HMS format (HH:MM:SS.SSS)
const double raToDouble(const std::string position);
/// Convert a DEC string to decimal degrees. Simply a front-end to positionToDouble.
const double decToDouble(const std::string position);

/// @brief Find the angular separation of two sky positions
/// @details Calculates the angular separation between two sky
/// positions, given as strings for RA and DEC. Uses the function
/// angularSeparation(double,double,double,double).
/// @param ra1 The right ascension for the first position.
/// @param dec1 The declination for the first position.
/// @param ra2 The right ascension for the second position.
/// @param dec2 The declination for the second position.
/// @return The angular separation in degrees.
const double angularSeparation(const std::string ra1, const std::string dec1,
                               const std::string ra2, const std::string dec2);

/// @brief Find the angular separation of two sky positions.
/// @details Calculates the angular separation between two sky
/// positions, where RA and DEC are given in decimal degrees.
/// @param ra1 The right ascension for the first position.
/// @param dec1 The declination for the first position.
/// @param ra2 The right ascension for the second position.
/// @param dec2 The declination for the second position.
/// @return The angular separation in degrees.
const double angularSeparation(double ra1, double dec1, double ra2, double dec2);

/// @brief Convert equatorial coordinates to Galactic.
/// @details Converts an equatorial (ra,dec) position to galactic
/// coordinates. The equatorial position is assumed to be J2000.0.
/// @param ra Right Ascension, J2000.0
/// @param dec Declination, J2000.0
/// @param gl Galactic Longitude. Returned value.
/// @param gb Galactic Latitude. Returned value.
void equatorialToGalactic(const double ra, const double dec, double &gl, double &gb);


}

}

#endif
