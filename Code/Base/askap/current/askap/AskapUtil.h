/// @file AskapUtils.h
/// @brief Common ASKAP utility functions and classes
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
/// @author Ger van Diepen (gvd AT astron DOT nl)
///

#ifndef ASKAP_UTIL_H
#define ASKAP_UTIL_H

// System includes
#include <ostream>
#include <string>
#include <vector>
#include <list>
#include <sstream>

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "casa/aips.h"
#include "casa/Quanta.h"
#include "casa/Quanta/MVDirection.h"
#include "measures/Measures/MDirection.h"
#include "measures/Measures/MPosition.h"
#include "measures/Measures/MEpoch.h"

namespace askap {

/// Get the hostname of the machine (as per unistd.h gethostname)
/// @param full get the full name with domain or base name
/// @return a string representing the hostname
std::string getHostName(bool full = false);

/// Convert a string to uppercase.
std::string toUpper(std::string s);

/// Convert a string to lowercase.
std::string toLower(std::string s);

/// Round to nearest integer
/// @param x Value to be rounded
int nint(double x);

/// Round to nearest integer
/// @param x Value to be rounded
int nint(float x);

/// Write a vector to an ostream with a given separator, prefix and postfix.
/// \note operator<<() must be defined for the container elements.
template<typename Container>
void printContainer(std::ostream& os, const Container& ctr,
                    const char* separator = ",",
                    const char* prefix = "[", const char* postfix = "]")
{
    os << prefix;

    for (typename Container::const_iterator it = ctr.begin();
            it != ctr.end();
            ++it) {
        if (it != ctr.begin()) {
            os << separator;
        }

        os << *it;
    }

    os << postfix;
}

/// @brief a helper method to print directions nicely
/// @details By default an instance of casa::MVDirection is printed
/// as 3 direction cosines. It is not very convenient. This method
/// allows to print it in a more log-reader-friendly way.
/// This is the only method in this file (so far) which introduces
/// dependency on casacore. It can be moved to a separate sub-package
/// if necessary
/// @param[in] dir MVDirection object to print
/// @return a string containing a nice representation of the direction
std::string printDirection(const casa::MVDirection &dir);

/// @brief Interpret string as a quantity.
/// Interpret a string such as "2.5arcsec" as a casa::Quantity
///
/// @param[in] s    String to be interpreted
/// @param[in] unit ensure the constructed quantity conforms to
///                 units of this type.
/// @throw AskapError   if the string "s" cannot be interpreted as
///                     a quantity which conforms to the units specified
///                     by the "unit" parameters.
casa::Quantity asQuantity(const std::string& s,
        const std::string& unit = "");

/// @brief Interpret string as an MEpoch
/// @param[in] epoch String to be interpreted
casa::MEpoch asMEpoch(const std::vector<std::string>& epoch);

/// @brief Interpret string vector as an MDirection
/// The string vector shall have RA in the first element, declination in
/// the second and reference frame in the third. For example:
/// [12h30m00.00, -45.00.00.00, J2000]
/// @param[in] direction String to be interpreted
casa::MDirection asMDirection(const std::vector<std::string>& direction);

/// @brief Convert a string representation of a position to a
/// casa::MPosition. Syntax for the position string is:
/// @verbatim
/// [latitude, longitude, altitude, type]
/// @endverbatim
///
/// For example:
/// @verbatim
/// [+117.471deg, -25.692deg, 192m, WGS84]
/// @endverbatim
/// Supported types are WGS84 and ITRF.
///
/// @param position[in] String to be interpreted
/// @return an MPosition
casa::MPosition asMPosition(const std::vector<std::string>& position);

/// a number of helper functions are gathered in this namespace
namespace utility {

// comments from Max Voronkov:
// probably something like this exists somewhere in the Blob support.
// I grabbed this code from one of my old programs to speed up the development.
// It sat for a while inside GainCalibrationEquation, but now I need it in
// other unrelated classes, so the code has been moved here.
// If the appropriate code existing somewhere else in askap,
// we can switch to use that code instead.

/// @brief helper method to interpret string
/// @details any type supported by the input stream can be converted
/// using this method (e.g. string to numbers)
/// @param[in] str input string
/// @return result of the conversion
/// @exception AskapError is thrown if the conversion failed
template<class T> T fromString(const std::string &str) throw(AskapError)
{
    std::istringstream is(str);
    T buf;
    is >> buf;

    if (!is) {
        ASKAPTHROW(AskapError, "Unable to convert " << str);
    }

    return buf;
}

/// @brief helper method to convert any type (e.g. numbers) to a string
/// @details any type supported by the input stream can be converted
/// using this method.
/// @param[in] in a const reference to the value to convert
/// @return resulting string
/// @exception AskapError is thrown if the conversion failed
template<class T> std::string toString(const T &in) throw(AskapError)
{
    std::ostringstream os;
    os << in;

    if (!os) {
        ASKAPTHROW(AskapError, "Unable to convert " << in << " to string");
    }

    return os.str();
}

/// @brief helper class - null deleter
/// @details To prevent boost::shared_ptr from disposing of an object
/// passed by reference. This class used to be declared inside the data accessor, but
/// apparently there is a need for a wider visibility of this method. We can strip the
/// code into a separate file later, if it is really needed.
struct NullDeleter {
    /// @brief dummy method, does nothing
    void operator()(void const *) const {}
};


} // namespace utility
} // end namespace askap


namespace std {
/// @name operator<< Extensions
/// Print the contents of a vector or list.
///
/// Enclose it in square brackets, using a comma as separator.
/// \note operator<<() must be defined for type \c T.
//@{
template<typename T>
inline ostream& operator<<(ostream& os, const vector<T>& c)
{
    askap::printContainer(os, c, ",", "[", "]");
    return os;
}
template<typename T>
inline ostream& operator<<(ostream& os, const list<T>& c)
{
    askap::printContainer(os, c, ",", "[", "]");
    return os;
}
//@}
} // end namespace std

#endif
