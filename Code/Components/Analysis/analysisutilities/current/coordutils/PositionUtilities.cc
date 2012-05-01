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

              std::string removeLeadingBlanks(std::string s)
        {
            /// @brief Remove blank spaces from the beginning of a string
            /// @details
            /// All blank spaces from the start of the string to the first
            /// non-blank-space character are deleted.
            ///
            int i = 0;

            while (s[i] == ' ') {
                i++;
            }

            std::string newstring;

            for (unsigned int j = i; j < s.size(); j++) newstring += s[j];

            return newstring;
        }

        double dmsToDec(std::string input)
        {
            /// @details
            ///  Assumes the angle given is in degrees, so if passing RA as
            ///  the argument, need to multiply by 15 to get the result in
            ///  degrees rather than hours.  The sign of the angle is
            ///  preserved, if present.
            ///
            std::string dms = removeLeadingBlanks(input);
            bool isNeg = false;

            if (dms[0] == '-') isNeg = true;

            for (unsigned int i = 0; i < dms.size(); i++) if (dms[i] == ':') dms[i] = ' ';

            std::stringstream ss;
            ss.str(dms);
            double d, m, s;
            ss >> d >> m >> s;
            double dec = fabs(d) + m / 60. + s / 3600.;

            if (isNeg) dec = dec * -1.;

            return dec;
        }

        std::string decToDMS(const double input, std::string type, int secondPrecision, std::string separator)
        {
            /// @details
            ///  This is the general form, where one can specify the degree of
            ///  precision of the seconds, and the separating character. The format reflects the axis type:
            ///  @li RA   (right ascension):     hh:mm:ss.ss, with dec modulo 360. (24hrs)
            ///  @li DEC  (declination):        sdd:mm:ss.ss  (with sign, either + or -)
            ///  @li GLON (galactic longitude): ddd:mm:ss.ss, with dec made modulo 360.
            ///  @li GLAT (galactic latitude):  sdd:mm:ss.ss  (with sign, either + or -)
            ///    Any other type defaults to RA, and prints warning.
            /// @param input Angle in decimal degrees.
            /// @param type Axis type to be used
            /// @param secondPrecision How many decimal places to quote the seconds to.
            /// @param separator The character (or string) to use as a
            /// separator between hh and mm, and mm and ss.sss.
            ///
            double normalisedInput = input;
            int degSize = 2; // number of figures in the degrees part of the output.
            std::string sign = "";

            if ((type == "RA") || (type == "GLON")) {
                if (type == "GLON")  degSize = 3; // longitude has three figures in degrees.

                // Make these modulo 360.;
                while (normalisedInput < 0.) { normalisedInput += 360.; }

                while (normalisedInput > 360.) { normalisedInput -= 360.; }

                if (type == "RA") normalisedInput /= 15.;  // Convert to hours.
            } else if ((type == "DEC") || (type == "GLAT")) {
                if (normalisedInput < 0.) sign = "-";
                else sign = "+";
            } else { // UNKNOWN TYPE -- DEFAULT TO RA.
                std::cerr << "WARNING <decToDMS> : Unknown axis type ("
                              << type << "). Defaulting to using RA.\n";

                while (normalisedInput < 0.) { normalisedInput += 360.; }

                while (normalisedInput > 360.) { normalisedInput -= 360.; }

                normalisedInput /= 15.;
            }

            normalisedInput = fabs(normalisedInput);

            int secondWidth = 2;

            if (secondPrecision > 0) secondWidth += 1 + secondPrecision;

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

            if (type == "RA") hourOrDeg = hourOrDeg % 24;
            else if (type == "GLON") hourOrDeg = hourOrDeg % 360;
            else if (type == "GLAT" || type == "DEC") hourOrDeg = ((hourOrDeg + 90) % 180) - 90;

            std::stringstream output;
            output.setf(std::ios::fixed);
            output << sign
                << std::setw(degSize) << std::setfill('0') << std::setprecision(0)
                << fabs(hourOrDeg) << separator
                << std::setw(2) << std::setfill('0') << std::setprecision(0)
                << min  << separator ;
            output << std::setw(secondWidth) << std::setfill('0')
                << std::setprecision(secondPrecision) << sec;
            return output.str();
        }


        double angularSeparation(const std::string ra1, const std::string dec1,
                                 const std::string ra2, const std::string dec2)
        {
            /// @details
            /// Calculates the angular separation between two sky positions,
            /// given as strings for RA and DEC. Uses the function
            /// angularSeparation(double,double,double,double).
            /// @param ra1 The right ascension for the first position.
            /// @param dec1 The declination for the first position.
            /// @param ra2 The right ascension for the second position.
            /// @param dec2 The declination for the second position.
            /// @return The angular separation in degrees.
            ///
            if ((ra1 == ra2) && (dec1 == dec2))
                return 0.;
            else {
                double sep = angularSeparation(
                                 dmsToDec(ra1) * 15.,
                                 dmsToDec(dec1),
                                 dmsToDec(ra2) * 15.,
                                 dmsToDec(dec2)
                             );
                return sep;
            }
        }

        double angularSeparation(double ra1, double dec1, double ra2, double dec2)
        {
            /// @details
            /// Calculates the angular separation between two sky positions,
            /// where RA and DEC are given in decimal degrees.
            /// @param ra1 The right ascension for the first position.
            /// @param dec1 The declination for the first position.
            /// @param ra2 The right ascension for the second position.
            /// @param dec2 The declination for the second position.
            /// @return The angular separation in degrees.
            ///
            double r1, d1, r2, d2;
            r1 = ra1  * M_PI / 180.;
            d1 = dec1 * M_PI / 180.;
            r2 = ra2  * M_PI / 180.;
            d2 = dec2 * M_PI / 180.;
            long double angsep = cos(r1 - r2) * cos(d1) * cos(d2) + sin(d1) * sin(d2);
            return acosl(angsep)*180. / M_PI;
        }


        void equatorialToGalactic(double ra, double dec, double &gl, double &gb)
        {
            /// @details
            /// Converts an equatorial (ra,dec) position to galactic
            /// coordinates. The equatorial position is assumed to be J2000.0.
            ///
            /// @param ra Right Ascension, J2000.0
            /// @param dec Declination, J2000.0
            /// @param gl Galactic Longitude. Returned value.
            /// @param gb Galactic Latitude. Returned value.
            ///
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
            gl = atan(fabs(sinl / cosl));

            // atan of the abs.value of the ratio returns a value between 0 and 90 degrees.
            // Need to correct the value of l according to the correct quandrant it is in.
            // This is worked out using the signs of sinl and cosl
            if (sinl > 0) {
                if (cosl > 0) gl = gl;
                else       gl = M_PI - gl;
            } else {
                if (cosl > 0) gl = 2.*M_PI - gl;
                else       gl = M_PI + gl;
            }

            // Find the correct values of the lat & lon in degrees.
            gb = asin(sinb) * 180. / M_PI;
            gl = gl * 180. / M_PI + ASC_NODE;
        }

    }
}
