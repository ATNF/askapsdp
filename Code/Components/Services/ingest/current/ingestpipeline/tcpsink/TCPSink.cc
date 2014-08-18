/// @file TCPSink.cc
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

// Include own header file first
#include "TCPSink.h"

// Include package level header file
#include "askap_cpingest.h"

// System includes
#include <string>
#include <stdint.h>

// ASKAPsoft includes
#include "boost/scoped_ptr.hpp"
#include "boost/thread/mutex.hpp"
#include "boost/thread/condition.hpp"
#include "boost/asio.hpp"
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "askap/AskapUtil.h"
#include "cpcommon/VisChunk.h"

// Casecore includes
#include "casa/aips.h"

// Local package includes
#include "configuration/Configuration.h" // Includes all configuration attributes too

ASKAP_LOGGER(logger, ".TCPSink");

using namespace askap;
using namespace casa;
using namespace askap::cp::common;
using namespace askap::cp::ingest;
using boost::asio::ip::tcp;

//////////////////////////////////
// Public methods
//////////////////////////////////

TCPSink::TCPSink(const LOFAR::ParameterSet& parset,
                 const Configuration& config)
    : itsParset(parset), itsSocket(itsIOService)
{
    ASKAPLOG_DEBUG_STR(logger, "Constructor");
    itsThread.reset(new boost::thread(boost::bind(&TCPSink::runSender, this)));
}

TCPSink::~TCPSink()
{
    ASKAPLOG_DEBUG_STR(logger, "Destructor");
    if (itsThread.get()) {
        itsThread->interrupt();
        itsThread->join();
    }
}

void TCPSink::process(VisChunk::ShPtr chunk)
{
    // 1: Try to acquire the mutex protecting the buffer. Don't wait because we
    // don't want to block the main thread
    boost::mutex::scoped_lock lock(itsMutex, boost::try_to_lock);
    if (!lock) return;

    // 2: Serialise the VisChunk to a byte-array
    itsBuf.clear();
    serialiseVisChunk(*chunk, itsBuf);

    // 3: Release the lock and signal the network sender thread
    lock.unlock();
    itsCondVar.notify_all();
}

//////////////////////////////////
// Private methods
//////////////////////////////////

template <typename T>
void TCPSink::pushBack(const T src, std::vector<uint8_t>& dest)
{
    const size_t idx = dest.size(); // Must be before resize
    const size_t nbytes = sizeof(T);
    dest.resize(dest.size() + nbytes);
    memcpy(&dest[idx], &src, nbytes);
}

template <typename T>
void TCPSink::pushBackArray(const casa::Array<T>& src, std::vector<uint8_t>& dest)
{
    const size_t idx = dest.size(); // Must be before resize
    const size_t nbytes = src.size() * sizeof(T);
    dest.resize(dest.size() + nbytes);
    memcpy(&dest[idx], src.data(), nbytes);
}

template <typename T>
void TCPSink::pushBackVector(const std::vector<T>& src, std::vector<uint8_t>& dest)
{
    const size_t idx = dest.size(); // Must be before resize
    const size_t nbytes = src.size() * sizeof(T);
    dest.resize(dest.size() + nbytes);
    memcpy(&dest[idx], src.data(), nbytes);
}

void TCPSink::serialiseVisChunk(const askap::cp::common::VisChunk& chunk, std::vector<uint8_t>& v)
{
    pushBack<uint32_t>(chunk.nRow(), v);
    pushBack<uint32_t>(chunk.nChannel(), v);
    pushBack<uint32_t>(chunk.nPol(), v);
    pushBack<uint64_t>(askap::epoch2bat(MEpoch(chunk.time(), MEpoch::UTC)), v);

    pushBack<double>(chunk.channelWidth(), v);
    pushBackArray<double>(chunk.frequency(), v);

    pushBackArray<uint32_t>(chunk.antenna1(), v);
    pushBackArray<uint32_t>(chunk.antenna2(), v);
    pushBackArray<uint32_t>(chunk.beam1(), v);

    // Stokes - Map from casa:StokesTypes to 0=XX, 1=XY, 2=YX, 3=YY
    vector<uint32_t> stokesvec;
    const casa::Vector<casa::Stokes::StokesTypes>& casaStokes = chunk.stokes();
    for (size_t i = 0; i < casaStokes.size(); ++i) {
        stokesvec.push_back(mapStokes(casaStokes[i]));
    }
    pushBackVector<uint32_t>(stokesvec, v);

    // Visibilities
    pushBackArray< std::complex<float> >(chunk.visibility(), v);

    // Treat bool more specifically because there is no guarantee how they are
    // represented in memory
    const casa::Bool* data = chunk.flag().data();
    vector<uint8_t> flagvec(chunk.flag().size(), 0);
    for (size_t i = 0; i < flagvec.size(); ++i) {
        if (data[i]) flagvec[i] = 1;
    }
    pushBackVector<uint8_t>(flagvec, v);
}

void TCPSink::runSender()
{
    while (!boost::this_thread::interruption_requested()) {
        boost::mutex::scoped_lock lock(itsMutex);
        while (itsBuf.empty()) {
            itsCondVar.wait(lock);
        }

        if (boost::this_thread::interruption_requested()) break;

        bool connected = itsSocket.is_open();
        if (!connected) {
            lock.unlock();
            connected = connect();
            lock.lock();
        }

        if (boost::this_thread::interruption_requested()) break;

        if (connected) {
            boost::system::error_code error;
            boost::asio::write(itsSocket, boost::asio::buffer(itsBuf), error);
            if (error) {
                ASKAPLOG_WARN_STR(logger, "Send failed: " << error.message());
                itsSocket.close();
            }
        }

        // Reset the buffer, even if the connect/send failed so the loop will not try
        // to reconnect/resend until the next integration cycle
        itsBuf.clear();
    }
    ASKAPLOG_DEBUG_STR(logger, "TCP sender thread exiting");
}

bool TCPSink::connect(void)
{
    boost::system::error_code error;

    // Query the nameservice
    const std::string hostname = itsParset.getString("dest.hostname");
    const std::string port = itsParset.getString("dest.port");
    tcp::resolver resolver(itsIOService);
    tcp::resolver::query query(tcp::v4(), hostname, port);

    // Get the remote endpoint
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query, error);
    if (error) {
        ASKAPLOG_WARN_STR(logger, "Resolver failed: " << error.message());
        return false;
    }

    // Connect
    boost::asio::connect(itsSocket, endpoint_iterator, error);
    if (error) {
        ASKAPLOG_WARN_STR(logger, "Connect to '" << hostname << ":"
                          << port << "' failed: " << error.message());
        itsSocket.close();
        return false;
    }
    return true;
}

uint32_t TCPSink::mapStokes(casa::Stokes::StokesTypes type)
{
    switch (type) {
        case Stokes::XX: return 0;
            break;
        case Stokes::XY: return 1;
            break;
        case Stokes::YX: return 2;
            break;
        case Stokes::YY: return 3;
            break;
        default:         ASKAPTHROW(AskapError, "Unsupported stokes type");
            break;
    }
}
