//# NodeDesc.cc: Description of a node
//#
//# @copyright (c) 2007 CONRAD, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <mwcommon/NodeDesc.h>
#include <conrad/ConradUtil.h>
#include <ostream>

using namespace std;

namespace conrad { namespace cp {

  NodeDesc::NodeDesc (const LOFAR::ACC::APS::ParameterSet& parset)
  {
    itsName = parset.getString ("NodeName");
    itsFileSys = parset.getStringVector ("NodeFileSys");
  }

  void NodeDesc::write (std::ostream& os, const std::string& prefix) const
  {
    os << prefix << "NodeName = " << itsName << endl;
    os << prefix << "NodeFileSys = " << itsFileSys << endl;
  }

}} // end namespaces
