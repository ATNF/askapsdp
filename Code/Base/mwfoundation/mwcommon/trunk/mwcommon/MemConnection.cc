//# MemConnection.cc: Memory connection to a worker
//#
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
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <mwcommon/MemConnection.h>
#include <mwcommon/MWError.h>

namespace askap { namespace mwbase {

  MemConnection::MemConnection (const WorkerProxy::ShPtr& worker)
    : itsWorker (worker)
  {}

  MemConnection::~MemConnection()
  {}

  int MemConnection::getMessageLength()
  {
    ASKAPCHECK (itsResult.size() > 0,
		 "MemConnection: no result has been received");
    return itsResult.size();
  }

  void MemConnection::receive (void* buf, unsigned size)
  {
    ASKAPASSERT (itsResult.size() == size);
    memcpy (buf, itsResult.data(), size);
    // Clear buffer to make sure data cannot be read twice.
    itsResult.resize (0);
  }

  void MemConnection::write (const LOFAR::BlobString& data)
  {
    // Internal buffer must be empty, otherwise no read was done.
    ASKAPCHECK (itsResult.size() == 0,
		 "MemConnection: received result has not been read");
    // Let the worker process the data and keep its result.
    itsWorker->handleMessage (data, itsResult);
  }

  void MemConnection::send (const void*, unsigned)
  {
    ASKAPTHROW (MWError, "MemConnection::send should not be called");
  }

}} // end namespaces
