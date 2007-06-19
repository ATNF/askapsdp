//#  MWSingleSpec.cc: 
//#
//#  Copyright (C) 2007
//#
//#  $Id$

#include <mwcontrol/MWSingleSpec.h>
#include <APS/ParameterSet.h>

using namespace LOFAR;
using namespace LOFAR::ACC::APS;
using namespace std;

namespace conrad { namespace cp {

  MWSingleSpec::~MWSingleSpec()
  {}

  void MWSingleSpec::printSpec (ostream& os, const string& indent,
				const string& type) const
  {
    MWSpec::printSpec (os, indent, type);
    os << endl << indent << " Output data: " << itsOutputData;
  }

  MWSingleSpec::MWSingleSpec(const MWSpec* parent) :
    MWSpec(parent)
  {}

  MWSingleSpec::MWSingleSpec(const string& name, 
			     const ParameterSet& parset,
			     const MWSpec* parent) :
    MWSpec(name, parset, parent)
  {
    // Create a subset of \a parset, containing only the relevant keys for
    // the current MWSingleSpec.
    ParameterSet ps(parset.makeSubset("Step." + name + "."));
    // Get the name of the data column to write to
    itsOutputData = ps.getString("OutputData");
  }

}} // end namespaces
