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

#ifndef ASKAP_ANALYSIS_COMPONENT_H_
#define ASKAP_ANALYSIS_COMPONENT_H_
#include <iostream>
#include <iomanip>

namespace askap {

    namespace analysis {

        namespace sourcefitting {

            /// @ingroup sourcefitting
            /// @brief Class to store basic details of a Gaussian component
            ///
            /// @details This class stores the basic information needed to
            /// describe a 2D Gaussian component: location, peak flux, shape and
            /// orientation. The aim is to provide a simple class that can be ordered
            /// by the peak flux.
            class SubComponent {
                public:
                    /// @brief Default constructor
                    SubComponent() {};
                    /// @brief Default destructor
                    virtual ~SubComponent() {};
                    /// @brief Copy constructor
                    SubComponent(const SubComponent& c);
                    /// @brief Copy function
                    SubComponent& operator= (const SubComponent& c);

                    /// @brief Returns the x-pixel centre location
                    double x() {return itsXpos;};
                    /// @brief Returns the y-pixel centre location
                    double y() {return itsYpos;};
                    /// @brief Returns the peak flux
                    double peak() {return itsPeakFlux;};
                    /// @brief Returns the major axis length
                    double maj() {return itsMajorAxis;};
                    /// @brief Returns the minor axis length
                    double min() {return itsMinorAxis;};
                    /// @brief Returns the position angle
                    double pa() {return itsPositionAngle;};

                    /// @brief Set the x-pixel centre location
                    void setX(double d) {itsXpos = d;};
                    /// @brief Set the y-pixel centre location
                    void setY(double d) {itsYpos = d;};
                    /// @brief Set the peak flux
                    void setPeak(double d) {itsPeakFlux = d;};
                    /// @brief Set the major axis length
                    void setMajor(double d) {itsMajorAxis = d;};
                    /// @brief Set the minor axis length
                    void setMinor(double d) {itsMinorAxis = d;};
                    /// @brief Set the position angle
                    void setPA(double d) {itsPositionAngle = d;};

                    /// @brief Less-than function, that compares peak fluxes
                    friend bool operator< (SubComponent lhs, SubComponent rhs);

                    /// @brief Stream output function.
                    friend std::ostream& operator<< (std::ostream& theStream, SubComponent& c);

                protected:
                    /// @brief The x-pixel location of the component centre.
                    double itsXpos;
                    /// @brief The y-pixel location of the component centre.
                    double itsYpos;
                    /// @brief The peak flux of the component
                    double itsPeakFlux;
                    /// @brief The length of the major axis of the component
                    double itsMajorAxis;
                    /// @brief The length of the minor axis of the component
                    double itsMinorAxis;
                    /// @brief The position angle of the component
                    double itsPositionAngle;
            };

        }

    }

}
#endif
