/// @file FrtCommunicator.h
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
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#ifndef ASKAP_CP_INGEST_FRTCOMMUNICATOR_H
#define ASKAP_CP_INGEST_FRTCOMMUNICATOR_H

// ASKAPsoft includes
#include "frtmetadata/FrtMetadataOutputPort.h"
#include "Common/ParameterSet.h"

// local includes
#include "configuration/Configuration.h" // Includes all configuration attributes too
#include "ingestpipeline/phasetracktask/FrtMetadataSource.h"

// casa includes
#include "casa/aips.h"
#include "casa/Quanta/MVEpoch.h"
#include <casa/Arrays/Vector.h>

// std includes
#include <vector>
#include <string>
#include <map>

// boost includes
#include <boost/utility.hpp>


namespace askap {
namespace cp {
namespace ingest {

/// @brief ice communication pattern and flag management
/// @details Different approaches to fringe rotation (derivative of IFrtApproach) all
/// use the same protocol of talking to the OSL scripts via ice and manage flags per
/// antenna while such requests are in progress. It is handy to incapsulate this activity
/// in a single class. The initial plan was to do this anynchronously in a parallel thread.
/// However, given that some implementation details are closely connected to the correlator cycle
/// time (and expected to have the same latency), initially a synchronous approach has been adopted
class FrtCommunicator : public boost::noncopyable {
    public:
        /// @brief Constructor.
        /// @param[in] parset the configuration parameter set.
        /// @param[in] config configuration
        FrtCommunicator(const LOFAR::ParameterSet& parset, const Configuration& config);

        /// @brief request DRx delay
        /// @param[in] ant antenna index
        /// @param[in] delay delay setting (in the units required by hardware)
        void setDRxDelay(const casa::uInt ant, const int delay);

        /// @brief request FR setting
        /// @details upload hardware fringe rotator parameters
        /// @param[in] ant antenna index
        /// @param[in] phaseRate phase rate to set (in the units required by hardware)
        /// @param[in] phaseSlope phase slope to set (in the units required by hardware)
        /// @param[in] phaseOffset phase offset to set (in the units required by hardware)
        void setFRParameters(const casa::uInt ant, const int phaseRate, const int phaseSlope, const int phaseOffset);

        /// @brief simultaneously request both DRx and FR setting
        /// @details upload hardware fringe rotator parameters and DRx delays in a single call
        /// @param[in] ant antenna index
        /// @param[in] delay delay setting (in the units required by hardware)
        /// @param[in] phaseRate phase rate to set (in the units required by hardware)
        /// @param[in] phaseSlope phase slope to set (in the units required by hardware)
        /// @param[in] phaseOffset phase offset to set (in the units required by hardware)
        void setDRxAndFRParameters(const casa::uInt ant, const int delay, const int phaseRate, const int phaseSlope, const int phaseOffset);

        /// @brief get requested DRx delay
        /// @param[in] ant antenna index
        /// @return DRx delay setting
        int requestedDRxDelay(const casa::uInt ant) const;

        /// @brief get requested FR phase rate
        /// @param[in] ant antenna index
        /// @return FR phase rate (in hardware units)
        int requestedFRPhaseRate(const casa::uInt ant) const;

        /// @brief get requested FR phase frequency slope
        /// @param[in] ant antenna index
        /// @return FR phase frequency slope (in hardware units)
        int requestedFRPhaseSlope(const casa::uInt ant) const;

        /// @brief get requested FR phase offset
        /// @param[in] ant antenna index
        /// @return FR phase offset (in hardware units)
        int requestedFRPhaseOffset(const casa::uInt ant) const;

        /// @brief get the BAT of the last FR parameter update
        /// @param[in] ant antenna index
        /// @return BAT when the last update was implemented
        uint64_t lastFRUpdateBAT(const casa::uInt ant) const;

        /// @brief test if antenna produces valid data
        /// @param[in] ant antenna index
        /// @return true, if the given antenna produces valid data
        bool isValid(const casa::uInt ant) const;

        /// @brief test if antenna is uninitialised
        /// @param[in] ant antenna index
        /// @return true, if the given antenna is uninitialised
        bool isUninitialised(const casa::uInt ant) const;

        /// @brief invalidate the antenna
        /// @param[in] ant antenna index
        void invalidate(const casa::uInt ant);

        /// @brief signal of the new time stamp
        /// @details Without asynchronous thread, the current implementation relies on this method
        /// being called every cycle. It manages time outs and flags/unflags antennas as necessary.
        void newTimeStamp(const casa::MVEpoch &epoch);

    protected:

        /// @brief helper method to form a message to set fringe rotation parameters
        /// @param[in] ant antenna index
        /// @param[in] phaseRate phase rate to set (in the units required by hardware)
        /// @param[in] phaseSlope phase slope to set (in the units required by hardware)
        /// @param[in] phaseOffset phase offset to set (in the units required by hardware)
        /// @return map with the message
        std::map<std::string, int> getFRParametersMsg(const casa::uInt ant, const int phaseRate,
                                                      const int phaseSlope, const int phaseOffset);

        /// @brief helper method to form a message to set DRx delay
        /// @param[in] ant antenna index
        /// @param[in] delay delay setting (in the units required by hardware)
        /// @return map with the message
        std::map<std::string, int> getDRxDelayMsg(const casa::uInt ant, const int delay);

        /// @brief helper method to tag a message with time-based ID
        /// @details We need to be able to track which requests are completed and when. It is done
        /// by passing an ID string which is buffered per antenna. When reply is received, the post-processing
        /// actions are finalised. This method forms an ID based on current epoch, tags the message and
        /// returns the id
        /// @param[in] msg message to update
        /// @return assigned ID (same as msg["id"])
        int tagMessage(std::map<std::string, int> &msg) const;

    private:
        /// @brief status enum for the flag state machine
        enum AntennaFlagStatus {
            ANT_VALID = 0,
            ANT_DRX_REQUESTED,
            ANT_FR_REQUESTED,
            ANT_DRX_AND_FR_REQUESTED,
            ANT_BEING_UPDATED,
            ANT_UNINITIALISED,
            ANT_IGNORED
        };

        /// @brief flag status for each antenna
        std::vector<AntennaFlagStatus> itsAntennaStatuses;

        /// @brief request IDs for all antennas with "requested" statuses (unused value for others)
        std::vector<int> itsAntennaRequestIDs;

        /// @brief times of the request passing through for each antenna in the "being updated" status
        casa::Vector<casa::MVEpoch> itsRequestCompletedTimes;

        /// @brief BAT of the last update of the hardware fringe rotator parameters
        std::vector<uint64_t> itsFRUpdateBATs;

        /// @brief antenna names as setup in the configuration
        /// @details They are used to form a string key in the form akXX.param.
        std::vector<std::string> itsAntennaNames;

        /// @brief requested or current DRx delays
        std::vector<int> itsRequestedDRxDelays;

        /// @brief requested or current FR phase rates
        std::vector<int> itsRequestedFRPhaseRates;

        /// @brief requested or current FR frequency phase slopes
        std::vector<int> itsRequestedFRPhaseSlopes;

        /// @brief requested or current FR phase offsets
        std::vector<int> itsRequestedFRPhaseOffsets;

        /// @brief output port for ice communication
        boost::shared_ptr<icewrapper::FrtMetadataOutputPort> itsOutPort;

        /// @brief input port for ice communication
        boost::shared_ptr<FrtMetadataSource> itsInPort;

        /// @brief number of cycles to wait after the request has come through
        /// @details It takes 5 cycles or or so for the change to propagate
        /// through the system. This class implements a delay before unflagging
        /// a particular antenna. This field determines how log to wait (in 5 sec cycles)
        casa::uInt itsCyclesToWait;

        /// @brief message counter
        mutable int itsMsgCounter;
}; // class FrtCommunicator

} // namespace ingest
} // namespace cp
} // namespace askap

#endif // #ifndef ASKAP_CP_INGEST_FRTCOMMUNICATOR_H

