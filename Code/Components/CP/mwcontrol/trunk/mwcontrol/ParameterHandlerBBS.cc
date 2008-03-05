//#  ParameterHandlerBBS.cc: 
//#
//#  Copyright (C) 2007
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
