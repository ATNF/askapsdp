/// @file
///
/// Defines the basic features of a component of a RadioSource
///  object. These will be used in the profile fitting.
///
/// @copyright (c) 2008 CSIRO
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

#include <sourcefitting/Component.h>
#include <iostream>
#include <iomanip>

namespace askap {

    namespace analysis {

        namespace sourcefitting {

            SubComponent::SubComponent(const SubComponent& c)
            {
                operator=(c);
            }

            SubComponent& SubComponent::operator= (const SubComponent& c)
            {
                if (this == &c) return *this;

                itsXpos = c.itsXpos;
                itsYpos = c.itsYpos;
                itsPeakFlux = c.itsPeakFlux;
                itsMajorAxis = c.itsMajorAxis;
                itsMinorAxis = c.itsMinorAxis;
                itsPositionAngle = c.itsPositionAngle;
                return *this;
            }

            bool operator< (SubComponent lhs, SubComponent rhs)
            {
                /// @details Comparison of SubComponents is done on the basis of the peak flux only, with a strict less-than comparison.
                return lhs.itsPeakFlux < rhs.itsPeakFlux;
            }

            std::ostream& operator<< (std::ostream& theStream, SubComponent& c)
            {
                /// @details Output the key parameter values to the stream. The flux has a precision of 8 and the rest a precision of 3.
                theStream.setf(std::ios::fixed);
                theStream << std::setprecision(8);
                theStream << c.itsPeakFlux << " ";
                theStream << std::setprecision(3);
                theStream << c.itsXpos << " " << c.itsYpos << " "
                    << c.itsMajorAxis << " " << c.itsMinorAxis << " " << c.itsPositionAngle;
                return theStream;
            }

        }

    }

}
