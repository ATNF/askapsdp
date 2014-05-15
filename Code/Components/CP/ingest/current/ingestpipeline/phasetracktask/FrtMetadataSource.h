/// @file FrtMetadataSource.h
///
/// @brief This class is an adaptation of the original Ben's MetadataSource
/// but it deals with a different datatype. Perhaps we could've refactor this
/// code (e.g. with templates) to get a generic version. This class is intended
/// to receive fringe rotator and DRx specific messages to allow ingest pipeline
/// to control fringe rotation. We probably don't need buffering, but reuse
/// of Ben's thread-safe circular buffer seems to make things easier.
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
/// @author Max Voronkov <Max.Voronkov@csiro.au>

#ifndef ASKAP_CP_INGEST_FRTMETADATASOURCE_H
#define ASKAP_CP_INGEST_FRTMETADATASOURCE_H

// System includes
#include <string>
#include <map>

// ASKAPsoft includes
#include "boost/shared_ptr.hpp"
#include "frtmetadata/FrtMetadataReceiver.h"

// Local package includes
#include "ingestpipeline/sourcetask/CircularBuffer.h"

namespace askap {
namespace cp {
namespace ingest {

/// @brief This class is an adaptation of the original Ben's MetadataSource
/// but it deals with a different datatype. Perhaps we could've refactor this
/// code (e.g. with templates) to get a generic version. This class is intended
/// to receive fringe rotator and DRx specific messages to allow ingest pipeline
/// to control fringe rotation. We probably don't need buffering, but reuse
/// of Ben's thread-safe circular buffer seems to make things easier.
class FrtMetadataSource :
    virtual public askap::cp::icewrapper::FrtMetadataReceiver,
    boost::noncopyable {
    public:
        /// @brief Constructor.
        ///
        /// @param[in] locatorHost  the hostname or IP address of the host that
        ///                         the ICE locator service is running on.
        /// @param[in] locatorPort  the port number the ICE locator service is
        ///                         running on.
        /// @param[in] topicManager the name of the IceStorm topic manager.
        /// @param[in] topic        the topic name of the IceStorm topic which
        ///                         should be subscribed to.
        /// @param[in] adapterName  the name of the adapter. This is a quirk of
        ///                         IceStorm being built on top of Ice. Subscribers
        ///                         need to be connected via an adapter which is
        ///                         identified by a name.
        /// @param[in] bufSize      the number of TosMetadata objects to buffer
        ///                         internally. If objects are being received faster
        ///                         than they are being consumed, and if this buffer
        ///                         becomes full then the older objects are discarded
        ///                         to make room for the newer incoming objects.
        FrtMetadataSource(const std::string& locatorHost,
                          const std::string& locatorPort,
                          const std::string& topicManager,
                          const std::string& topic,
                          const std::string& adapterName,
                          const unsigned int bufSize);

        /// @brief Callback method, called when a new message is available.
        ///
        /// @param[in] msg  the metadata object.
        virtual void receive(const std::map<std::string, int>& msg);

        /// @brief advance to the next element in the buffer
        /// @details
        /// @param timeout timeout in microseconds, negative value causes blocking
        /// @return shared pointer to next message (empty pointer is returned in the
        /// case of timeout)
        boost::shared_ptr<std::map<std::string, int> > next(const long timeout = -1);

    private:
        // Circular buffer of message maps objects
        askap::cp::ingest::CircularBuffer<std::map<std::string, int> > itsBuffer;

};

}
}
}

#endif // #ifndef ASKAP_CP_INGEST_FRTMETADATASOURCE_H

