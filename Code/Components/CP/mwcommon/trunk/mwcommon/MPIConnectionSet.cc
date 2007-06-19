//# MPIConnectionSet.cc: Set of MPI connections
//#
//# @copyright (c) 2007 CONRAD, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <mwcommon/MPIConnectionSet.h>
#include <mwcommon/MWError.h>


namespace conrad { namespace cp {

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
      CONRADASSERT (i>=0 && i<nrconn);
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
