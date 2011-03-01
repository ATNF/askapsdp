/// @file UVChannelDataSelector.h
/// @brief An implementation of the IDataSelector interface for the
/// uv-channel.
///
/// @copyright (c) 2011 CSIRO
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

#ifndef ASKAP_CP_CHANNELS_UVCHANNEL_VISSTREAM_DATA_SELECTOR_H
#define ASKAP_CP_CHANNELS_UVCHANNEL_VISSTREAM_DATA_SELECTOR_H

// ASKAPsoft includes
#include "boost/shared_ptr.hpp"
#include "casa/aipstype.h"
#include "dataaccess/IDataSelector.h"

namespace askap {
namespace cp {
namespace channels {

/// @brief An implementation of the IDataSelector interface for the
/// uv-channel.
///
/// @details This implementation currently supports no selection. All
/// "choose" methods throw an exception when called.
class UVChannelDataSelector : public askap::synthesis::IDataSelector {
    public:
        explicit UVChannelDataSelector();

        /// @brief Choose a single feed, the same for both antennae
        /// @param[in] feedID the sequence number of feed to choose
        virtual void chooseFeed(casa::uInt feedID);

        /// @brief Choose a single baseline
        /// @param[in] ant1 the sequence number of the first antenna
        /// @param[in] ant2 the sequence number of the second antenna
        /// Which one is the first and which is the second is not important
        virtual void chooseBaseline(casa::uInt ant1, casa::uInt ant2);

        /// @brief Choose autocorrelations only
        virtual void chooseAutoCorrelations();

        /// @brief Choose crosscorrelations only
        virtual void chooseCrossCorrelations();

        /// @brief Choose samples corresponding to a uv-distance larger than threshold
        /// @details This effectively rejects the baselines giving a smaller
        /// uv-distance than the specified threshold (in metres)
        /// @param[in] uvDist threshold
        virtual void chooseMinUVDistance(casa::Double uvDist);

        /// @brief Choose samples corresponding to a uv-distance smaller than threshold
        /// @details This effectively rejects the baselines giving a larger
        /// uv-distance than the specified threshold (in metres)
        /// @param[in] uvDist threshold
        virtual void chooseMaxUVDistance(casa::Double uvDist);

        /// @brief Choose a subset of spectral channels
        /// @param[in] nChan a number of spectral channels wanted in the output
        /// @param[in] start the number of the first spectral channel to choose
        /// @param[in] nAvg a number of adjacent spectral channels to average
        ///             default is no averaging
        virtual void chooseChannels(casa::uInt nChan,
                                    casa::uInt start, casa::uInt nAvg = 1);

        /// @brief Choose a subset of frequencies. The reference frame is
        /// defined by the DataSource object
        /// @param[in] nChan a number of spectral channels wanted in the output
        /// @param[in] start the frequency of the first spectral channel to
        ///        choose (given as casa::MVFrequency object)
        /// @param[in] freqInc an increment in terms of the frequency in the
        ///        same reference frame as start. This parameter plays
        ///        the same role as nAvg for chooseChannels, i.e. twice
        ///        the frequency resolution would average two adjacent channels
        virtual void chooseFrequencies(casa::uInt nChan,
                                       const casa::MVFrequency &start,
                                       const casa::MVFrequency &freqInc);

        /// @brief Choose a subset of radial velocities. The reference frame is
        /// defined by the DataSource object
        /// @param[in] nChan a number of spectral channels wanted in the output
        /// @param[in] start the velocity of the first spectral channel to
        ///        choose (given as casa::MVRadialVelocity object)
        /// @param[in] velInc an increment in terms of the radial velocity in the
        ///        same reference frame as start. This parameter plays
        ///        the same role as nAvg for chooseChannels, i.e. twice
        ///        the velocity resolution would average two adjacent channels
        virtual void chooseVelocities(casa::uInt nChan,
                                      const casa::MVRadialVelocity &start,
                                      const casa::MVRadialVelocity &velInc);

        /// @brief Choose a single spectral window (also known as IF).
        /// @param[in] spWinID the ID of the spectral window to choose
        virtual void chooseSpectralWindow(casa::uInt spWinID);

        /// @brief Choose a time range. The behavior for streams needs thinking.
        /// Probably the iterator should just ignore all data before the
        /// start time range and flags the end as soon as the time passed
        /// the stop time. Both start and stop times are given via
        /// casa::MVEpoch object. The reference frame is specified by
        /// the DataSource object.
        /// @param[in] start the beginning of the chosen time interval
        /// @param[in] stop  the end of the chosen time interval
        virtual void chooseTimeRange(const casa::MVEpoch &start,
                                     const casa::MVEpoch &stop);

        /// @brief Choose time range. This method accepts a time range with
        /// respect to the origin defined by the DataSource object.
        /// Both start and stop times are given as Doubles.
        /// The reference frame is the same as for the version accepting
        /// MVEpoch and is specified via the DataSource object.
        /// @param[in] start the beginning of the chosen time interval
        /// @param[in] stop the end of the chosen time interval
        virtual void chooseTimeRange(casa::Double start, casa::Double stop);

        /// @brief Choose polarization.
        /// @param pols a string describing the wanted polarization
        /// in the output. Allowed values are: I, "IQUV","XXYY","RRLL"
        virtual void choosePolarizations(const casa::String &pols);

        /// @brief Choose cycles. This is an equivalent of choosing the time
        /// range, but the selection is done in integer cycle numbers
        /// @param[in] start the number of the first cycle to choose
        /// @param[in] stop the number of the last cycle to choose
        virtual void chooseCycles(casa::uInt start, casa::uInt stop);
};

} // namespace channels
} // namespace cp
} // namespace askap

#endif
