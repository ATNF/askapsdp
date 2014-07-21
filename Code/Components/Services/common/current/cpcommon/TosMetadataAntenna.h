/// @file TosMetadataAntenna.h
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

#ifndef ASKAP_CP_TOSMETADATAANTENNA_H
#define ASKAP_CP_TOSMETADATAANTENNA_H

// ASKAPsoft includes
#include "casa/aips.h"
#include "measures/Measures/MDirection.h"
#include "casa/Quanta/Quantum.h"

namespace askap {
namespace cp {

/// @brief This class encapsulates the per-antenna part of the dataset which
/// comes from the Telescope Operating System (TOS) for each correlator
/// integration cycle.
///
/// This class is used by the TosMetdata class, with one instance of this class
/// existing for each physical antenna.
class TosMetadataAntenna {

    public:
        /// @brief Constructor
        ///
        /// This object is constructed with three dimensions. These are
        /// used to size the internal arrays, matrices and cubes.
        ///
        /// @param[in] name the name of the antenna.
        TosMetadataAntenna(const casa::String& name);

        /// @brief Get the name of the antenna.
        /// @return the name of the antenna.
        casa::String name(void) const;

        /// @brief Get the actual coordinates for the dish pointing
        /// @return the actual coordinates for the dish pointing
        casa::MDirection actualRaDec(void) const;

        /// @brief Set the actual coordinates for the dish pointing
        /// @param[in] val the actual coordinates for the dish pointing
        void actualRaDec(const casa::MDirection& val);

        /// @brief Get the actual coordinates for the dish pointing
        /// @return the actual coordinates for the dish pointing
        casa::MDirection actualAzEl(void) const;

        /// @brief Set the actual coordinates for the dish pointing
        /// @param[in] val the actual coordinates for the dish pointing
        void actualAzEl(const casa::MDirection& val);

        /// @brief Get the polarisation axis angle.
        /// @return the polarisation axis angle
        casa::Quantity actualPolAngle(void) const;

        /// @brief Set the polarisation axis angle.
        /// @param[in] q    the polarisation axis angle
        void actualPolAngle(const casa::Quantity& q);

        /// @brief Get the value of the onSource flag.
        ///
        /// @return True if antenna was within tolerance thresholds of the
        /// target trajectory throughout the entire integration cycle. If
        /// this is false then all data from this antenna should be flagged.
        casa::Bool onSource(void) const;

        /// @brief Set the value of the onSource flag.
        ///
        /// @param[in] val the value of the on source flag. True if antenna
        /// was within tolerance thresholds of the target trajectory throughout
        /// the entire integration cycle.
        void onSource(const casa::Bool& val);

        /// @brief Get the value of the general (misc error) flag.
        ///
        /// @return true if hardware monitoring reveals a problem
        /// (eg. LO out of lock) that means all data from this antenna
        /// should be flagged.
        casa::Bool flagged(void) const;

        /// @brief Set the value of the general (misc error) flag.
        /// @param[in] val teh value of the hardware error flag. Use true to
        /// indicate a hardware error, otherwise false.
        void flagged(const casa::Bool& val);

    private:

        /// The name of the antenna
        casa::String itsName;

        /// The actual RA/DEC 
        casa::MDirection itsActualRaDec;

        /// The actual AZ/EL 
        casa::MDirection itsActualAzEl;

        /// The polarisation axis angle.
        casa::Quantity itsPolAngle;

        /// True if antenna was within tolerance thresholds of the target
        /// trajectory throughout the entire integration cycle. If this is
        /// false then all data from this antenna should be flagged.
        casa::Bool itsOnSource;

        /// True if hardware monitoring reveals a problem (eg. LO out of lock)
        /// that means all data from this antenna should be flagged. If this
        /// is true, other metadata for this antenna may be invalid.
        casa::Bool itsFlagged;
};

}
}

#endif
