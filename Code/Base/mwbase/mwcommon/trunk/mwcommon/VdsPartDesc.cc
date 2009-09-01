//# VdsPartDesc.cc: Description of a visibility data set or part thereof
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

#include <mwcommon/VdsPartDesc.h>
#include <askap/AskapUtil.h>
#include <ostream>

using namespace std;

namespace askap { namespace mwbase {

  VdsPartDesc::VdsPartDesc (const LOFAR::ParameterSet& parset)
  {
    itsName = parset.getString ("Name");
    itsFileSys = parset.getString ("FileSys");
    itsStartTime = parset.getDouble ("StartTime");
    itsEndTime = parset.getDouble ("EndTime");
    itsNChan = parset.getInt32Vector ("NChan");
    itsStartFreqs = parset.getDoubleVector ("StartFreqs");
    itsEndFreqs = parset.getDoubleVector ("EndFreqs");
    itsAnt1 = parset.getInt32Vector ("Ant1");
    itsAnt2 = parset.getInt32Vector ("Ant2");
  }

  void VdsPartDesc::write (std::ostream& os, const std::string& prefix) const
  {
    os << prefix << "Name = " << itsName << endl;
    os << prefix << "FileSys = " << itsFileSys << endl;
    os << prefix << "StartTime = " << itsStartTime << endl;
    os << prefix << "EndTime = " << itsEndTime << endl;
    os << prefix << "NChan = " << itsNChan << endl;
    os << prefix << "StartFreqs = " << itsStartFreqs << endl;
    os << prefix << "EndFreqs = " << itsEndFreqs << endl;
    os << prefix << "Ant1 = " << itsAnt1 << endl;
    os << prefix << "Ant2 = " << itsAnt2 << endl;
  }

  void VdsPartDesc::setName (const std::string& name,
                             const std::string& fileSys)
  {
    itsName    = name;
    itsFileSys = fileSys;
  }

  void VdsPartDesc::setTimes (double startTime, double endTime)
  {
    itsStartTime = startTime;
    itsEndTime   = endTime;
  }

  void VdsPartDesc::addBand (int nchan, double startFreq, double endFreq)
  {
    itsNChan.push_back (nchan);
    itsStartFreqs.push_back (startFreq);
    itsEndFreqs.push_back (endFreq);
  }

  void VdsPartDesc::setBaselines (const std::vector<int>& ant1,
			      const std::vector<int>& ant2)
  {
    itsAnt1 = ant1;
    itsAnt2 = ant2;
  }

}} // end namespaces
