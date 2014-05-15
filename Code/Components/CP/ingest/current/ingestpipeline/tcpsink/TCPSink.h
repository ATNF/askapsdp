/// @file TCPSink.h
///
/// @copyright (c) 2014 CSIRO
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

#ifndef ASKAP_CP_INGEST_TCPSINK_H
#define ASKAP_CP_INGEST_TCPSINK_H

// Std includes
#include <vector>
#include <string>
#include <stdint.h>

// ASKAPsoft includes
#include "boost/scoped_ptr.hpp"
#include "boost/thread/mutex.hpp"
#include "boost/thread/condition.hpp"
#include "boost/thread.hpp"
#include "boost/asio.hpp"
#include "Common/ParameterSet.h"
#include "cpcommon/VisChunk.h"
#include "measures/Measures/Stokes.h"

// Local package includes
#include "ingestpipeline/ITask.h"
#include "configuration/Configuration.h" // Includes all configuration attributes too

namespace askap {
namespace cp {
namespace ingest {

/// @brief A sink task for the central processor ingest pipeline which writes
/// the VisChunk to a TCP network port.
class TCPSink : public askap::cp::ingest::ITask {
    public:
        /// @brief Constructor.
        /// @param[in] parset   the parameter set used to configure this task.
        /// @param[in] config   an object containing the system configuration.
        TCPSink(const LOFAR::ParameterSet& parset,
                const Configuration& config);

        /// @brief Destructor.
        virtual ~TCPSink();

        /// @brief Writes out the data in the VisChunk parameter to the
        /// measurement set.
        ///
        /// @param[in,out] chunk    the instance of VisChunk to send. Note
        ///                         the VisChunk pointed to by "chunk" nor the pointer
        ///                         itself are modified by this function.
        virtual void process(askap::cp::common::VisChunk::ShPtr chunk);

    private:

        /// No support for assignment
        TCPSink& operator=(const TCPSink& rhs);

        /// No support for copy constructor
        TCPSink(const TCPSink& src);

        /// The main loop for the "sender" thread
        void runSender(void);

        /// Attempted to connect to the destination
        /// @return true if connection succeeded, otherwise false
        bool connect(void);

        /// Serialise a "VisChunk" to a byte-array for asynchronous sending to
        /// the specified destination
        static void serialiseVisChunk(const askap::cp::common::VisChunk& chunk,
                                      std::vector<uint8_t>& v);

        /// Is used to append the bytes for a primative type to the
        /// byte vector
        template <typename T>
        static void pushBack(const T src, std::vector<uint8_t>& dest);

        /// Is used to append the bytes for a CASA Array to the byte vector
        /// The array elements should be primative types
        template <typename T>
        static void pushBackArray(const casa::Array<T>& src, std::vector<uint8_t>& dest);

        /// Is used to append the bytes for a STL vector to the byte vector.
        /// The array elements should be primative types
        template <typename T>
        static void pushBackVector(const std::vector<T>& src, std::vector<uint8_t>& dest);

        static uint64_t convertToBAT(const casa::MVEpoch& time);

        static uint32_t mapStokes(casa::Stokes::StokesTypes type);

        /// Parameter set
        const LOFAR::ParameterSet itsParset;

        /// Buffer to send - This is shared between the producer (main thread)
        /// and the consumer (sender thread) so no double buffering. Only the owner
        /// of "itsMutex" should read/write this buffer.
        std::vector<uint8_t> itsBuf;

        /// Mutex used to synchronise access to "itsBuf"
        boost::mutex itsMutex;

        /// Condition variable used for signalling between the main thread and
        /// the network sender thread
        boost::condition itsCondVar;

        /// io_service
        boost::asio::io_service itsIOService;

        /// Network socket
        boost::asio::ip::tcp::socket itsSocket;

        /// Network sender thread
        boost::scoped_ptr<boost::thread> itsThread;
};

}
}
}

#endif
