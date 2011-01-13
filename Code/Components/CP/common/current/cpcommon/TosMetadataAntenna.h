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
#include "casa/Arrays/Vector.h"
#include "casa/Arrays/Cube.h"
#include "measures/Measures/MDirection.h"

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
        /// @param[in] nCoarseChannels  number of coarse channels.
        /// @param[in] nBeams   number of beams.
        /// @param[in] nPol     number of polarisations.
        TosMetadataAntenna(const casa::String& name,
                           const casa::uInt& nCoarseChannels,
                           const casa::uInt& nBeams,
                           const casa::uInt& nPol);

        /// @brief Get the name of the antenna.
        /// @return the name of the antenna.
        casa::String name(void) const;

        /// @brief Get the number of coarse channels.
        /// @return the number of coarse channels.
        casa::uInt nCoarseChannels(void) const;

        /// @brief Get the number of beams.
        /// @return the number of beams.
        casa::uInt nBeams(void) const;

        /// @brief Get the number of polarisations.
        /// @return the number of polarisations.
        casa::uInt nPol(void) const;

        /// @brief Get the target coordinates for the dish pointing
        /// @return the target coordinates for the dish pointing
        casa::MDirection targetRaDec(void) const;

        /// @brief Set the target coordinates for the dish pointing
        /// @param[in] val the target coordinates for the dish pointing
        void targetRaDec(const casa::MDirection& val);

        /// @brief Get the centre frequency for this antenna.
        /// @return the centre frequency for this antenna.
        casa::Double frequency(void) const;

        /// @brief Set the centre frequency for this antenna.
        /// @param[in] val the centre frequency for this antenna.
        void frequency(const casa::Double& val);

        /// @brief Get the client id.
        /// The client id is typically scheduling block id that the antenna
        /// is allocated to.
        ///
        /// @return the client id.
        casa::String clientId(void) const;

        /// @brief Set the client id.
        /// The client id is typically scheduling block id that the antenna
        /// @param[in] val the client id.
        void clientId(const casa::String& val);

        /// @brief Get the scan active field
        /// @return true if the antenna is carrying out a scan, otherwise
        /// false;
        casa::Bool scanActive(void) const;

        /// @brief Set the scan active field
        /// Set this to true if the antenna is carrying out a scan,
        /// otherwise false;
        void scanActive(const casa::Bool& val);

        /// @brief Get the scan id.
        /// The scan id is the TOS scan id the antenna is currently performing.
        /// @return the scan id.
        casa::String scanId(void) const;

        /// @brief Set the scan id.
        /// The scan id is the TOS scan id the antenna is currently performing.
        /// @param[in] val the scan id.
        void scanId(const casa::String& val);

        /// @brief Get the phase tracking centre for a given beam and coarse
        /// channel.
        ///
        /// @li The value of beam must be between 0 and(nBeams() - 1).
        /// @li The value of coarseChannel must be between 0 and
        ///     (nCoarseChannels() - 1).
        ///
        /// @param[in] beam the beam for which the phase tracking centre is
        ///     desired.
        ///
        /// @throw AskapError if the value of beam or coarseChannel is invalid
        ///     for this antenna.
        ///
        /// @return the phase tracking centre for the given beam and coarse
        ///     channel.
        casa::MDirection phaseTrackingCentre(const casa::uInt& beam) const;

        /// @brief Set the phase tracking centre for a given beam and
        /// coarse channel.
        ///
        /// @li The value of beam must be between 0 and(nBeams() - 1).
        /// @li The value of coarseChannel must be between 0 and
        ///     (nCoarseChannels() - 1).
        ///
        /// @param[in] beam the beam for which the phase tracking centre is
        ///     to be set.
        /// @param[in] val 
        ///
        /// @throw AskapError if the value of beam or coarseChannel is invalid
        ///     for this antenna.
        void phaseTrackingCentre(const casa::MDirection& val,
                                 const casa::uInt& beam);

        /// @brief Get the polarisation offset.
        /// @return the polarisation offset (in radians).
        casa::Double polarisationOffset(void) const;

        /// @brief Set the polarisation offset
        /// @param[in] val the polarisation offset (in radians).
        void polarisationOffset(const casa::Double& val);

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

        /// @brief Get the value of the hwError (hardware error) flag.
        ///
        /// @return true if hardware monitoring reveals a problem
        /// (eg. LO out of lock) that means all data from this antenna
        /// should be flagged.
        casa::Bool hwError(void) const;

        /// @brief Set the value of the hwError (hardware error) flag.
        /// @param[in] val teh value of the hardware error flag. Use true to
        /// indicate a hardware error, otherwise false.
        void hwError(const casa::Bool& val);

        /// @brief Get the flag value for a given beam, coarse channel,
        /// or polarisation.
        ///
        /// @note If the value of onSource() if false, or the value of
        /// hwError() is true, this detailed flagging information should
        /// be ignored and all data for this antenna for this integration
        /// should be considered bad.
        ///
        /// @li The value of beam must be between 0 and(nBeams() - 1).
        /// @li The value of coarseChannel must be between 0 and
        ///     (nCoarseChannels() - 1).
        /// @li The value of pol must be between 0 and (nPol() - 1).
        ///
        /// @param[in] beam the beam for which the flagging information is
        ///     desired.
        /// @param[in] coarseChannel the coarse channel for which the
        ///     flagging information is desired.
        /// @param[in] pol] the polarisation for which the flagging information
        ///     is desired.
        ///
        /// @throw AskapError if the value of beam, coarseChannel or pol
        ///     is invalid for this antenna.
        ///
        /// @return true if the visibility is flagged (i.e. suspect or bad),
        ///     otherwise false.
        casa::Bool flagDetailed(const casa::uInt& beam,
                                const casa::uInt& coarseChannel,
                                const casa::uInt& pol) const;

        /// @brief Set the
        ///
        /// @li The value of beam must be between 0 and(nBeams() - 1).
        /// @li The value of coarseChannel must be between 0 and
        ///     (nCoarseChannels() - 1).
        /// @li The value of pol must be between 0 and (nPol() - 1).
        ///
        /// @param[in] val true if the visibility should be flagged (i.e
        ///     treated as suspect or bad) otherwise false.
        /// @param[in] beam the beam for which the flagging information is
        ///     to be set.
        /// @param[in] coarseChannel the coarse channel for which the
        ///     flagging information is to be set.
        /// @param[in] pol] the polarisation for which the flagging information
        ///     is to be set.
        ///
        /// @throw AskapError if the value of beam, coarseChannel or pol
        ///     is invalid for this antenna.
        ///
        void flagDetailed(const casa::Bool& val,
                          const casa::uInt& beam,
                          const casa::uInt& coarseChannel,
                          const casa::uInt& pol);

        /// @brief Get the
        ///
        /// @li The value of beam must be between 0 and(nBeams() - 1).
        /// @li The value of coarseChannel must be between 0 and
        ///     (nCoarseChannels() - 1).
        /// @li The value of pol must be between 0 and (nPol() - 1).
        ///
        /// @param[in] beam the beam for which the system temperature is
        ///     desired.
        /// @param[in] coarseChannel the coarse channel for which the
        ///     system temperature is desired.
        /// @param[in] pol] the polarisation for which the system temperature 
        ///     is desired.
        ///
        /// @throw AskapError if the value of beam, coarseChannel or pol
        ///     is invalid for this antenna.
        ///
        /// @return the system temperature value for the given beam, coarse
        ///     channel and polarisation (units in Kelvin).
        casa::Float systemTemp(const casa::uInt& beam,
                               const casa::uInt& coarseChannel,
                               const casa::uInt& pol) const;

        /// @brief Set the system temperature for a given beam, coarse channel
        /// and polarisation.
        ///
        /// @li The value of beam must be between 0 and(nBeams() - 1).
        /// @li The value of coarseChannel must be between 0 and
        ///     (nCoarseChannels() - 1).
        /// @li The value of pol must be between 0 and (nPol() - 1).
        ///
        /// @param[in] val the system temperature (units in Kelvin).
        /// @param[in] beam the beam for which the system temperature is
        ///     to be set.
        /// @param[in] coarseChannel the coarse channel for which the
        ///     system temperature is to be set.
        /// @param[in] pol] the polarisation for which the system temperature 
        ///     is to be set.
        ///
        /// @throw AskapError if the value of beam, coarseChannel or pol
        ///     is invalid for this antenna.
        void systemTemp(const casa::Float& val,
                        const casa::uInt& beam,
                        const casa::uInt& coarseChannel,
                        const casa::uInt& pol);

    private:

        /// Throws an exception if the given beam is invalid.
        void checkBeam(const casa::uInt& beam) const;

        /// Throws an exception if the given coarse channel is invalid.
        void checkCoarseChannel(const casa::uInt& coarseChannel) const;

        /// Throws an exception if the given polarisation is invalid.
        void checkPol(const casa::uInt& pol) const;

        /// The name of the antenna
        casa::String itsName;

        /// The number of coarse channels
        casa::uInt itsNumCoarseChannels;

        /// The number of beams. This is the same for each
        /// coarse channel.
        casa::uInt itsNumBeams;

        /// The number of polarisations.
        casa::uInt itsNumPol;

        /// The target coordinates
        casa::MDirection itsTargetRaDec;

        /// The centre frequency (in Hz)
        casa::Double itsFrequency;

        /// The The client id (ie. normally scheduling block id) that
        /// the antenna is allocated to.
        casa::String itsClientId;

        /// True if a scan is being executed, otherwise false indicating
        /// the antenna is idle
        casa::Bool itsScanActive;

        /// The TOS scan id the antenna is currently performing.
        casa::String itsScanId;

        /// The phase tracking centre, per beam.
        casa::Vector<casa::MDirection> itsPhaseTrackingCentre;

        /// The polarisation offset (in Radians).
        casa::Double itsPolarisationOffset;

        /// True if antenna was within tolerance thresholds of the target
        /// trajectory throughout the entire integration cycle. If this is
        /// false then all data from this antenna should be flagged.
        casa::Bool itsOnSource;

        /// True if hardware monitoring reveals a problem (eg. LO out of lock)
        /// that means all data from this antenna should be flagged.
        casa::Bool itsHwError;

        /// Detailed, per beam, per coarse channel, per polarisation flagging
        /// information.
        ///
        /// @note This cube of flag data is independant of the onSource and
        /// hwError flags.
        casa::Cube<casa::Bool> itsFlagDetailed;

        /// The system temperature, per beam, per coarse channel, per
        /// polarisation.
        casa::Cube<casa::Float> itsSystemTemp;
};

}
}

#endif
