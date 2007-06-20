//#  MWMultiSpec.cc: 
//#
//#  Copyright (C) 2007
//#
//#  $Id$

#include <mwcontrol/MWMultiSpec.h>
#include <mwcommon/MWError.h>
#include <APS/ParameterSet.h>

using namespace LOFAR;
using namespace LOFAR::ACC::APS;
using namespace std;

namespace conrad { namespace cp {

  MWMultiSpec::MWMultiSpec()
    : MWSpec()
  {}

  MWMultiSpec::MWMultiSpec(const string& name,
			   const ParameterSet& parset,
			   const MWSpec* parent)
    : MWSpec(name, parset, parent)
  {
    // This multispec consists of the following specs.
    vector<string> specs(parset.getStringVector("Step." + name + ".Steps"));
    // Create a new spec for each name in \a specs.
    for (uint i=0; i<specs.size(); ++i) {
      infiniteRecursionCheck (specs[i]);
      itsSpecs.push_back (MWSpec::create(specs[i], parset, this));
    }
  }


  MWMultiSpec::~MWMultiSpec()
  {}

  void MWMultiSpec::visit (MWSpecVisitor& visitor) const
  {
    visitor.visitMulti (*this);
  }

  void MWMultiSpec::print(ostream& os, const string& indent) const
  {
    printSpec (os, indent, "Multi");
    string indent2 = indent + ". ";
    for (const_iterator it=begin(); it!=end(); ++it) {
      os << endl;
      (*it)->print (os, indent2);
    }
  }

  void MWMultiSpec::infiniteRecursionCheck(const string& name) const
  {
    if (name == getName()) {
      CONRADTHROW (MWError, 
		   "Infinite recursion detected in definition of MWSpec \""
		   << name << "\". Please check your ParameterSet file.");
    }
    const MWMultiSpec* parent;
    if ((parent = dynamic_cast<const MWMultiSpec*>(getParent())) != 0) {
      parent->infiniteRecursionCheck(name);
    }
  }

}} // end namespaces
