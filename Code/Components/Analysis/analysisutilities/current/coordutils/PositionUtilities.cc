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

#include <askap_analysisutilities.h>

#include <coordutils/PositionUtilities.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <utility>
#include <string>
#include <stdlib.h>
#include <math.h>

ASKAP_LOGGER(logger, ".positionutils");

namespace askap {

namespace analysisutilities {

const std::string removeLeadingBlanks(const std::string s)
{
    int i = 0;

    while (s[i] == ' ') {
        i++;
    }

    std::string newstring;

    for (unsigned int j = i; j < s.size(); j++) {
        newstring += s[j];
    }

    return newstring;
}

const double dmsToDec(const std::string input)
{
    std::string dms = removeLeadingBlanks(input);
    bool isNeg = false;

    if (dms[0] == '-') {
        isNeg = true;
    }

    for (unsigned int i = 0; i < dms.size(); i++) {
        if (dms[i] == ':') {
            dms[i] = ' ';
        }
    }

    std::stringstream ss;
    ss.str(dms);
    double d, m, s;
    ss >> d >> m >> s;
    double dec = fabs(d) + m / 60. + s / 3600.;

    if (isNeg) {
        dec = dec * -1.;
    }

    return dec;
}

const std::string decToDMS(const double input, const std::string type,
                           const int secondPrecision, const std::string separator)
{
    double normalisedInput = input;
    int degSize = 2; // number of figures in the degrees part of the output.
    std::string sign = "";

    bool convertToParset = false;
    if (separator == "parset") {
        convertToParset = true;
        separator = ":";
    }


    if ((type == "RA") || (type == "GLON")) {
        if (type == "GLON")  degSize = 3; // longitude has three figures in degrees.

        // Make these modulo 360.;
        while (normalisedInput < 0.) {
            normalisedInput += 360.;
        }

        while (normalisedInput > 360.) {
            normalisedInput -= 360.;
        }

        if (type == "RA") {
            normalisedInput /= 15.;  // Convert to hours.
        }
    } else if ((type == "DEC") || (type == "GLAT")) {
        if (normalisedInput < 0.) {
            sign = "-";
        } else {
            sign = "+";
        }
    } else { // UNKNOWN TYPE -- DEFAULT TO RA.
        ASKAPLOG_ERROR_STR(logger, "WARNING <decToDMS> : Unknown axis type (" <<
                           type << "). Defaulting to using RA.\n");

        while (normalisedInput < 0.) {
            normalisedInput += 360.;
        }

        while (normalisedInput > 360.) {
            normalisedInput -= 360.;
        }

        normalisedInput /= 15.;
    }

    normalisedInput = fabs(normalisedInput);

    int secondWidth = 2;

    if (secondPrecision > 0) {
        secondWidth += 1 + secondPrecision;
    }

    double dec_abs = normalisedInput;
    int hourOrDeg = int(dec_abs);
    int min = int(fmod(dec_abs, 1.) * 60.);
    const double onemin = 1. / 60.;
    double sec = fmod(dec_abs, onemin) * 3600.;

    if (fabs(sec - 60.) < 1.e-10) { // to prevent rounding errors stuffing things up
        sec = 0.;
        min++;
    } else if (sec > 60.) {
        sec -= 60.;
        min++;
    }

    if (min == 60) {
        min = 0;
        hourOrDeg++;
    }

    if (type == "RA") {
        hourOrDeg = hourOrDeg % 24;
    } else if (type == "GLON") {
        hourOrDeg = hourOrDeg % 360;
    } else if (type == "GLAT" || type == "DEC") {
        hourOrDeg = ((hourOrDeg + 90) % 180) - 90;
    } else {
        // UNKNOWN TYPE -- DEFAULT TO RA.
        ASKAPLOG_ERROR_STR(logger, "WARNING <decToDMS> : Unknown axis type (" <<
                           type << "). Defaulting to using RA.\n");
        hourOrDeg = hourOrDeg % 24;
    }

    std::stringstream output;
    output.setf(std::ios::fixed);
    output << sign
           << std::setw(degSize) << std::setfill('0') << std::setprecision(0)
           << fabs(hourOrDeg) << separator
           << std::setw(2) << std::setfill('0') << std::setprecision(0)
           << min  << separator ;
    output << std::setw(secondWidth) << std::setfill('0')
           << std::setprecision(secondPrecision) << sec;

    std::string outstring = output.str();
    if (convertToParset) {
        size_t pos;
        if (type == "DEC") {
            while (pos = outstring.find(":"), pos != std::string::npos) {
                outstring.replace(pos, 1, ".");
            }
        }
        if (type == "RA") {
            pos = outstring.find(":");
            if (pos != std::string::npos) outstring.replace(pos, 1, "h");
            pos = outstring.find(":");
            if (pos != std::string::npos) outstring.replace(pos, 1, "m");
        }
    }

    return outstring;
}

const double positionToDouble(const std::string position)
{
    size_t pos = position.find(':');
    if (pos == std::string::npos) {
        // Position doesn't have : in it => it is a position in decimal degrees.
        return atof(position.c_str());
    } else {
        // need to convert from dms to dec
        return dmsToDec(position);
    }
}

const double raToDouble(const std::string position)
{
    double dpos = positionToDouble(position);
    if (position.find(':') != std::string::npos) {
        // If the RA string is in H:M:S format, then assume it is in
        // hours, and we need to multiply the decimal value by 15.
        dpos *= 15.;
    }
    return dpos;
}

const double decToDouble(const std::string position)
{
    return positionToDouble(position);
}


const double angularSeparation(const std::string ra1, const std::string dec1,
                               const std::string ra2, const std::string dec2)
{
    if ((ra1 == ra2) && (dec1 == dec2)) {
        return 0.;
    } else {
        double sep = angularSeparation(dmsToDec(ra1) * 15., dmsToDec(dec1),
                                       dmsToDec(ra2) * 15., dmsToDec(dec2));
        return sep;
    }
}

const double angularSeparation(const double ra1, const double dec1,
                               const double ra2, const double dec2)
{
    double r1, d1, r2, d2;
    r1 = ra1  * M_PI / 180.;
    d1 = dec1 * M_PI / 180.;
    r2 = ra2  * M_PI / 180.;
    d2 = dec2 * M_PI / 180.;
    long double angsep = cos(r1 - r2) * cos(d1) * cos(d2) + sin(d1) * sin(d2);
    return acosl(angsep) * 180. / M_PI;
}


void equatorialToGalactic(const double ra, const double dec, double &gl, double &gb)
{
    const double NGP_RA = 192.859508 * M_PI / 180.;
    const double NGP_DEC = 27.128336 * M_PI / 180.;
    const double ASC_NODE = 32.932;
    double deltaRA = ra * M_PI / 180. - NGP_RA;
    double d = dec     * M_PI / 180.;
    double sinb = cos(d) * cos(NGP_DEC) * cos(deltaRA) + sin(d) * sin(NGP_DEC);
    gb = asin(sinb);
    // The l in sinl and cosl here is really gl-ASC_NODE
    double sinl = (sin(d) * cos(NGP_DEC) - cos(d) * cos(deltaRA) * sin(NGP_DEC)) / cos(gb);
    double cosl = cos(d) * sin(deltaRA) / cos(gb);
    gl = atan2(sinl, cosl);

    // Find the correct values of the lat & lon in degrees.
    gb = asin(sinb) * 180. / M_PI;
    gl = gl * 180. / M_PI + ASC_NODE;
}

}
}
