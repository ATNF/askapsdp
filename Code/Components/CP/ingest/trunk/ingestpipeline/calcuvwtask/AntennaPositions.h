/// @file AntennaPositions.h
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
/// @author Ben Humphreys <ben.humphreys@csiro.au>

#ifndef ASKAP_CP_ANTENNAPOSITIONS_H
#define ASKAP_CP_ANTENNAPOSITIONS_H

// ASKAPsoft includes
#include "Common/ParameterSet.h"
#include "casa/Arrays/Matrix.h"
#include "casa/Arrays/Vector.h"
#include "measures/Measures/MPosition.h"

namespace askap {
namespace cp {

/// @brief A utility class to help convert antenna positions as described by
/// a parameter set to a casa::Matrix.
///
/// An example of an input parameter set is shown here:
/// @verbatim
/// location     =   [+117.471deg, -25.692deg, 192m, WGS84]
/// names        =   [A0, A1, A2, A3, A4, A5]
/// scale        =   1.0
/// A0           =  [-175.233429,  -1673.460938,  0.0000]
/// A1           =  [261.119019,   -796.922119,   0.0000]
/// A2           =  [-29.200520,   -744.432068,   0.0000]
/// A3           =  [-289.355286,  -586.936035,   0.0000]
/// A4           =  [-157.031570,  -815.570068,   0.0000]
/// A5           =  [-521.311646,  -754.674927,   0.0000]
/// @endverbatim
class AntennaPositions {
    public:
        /// @brief Constructor
        /// @param[in] parset a parameter set containing antenna locations
        AntennaPositions(const LOFAR::ParameterSet& parset);

        /// @brief Return antenna locations as absolute X, Y, Z coordinates.
        /// @return a matrix containing antenna coordinates. Size is 3 rows by
        /// nAntenna columns. Rows are x, y, z and columns are indexed by
        /// antenna id.
        casa::Matrix<double> getPositionMatrix(void);

    private:
        void local2global(casa::Vector<double>& xGeo, casa::Vector<double>& yGeo,
                          casa::Vector<double>& zGeo, const casa::MPosition& mRefLocation,
                          const casa::Vector<double>& xLocal, const casa::Vector<double>& yLocal,
                          const casa::Vector<double>& zLocal);

        void longlat2global(casa::Vector<double>& xReturned,
                            casa::Vector<double>& yReturned,
                            casa::Vector<double>& zReturned,
                            const casa::MPosition& mRefLocation,
                            const casa::Vector<double>& xIn,
                            const casa::Vector<double>& yIn,
                            const casa::Vector<double>& zIn);

        /// @brief Convert a string representation of a position to a
        /// casa::MPosition.
        /// Syntax for the position string is:
        /// @verbatim
        /// [latitude, longitude, altitude, type]
        /// @endverbatim
        ///
        /// For example:
        /// @verbatim
        /// [+117.471deg, -25.692deg, 192m, WGS84]
        /// @endverbatim
        /// Supported types are WGS84 and ITRF.
        casa::MPosition asMPosition(const std::vector<std::string>& position);

        // The antenna positions. Size is 3 rows by nAntenna columns.
        // Rows are x, y, z and columns are indexed by antenna id.
        casa::Matrix<double> itsAntXYZ;
};

}
}

#endif
