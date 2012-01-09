/// @file
/// @brief Abstract base class for all MWConnections
///
/// @copyright (c) 2007 CSIRO
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
/// @author Ger van Diepen <diepen@astron.nl>


// ASKAPsoft includes
#include "Blob/BlobString.h"
#include "Blob/BlobHeader.h"

// Local package includes
#include "mwcommon/MWConnection.h"

namespace askap { namespace mwcommon {

    MWConnection::~MWConnection()
    {}

    void MWConnection::init()
    {}

    bool MWConnection::isConnected() const
    {
        return true;
    }

    void MWConnection::read(LOFAR::BlobString& buf)
    {
        unsigned long size = 0;
        receive(&size, sizeof(unsigned long));

        buf.resize(size);
        receive(buf.data(), size);
    }

    void MWConnection::write (const LOFAR::BlobString& buf)
    {
        // First send the length of the message
        const unsigned long size = buf.size();
        send(&size, sizeof(unsigned long));
        send(buf.data(), size);
    }
    
    /// @brief broadcast blob to all ranks by the connected MWConnection
    /// @details this method waits until all data has arrived into \a buf.
    /// The buffer is resized as needed.
    /// @param[in] buf blob string
    /// @param[in] root root rank which has the data
    void MWConnection::broadcast(LOFAR::BlobString& buf, int root)
    {
       // first broadcast the length of the message
       unsigned long size = isRoot(root) ? buf.size() : 0;
       bcast(&size,sizeof(unsigned long),root);
       
       if (!isRoot(root)) {
           buf.resize(size);
       }
       bcast(buf.data(), size, root);
    }
    

}} // end namespaces
