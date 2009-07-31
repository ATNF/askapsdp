//# MemConnectionSet.cc: Set of Memory connections
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

#include <mwcommon/MemConnectionSet.h>
#include <mwcommon/MWError.h>


namespace askap { namespace mwbase {

  MemConnectionSet::MemConnectionSet()
  {}

  MemConnectionSet::~MemConnectionSet()
  {}

  MWConnectionSet::ShPtr
  MemConnectionSet::clone (const std::vector<int>& inx) const
  {
    int nrconn = size();
    MemConnectionSet* set = new MemConnectionSet();
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

  int MemConnectionSet::addConnection (const WorkerProxy::ShPtr& worker)
  {
    int seqnr = itsConns.size();
    itsConns.push_back (MemConnection::ShPtr (new MemConnection(worker)));
    return seqnr;
  }

  int MemConnectionSet::size() const
  {
    return itsConns.size();
  }

  int MemConnectionSet::getReadyConnection()
  {
    return -1;
  }

  void MemConnectionSet::read (int seqnr, LOFAR::BlobString& buf)
  {
    itsConns[seqnr]->read (buf);
  }

  void MemConnectionSet::write (int seqnr, const LOFAR::BlobString& buf)
  {
    itsConns[seqnr]->write (buf);
  }

  void MemConnectionSet::writeAll (const LOFAR::BlobString& buf)
  {
    for (unsigned i=0; i<itsConns.size(); ++i) {
      itsConns[i]->write (buf);
    }
  }

}} // end namespaces
