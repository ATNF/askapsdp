/// @file AskapUtil.cc
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

// Include own header file first
#include "askap/AskapUtil.h"

// System includes
#include <algorithm>
#include <string>
#include <sstream>
#include <iomanip>
#include <unistd.h>

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "casa/aips.h"
#include "casa/Quanta.h"
#include "casa/Quanta/MVDirection.h"
#include "casa/Quanta/MVAngle.h"
#include "casa/Quanta/MVEpoch.h"
#include "measures/Measures/MDirection.h"
#include "measures/Measures/MPosition.h"
#include "measures/Measures/MEpoch.h"
#include "measures/Measures/MCEpoch.h"
#include "measures/Measures/MeasConvert.h"

namespace askap {

std::string printDirection(const casa::MVDirection &dir)
{
    std::ostringstream os;
    os << std::setprecision(8) << casa::MVAngle::Format(casa::MVAngle::TIME)
        << casa::MVAngle(dir.getLong("deg"))
        << " " << std::setprecision(8) << casa::MVAngle::Format(casa::MVAngle::ANGLE) <<
    casa::MVAngle(dir.getLat("deg"));
    return os.str();
}

std::string printLon(const casa::MDirection& dir)
{
    std::ostringstream os;
    os << std::setprecision(8) << casa::MVAngle::Format(casa::MVAngle::TIME)
        << casa::MVAngle(dir.getValue().getLong("deg"));
    return os.str();
}

std::string printLat(const casa::MDirection& dir)
{
    std::ostringstream os;
    os << std::setprecision(8) << casa::MVAngle::Format(casa::MVAngle::ANGLE)
        << casa::MVAngle(dir.getValue().getLat("deg"));
    return os.str();
}

std::string getHostName(bool full)
{
    char hname[256];

    if (gethostname(hname, 256) != 0) {
        return std::string("localhost");
    }

    std::string hostname(hname);

    if (!full) {
        std::string::size_type dotloc = hostname.find_first_of(".");

        if (dotloc != hostname.npos) {
            return hostname.substr(0, dotloc);
        }
    }

    return hostname;
}

std::string toUpper(std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(), toupper);
    return str;
}

std::string toLower(std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(), tolower);
    return str;
}

int nint(double x)
{
    return x > 0 ? int(x + 0.5) : int(x - 0.5);
}

int nint(float x)
{
    return x > 0 ? int(x + 0.5f) : int(x - 0.5f);
}

casa::Quantity asQuantity(const string& s, const std::string& unit)
{
    casa::Quantity q;
    casa::Quantity::read(q, s);

    if ((!unit.empty()) && (!q.isConform(casa::Unit(unit)))) {
        ASKAPTHROW(AskapError, "Quantity: " << s << " does not conform to unit " << unit);
    }

    return q;
}

casa::MEpoch asMEpoch(const vector<string>& epoch)
{
    ASKAPCHECK(epoch.size() == 2, "Not a valid epoch");

    casa::Quantity datetime;
    casa::Quantity::read(datetime, epoch[0]);
    casa::MEpoch::Types type;
    casa::MEpoch::getType(type, epoch[1]);
    casa::MEpoch ep(datetime, type);
    return ep;
}

/// @brief Converts a colon separated latitude coordinate,
/// a format not supported by casacore, to a format that is
/// supported.
/// For example:
/// 45:00:0.00 -> 45h00m0.00
static std::string convertLatitude(const std::string& s)
{
    std::string out(s);
    size_t i = out.find_first_of(":");
    if (i != string::npos) {
        out[i] = 'd';
        i = out.find_first_of(":");
        if (i != string::npos) {
            out[i] = 'm';
        }
    }
    return out;
}

casa::MDirection asMDirection(const vector<string>& direction)
{
    ASKAPCHECK(direction.size() == 3, "Not a valid direction");

    casa::Quantity lng;
    casa::Quantity::read(lng, direction[0]);

    casa::Quantity lat;
    casa::Quantity::read(lat, convertLatitude(direction[1]));

    casa::MDirection::Types type;
    casa::MDirection::getType(type, direction[2]);
    casa::MDirection dir(lng, lat, type);
    return dir;
}

casa::MPosition asMPosition(const vector<string>& position)
{
    ASKAPCHECK(position.size() == 4, "Not a valid position");

    casa::Quantity lng;
    casa::Quantity::read(lng, position[0]);
    casa::Quantity lat;
    casa::Quantity::read(lat, position[1]);
    casa::Quantity height;
    casa::Quantity::read(height, position[2]);
    casa::MPosition::Types type;
    casa::MPosition::getType(type, position[3]);
    casa::MVPosition mvPos(height, lng, lat);
    casa::MPosition pos(mvPos, type);
    return pos;
}

  
  
/// @brief convert BAT to UTC Epoch via casa
/// @param[in] bat BAT as 64-bit integer
/// @return casa epoch measure in the UTC frame
casa::MEpoch bat2epoch(const uint64_t &bat)
{
  const casa::MVEpoch timeTAI(static_cast<casa::Double>(bat / microsecondsPerDay), 
                              static_cast<casa::Double>(bat % microsecondsPerDay) /
                              static_cast<casa::Double>(microsecondsPerDay));
  const casa::MEpoch epoch = casa::MEpoch::Convert(casa::MEpoch(timeTAI, casa::MEpoch::Ref(casa::MEpoch::TAI)),
                             casa::MEpoch::Ref(casa::MEpoch::UTC))();
  return epoch;
}

/// @brief convert casa Epoch to BAT
/// @param[in] epoch casa epoch measure, typically in UTC frame, but can be anything supported by casa
/// @return BAT as 64-bit integer
uint64_t epoch2bat(const casa::MEpoch &epoch)
{
   const casa::MVEpoch epochTAI= casa::MEpoch::Convert(epoch, 
                  casa::MEpoch::Ref(casa::MEpoch::TAI))().getValue();
   const uint64_t startOfDayBAT = static_cast<uint64_t>(epochTAI.getDay()*microsecondsPerDay);
   return startOfDayBAT + static_cast<uint64_t>(epochTAI.getDayFraction()*microsecondsPerDay);
}


} // end namespace askap
