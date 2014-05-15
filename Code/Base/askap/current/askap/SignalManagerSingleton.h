/// @file SignalManagerSingleton.h
/// @brief
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

#ifndef ASKAP_SIGNALMANAGERSINGLETON_H
#define ASKAP_SIGNALMANAGERSINGLETON_H

// System includes
#include <csignal>
#include <vector>

// Local package includes
#include "ISignalHandler.h"

namespace askap {

    /// @brief A simple object-oriented wrapper around the standard ANSI C
    /// signal mechanism.
    class SignalManagerSingleton {
        public:
            /// @brief Obtain the singleton instance of the signal manager.
            /// @return the singleton instance.
            static SignalManagerSingleton* instance(void);

            /// @brief Register an object (which implements the ISignalHandler
            /// interface) to handle signals.
            ///
            /// @param[in] signum   signal to regester handler for.
            /// @param[in] handler  instance of a signal handler.
            /// @note The signal handler is not copied, so must remain allocated
            ///       while the handler is registered.
            ISignalHandler *registerHandler(int signum,
                                            ISignalHandler *handler);

            /// @brief Remove a signal handler.
            /// @param[in] signum   signal for which the handler will be
            ///                     removed. The signal will be ignored
            ///                     (i.e. SIG_IGN) after this call returns.
            void removeHandler(int signum);

        private:
            // Constructor
            SignalManagerSingleton();

            // Dispatches to the handler object
            static void dispatcher(int signum);

            // No support for assignment
            SignalManagerSingleton& operator=(const SignalManagerSingleton& rhs);

            // No support for copy constructor
            SignalManagerSingleton(const SignalManagerSingleton& src);

            // Singleton instance of this class
            static SignalManagerSingleton *itsInstance;

            // Vector of signal handlers. This vector gets sized to the maximum
            // number of signals (NSIG).
            static std::vector<ISignalHandler *> itsSignalHandlers;
    };

} // end namespace askap

#endif
