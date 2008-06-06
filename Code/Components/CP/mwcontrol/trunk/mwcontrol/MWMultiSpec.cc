//#  MWMultiSpec.cc: 
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
//#
//#  $Id$

#include <mwcontrol/MWMultiSpec.h>
#include <mwcommon/MWError.h>
#include <APS/ParameterSet.h>

using namespace LOFAR;
using namespace LOFAR::ACC::APS;
using namespace std;

namespace askap { namespace cp {

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
      ASKAPTHROW (MWError, 
		   "Infinite recursion detected in definition of MWSpec \""
		   << name << "\". Please check your ParameterSet file.");
    }
    const MWMultiSpec* parent;
    if ((parent = dynamic_cast<const MWMultiSpec*>(getParent())) != 0) {
      parent->infiniteRecursionCheck(name);
    }
  }

}} // end namespaces
