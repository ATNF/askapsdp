/// @file TosMetadata.h
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

#ifndef ASKAP_CP_TOSMETADATA_H
#define ASKAP_CP_TOSMETADATA_H

// System includes
#include <vector>

// ASKAPsoft includes
#include "casa/aips.h"

// Local package includes
#include "cpcommon/TosMetadataAntenna.h"

namespace askap {
namespace cp {

/// @brief This class encapsulates the dataset which comes from the
/// Telescope Operating System (TOS) for each correlator integration
/// cycle.
class TosMetadata {
    public:
        /// @brief Constructor
        ///
        /// This object is constructed with three dimensions. These are
        /// used to size the internal arrays, matrices and cubes.
        ///
        /// @param[in] nCoarseChannels  number of coarse channels.
        /// @param[in] nBeams   number of beams.
        /// @param[in] nPol     number of polarisations.
        TosMetadata(const casa::uInt& nCoarseChannels,
                    const casa::uInt& nBeams,
                    const casa::uInt& nPol);

        /////////////////////
        // Getters
        /////////////////////

        /// @brief Return the number of antennas.
        /// @return the number of antennas.
        casa::uInt nAntenna(void) const;

        /// @brief Return the number of coarse channels.
        /// @return the number of coarse channels.
        casa::uInt nCoarseChannels(void) const;

        /// @brief Return the number of beams.
        ///
        /// @note The number of beams applies to all antennas and to
        /// all coarse channels. The implication is sub-arraying is not
        /// really possible and a different number of beams per coarse
        /// channel is also not possible. This is a limitation which
        /// may need to be changed.
        ///
        /// @return the number of beams.
        casa::uInt nBeams(void) const;

        /// @brief Return the number of polarisations.
        /// @return the number of polarisations.
        casa::uInt nPol(void) const;

        /// @brief Return the integration cycle start time.
        ///
        /// @return the integration cycle start time. This is an
        ///     absolute time expressed as microseconds since MJD=0.
        casa::uLong time(void) const;

        /// @brief Return the integration cycle duration.
        ///
        /// @return the integration cycle duration. This is a
        ///     relative time expressed as microseconds.
        casa::uLong period(void) const;

        /////////////////////
        // Setters
        /////////////////////

        /// @brief Set the integration cycle start time.
        /// @param[in] time the integration cycle start time. This is
        ///     an absolute time expressed as microseconds since MJD=0.
        void time(const casa::uLong& time);

        /// @brief Set the integration cycle duration.
        /// @param[in] period the integration cycle duration. This is a
        ///     relative time expressed as microseconds.
        void period(const casa::uLong& period);


        /////////////////////////
        // Antenna access methods
        /////////////////////////

        /// @brief Add an antenna to the metadata.
        /// This method is used by the caller to build a complete
        /// TosMetadata object.
        ///
        /// @param[in] name the name of the antenna to add.
        ///
        /// @throw AskapError if an antenna with this name already
        ///     exists.
        /// @return the id of the antenna object created. The
        ///     implementation will guarantee the first id is zero
        ///     and additional id's will be incremented by one.
        casa::uInt addAntenna(const casa::String& name);

        /// @brief Return a const reference to the specified antenna.
        ///
        /// @param[in] id the identity of the antenna to be returned.
        ///
        /// @throw AskapError if the antenna ID is not valid.
        /// @return a const reference to the antenna specified by the
        ///     id parameter.
        const TosMetadataAntenna& antenna(const casa::uInt id) const;

        /// @brief Return a non-const reference to the specified
        /// antenna.
        ///
        /// @param[in] id the identity of the antenna to be returned.
        ///
        /// @throw AskapError if the antenna ID is not valid.
        /// @return a non-const reference to the antenna specified by
        ///     the id parameter.
        TosMetadataAntenna& antenna(const casa::uInt id);

    private:
        /// @brief Utility function to check the antenna ID passed
        /// is valid, otherwise an exception is thrown.
        ///
        /// @param[in] id the antenna ID to check.
        /// @throw AskapError if the antenna ID is not valid.
        void checkAntennaId(const casa::uInt& id) const;

        // Number of coarse channels
        casa::uInt itsNumCoarseChannels;

        // Number of beams
        casa::uInt itsNumBeams;

        // Number of polarisations
        casa::uInt itsNumPol;

        // Integration cycle start time.
        casa::uLong itsTime;

        // Integration cycle duration.
        casa::uLong itsPeriod;

        // Vector an TosMetadataAntenna objects
        std::vector<TosMetadataAntenna> itsAntenna;
};

}
}

#endif
