//# MemConnectionSet.cc: Set of Memory connections
//#
//# @copyright (c) 2007 CONRAD, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <mwcommon/MemConnectionSet.h>
#include <mwcommon/MWError.h>


namespace conrad { namespace cp {

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
      CONRADASSERT (i>=0 && i<nrconn);
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
