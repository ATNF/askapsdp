/// @file CubeMakerHelperFunctions.h
///
/// Utilitye functions to help with execution of makecube
///
/// @copyright (c) 2013 CSIRO
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
#ifndef ASKAP_CP_PIPELINETASKS_CUBEMAKERHELPERFUNCTIONS_H
#define ASKAP_CP_PIPELINETASKS_CUBEMAKERHELPERFUNCTIONS_H

// System includes
#include <vector>
#include <string>

// ASKAPsoft includes
#include <coordinates/Coordinates/CoordinateSystem.h>

namespace askap {
namespace cp {
namespace pipelinetasks {

class CubeMakerHelperFunctions {
    public:
        // @details Expands the string such as: "image.i.[0..15].spectral" into a
        // vector of strings from: "image.i.0.spectral" to "image.i.15.spectral"
        //
        // @param[in] pattern  a string containing the pattern of the image filesnames.
        //                     See the description for an example of this pattern.
        // @return A vector of filenames, that is, the expansion of the input pattern.
        // @throw AskapError   If the input pattern is inavlid.
        static std::vector<std::string> expandPattern(const std::string& pattern);

        /// @details Ensures the two coordinate systems are compatible, in that they
        /// have the same number of coordinates, pixel axes and world axes, the same
        /// type, and matching coordinate numbers for the SPECTRAL, STOKES and
        /// DIRECTION coordinates.
        static bool compatibleCoordinates(const casa::CoordinateSystem& c1,
                                          const casa::CoordinateSystem& c2);

        /// Ensures the coordinate system has a single spectral coordinate axis.
        static void assertValidCoordinates(const casa::CoordinateSystem& csys);

        /// @details Return the frequency value for channel zero of the spectral
        /// axis within the provided coordinate system.
        static double getChanFreq(const casa::CoordinateSystem& csys);

        /// Returns the increment between two coordinate systems. The channel-zero
        /// frequencies are extracted for each coodinate system, and the differnce
        /// is returned.
        static double getFreqIncrement(const casa::CoordinateSystem& c1,
                                       const casa::CoordinateSystem& c2);

        /// A new coordinate system is constructed. All coordinates from the first
        /// system are kept, with the exception of the spectral coordinate. This
        /// starts with that of the first, and has its frequency increment set to
        /// the difference between the zero-channel frequencies of the two systems.
        /// The reference pixel is set to zero and the reference value set to the
        /// zero-channel frequency of the first system.
        static casa::CoordinateSystem makeCoordinates(const casa::CoordinateSystem& c1,
                const casa::CoordinateSystem& c2,
                const casa::IPosition& refShape);
};
}
}
}

#endif
