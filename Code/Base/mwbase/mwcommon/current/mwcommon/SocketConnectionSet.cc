/// @file
/// @brief Set of socket connections
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


#include <mwcommon/SocketConnectionSet.h>
#include <mwcommon/MWError.h>


namespace askap { namespace mwbase {

  SocketConnectionSet::SocketConnectionSet (const std::string& port)
    : itsListener (port)
  {}

  SocketConnectionSet::SocketConnectionSet (const SocketListener& listener)
    : itsListener (listener)
  {}

  SocketConnectionSet::~SocketConnectionSet()
  {}

  MWConnectionSet::ShPtr
  SocketConnectionSet::clone (const std::vector<int>& inx) const
  {
    int nrconn = size();
    SocketConnectionSet* set = new SocketConnectionSet(itsListener);
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

  void SocketConnectionSet::addConnections (int nr)
  {
    itsConns.reserve (itsConns.size() + nr);
    for (int i=0; i<nr; ++i) {
      itsConns.push_back (itsListener.accept());
    }
  }

  int SocketConnectionSet::size() const
  {
    return itsConns.size();
  }

  int SocketConnectionSet::getReadyConnection()
  {
    return -1;
  }

  void SocketConnectionSet::read (int seqnr, LOFAR::BlobString& buf)
  {
    itsConns[seqnr]->read (buf);
  }

  void SocketConnectionSet::write (int seqnr, const LOFAR::BlobString& buf)
  {
    itsConns[seqnr]->write (buf);
  }

  void SocketConnectionSet::writeAll (const LOFAR::BlobString& buf)
  {
    for (unsigned i=0; i<itsConns.size(); ++i) {
      itsConns[i]->write (buf);
    }
  }

}} // end namespaces
