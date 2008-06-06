//#  ParameterHandlerBBS.cc: 
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

#include <mwcontrol/ParameterHandlerBBS.h>
#include <mwcontrol/MWStrategySpec.h>
#include <mwcontrol/MWMultiSpec.h>

using namespace LOFAR::ACC::APS;
using namespace std;

namespace askap { namespace cp {

  ParameterHandlerBBS::ParameterHandlerBBS (const ParameterSet& parSet)
    : ParameterHandler (parSet)
  {}

  void ParameterHandlerBBS::getInitInfo (string& msName,
                                         string& inputColumn,
                                         string& skyParameterDB,
                                         string& instrumentParameterDB,
                                         unsigned& subBand,
                                         bool& calcUVW) const
  {
    msName                = getString ("DataSet");
    inputColumn           = getString ("Strategy.InputData", "DATA");
    skyParameterDB        = getString ("ParmDB.LocalSky");
    instrumentParameterDB = getString ("ParmDB.Instrument");
    subBand               = getUint   ("SubBandID", 0);
    calcUVW               = getBool   ("CalcUVW", false);
  }

  int ParameterHandlerBBS::getNParts() const
  {
    return getUint ("NNode", 1);
  }

  vector<MWStrategySpec> ParameterHandlerBBS::getStrategies() const
  {
    // Get all strategy names.
    // Default is 'Strategy'.
    vector<string> defName(1, "Strategy");
    vector<string> strategyNames(getStringVector("Strategies", defName));
    // Create a new strategy specification object for each name.
    vector<MWStrategySpec> specs;
    for (unsigned i=0; i<strategyNames.size(); ++i) {
      specs.push_back (MWStrategySpec (strategyNames[i], itsParms));
    }
    return specs;
  }

  MWMultiSpec ParameterHandlerBBS::getSteps (const string& name) const
  {
    // Get all step names.
    vector<string> stepNames(itsParms.getStringVector(name+".Steps"));
    // Create a new step specification object for each name.
    MWMultiSpec specs;
    for (unsigned i=0; i<stepNames.size(); ++i) {
      specs.push_back (MWSpec::create (stepNames[i], itsParms, 0));
    }
    return specs;
  }

}} // end namespaces
