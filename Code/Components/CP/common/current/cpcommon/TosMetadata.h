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
        TosMetadata();

        /////////////////////
        // Getters
        /////////////////////

        /// @brief Return the number of antennas.
        /// @return the number of antennas.
        casa::uInt nAntenna(void) const;

        /// @brief Return the integration cycle start time.
        ///
        /// @return the integration cycle start time. This is an
        ///     absolute time expressed as microseconds since MJD=0.
        casa::uLong time(void) const;

        /// @brief Get the Scan ID. Valid values are:
        casa::Int scanId(void) const;

        /// @brief Get the FLAG which indicates the entire integration should
        /// be flagged.
        casa::Bool flagged(void) const;

        /////////////////////
        // Setters
        /////////////////////

        /// @brief Set the integration cycle start time.
        /// @param[in] time the integration cycle start time. This is
        ///     an absolute time expressed as microseconds since MJD=0.
        void time(const casa::uLong time);

        /// @brief Set the Scan ID. Valid values are:
        /// * -1 - Which indicates no observation is executing
        /// * > 0 - The scan ID.
        void scanId(const casa::Int id);

        /// @brief Set the FLAG which indicates the entire integration should
        /// be flagged.
        void flagged(const casa::Bool flag);

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

        /// Returns the numbers of antennas
        size_t nAntennas() const;

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

        // Integration cycle start time.
        casa::uLong itsTime;

        // Scan ID
        casa::Int itsScanId;

        // Indicates this integration (as indicated by the timestamp) should be flagged
        // in its entirety
        casa::Bool itsFlagged;

        // Vector an TosMetadataAntenna objects
        std::vector<TosMetadataAntenna> itsAntenna;
};

}
}

#endif
