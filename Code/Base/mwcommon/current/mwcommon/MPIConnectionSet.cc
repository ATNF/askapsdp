/// @file
/// @brief Set of MPI connections
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

// System includes
#include <vector>

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "Blob/BlobString.h"

// Local package includes
#include "mwcommon/MPIConnectionSet.h"

namespace askap { namespace mwcommon {

  MPIConnectionSet::MPIConnectionSet()
  {}

  MPIConnectionSet::~MPIConnectionSet()
  {}

  MWConnectionSet::ShPtr
  MPIConnectionSet::clone (const std::vector<int>& inx) const
  {
    int nrconn = size();
    MPIConnectionSet* set = new MPIConnectionSet();
    MWConnectionSet::ShPtr mwset(set);
    for (std::vector<int>::const_iterator it=inx.begin();
         it!=inx.end();
         ++it) {
      int i = *it;
      ASKAPASSERT (i>=0 && i<nrconn);
      set->itsConns.push_back (itsConns[i]);
    }
    return mwset;
  }

  int MPIConnectionSet::addConnection (int rank, int tag)
  {
    int seqnr = itsConns.size();
    MPIConnection::ShPtr ptr(new MPIConnection (rank, tag));
    itsConns.push_back (ptr);
    return seqnr;
  }

  int MPIConnectionSet::size() const
  {
    return itsConns.size();
  }

  int MPIConnectionSet::getReadyConnection()
  {
    return -1;
  }

  void MPIConnectionSet::read (int seqnr, LOFAR::BlobString& buf)
  {
    itsConns[seqnr]->read (buf);
  }

  void MPIConnectionSet::write (int seqnr, const LOFAR::BlobString& buf)
  {
    itsConns[seqnr]->write (buf);
  }

  void MPIConnectionSet::writeAll (const LOFAR::BlobString& buf)
  {
    for (unsigned i=0; i<itsConns.size(); ++i) {
      itsConns[i]->write (buf);
    }
  }

}} // end namespaces
