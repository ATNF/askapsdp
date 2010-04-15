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


#include <mwcommon/MWConnection.h>
#include <Blob/BlobString.h>
#include <Blob/BlobHeader.h>


namespace askap { namespace mwbase {

    MWConnection::~MWConnection()
    {}

    void MWConnection::init()
    {}

    bool MWConnection::isConnected() const
    {
        return true;
    }

    void MWConnection::read (LOFAR::BlobString& buf)
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

}} // end namespaces
