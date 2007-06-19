//# VdsPartDesc.cc: Description of a visibility data set or part thereof
//#
//# @copyright (c) 2007 CONRAD, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <mwcommon/VdsPartDesc.h>
#include <mwcommon/ConradUtil.h>
#include <ostream>

using namespace std;

namespace conrad { namespace cp {

  VdsPartDesc::VdsPartDesc (const LOFAR::ACC::APS::ParameterSet& parset)
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
