/// @file SignalCounter.h
/// @brief A simple signal handler which counts signals recieved.
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
/// @author Ben Humphreys<ben.humphreys@csiro.au>
///

#ifndef ASKAP_SIGNALCOUNTER_H
#define ASKAP_SIGNALCOUNTER_H

#include "ISignalHandler.h"

namespace askap {

    /// @brief A simple signal handler which counts signals recieved.
    class SignalCounter : public ISignalHandler {
        public:
            /// Constructor
            SignalCounter();

            /// Destructor
            virtual ~SignalCounter();

            /// @brief Callback function which is called upon receipt of
            /// a signal.
            /// @param[in] signum   the signal number of the signal which resulted
            ///                     in this callback being called. See manpage
            ///                     signal(3) for a list of signal numbers and
            ///                     their meaning.
            virtual void handleSignal(int signum);

            /// @brief Get the number of times the signal has been received.
            /// @return the number of times the signal was received since this
            /// object was registered with the signal handler, or since the
            /// last call to resetCount().
            unsigned long getCount(void);

            /// @brief Reset the counter. (i.e. set to zero).
            void resetCount(void);

        private:
            // Count for number of signals received
            unsigned long itsCount;
    };

} // end namespace askap

#endif
