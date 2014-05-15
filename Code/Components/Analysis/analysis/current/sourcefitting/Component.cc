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

#include <askap_analysis.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <sourcefitting/Component.h>
#include <iostream>
#include <iomanip>
#include <scimath/Functionals/Gaussian2D.h>

///@brief Where the log messages go.
ASKAP_LOGGER(logger, ".component");

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

	  casa::Gaussian2D<casa::Double> SubComponent::asGauss()
	  {
	    float axialRatio,axis;
	    if(this->itsMajorAxis>0) axialRatio = this->itsMinorAxis / this->itsMajorAxis;
	    else axialRatio = 1.;
	    axis=this->itsMajorAxis;
	    if(axialRatio > 1){
	      axialRatio = 1./axialRatio;
	      axis=this->itsMinorAxis;
	    }
	    axis = std::max(1.e-10f,axis);
	    // ASKAPLOG_DEBUG_STR(logger, "component " << *this << " has axial ratio " << axialRatio);
	    casa::Gaussian2D<casa::Double> gauss(this->itsPeakFlux,this->itsXpos,this->itsYpos, axis, axialRatio, this->itsPositionAngle);
	    return gauss;

	  }

            std::ostream& operator<< (std::ostream& theStream, SubComponent& c)
            {
                /// @details Output the key parameter values to the stream. The flux has a precision of 8 and the rest a precision of 3.
                theStream.setf(std::ios::fixed);
                theStream << std::setprecision(8);
                theStream << c.itsPeakFlux << " ";
                theStream << std::setprecision(6);
                theStream << c.itsXpos << " " << c.itsYpos << " "
                    << c.itsMajorAxis << " " << c.itsMinorAxis << " " << c.itsPositionAngle;
                return theStream;
            }

        }

    }

}
