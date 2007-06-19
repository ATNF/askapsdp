//#  ParameterHandler.cc: 
//#
//#  Copyright (C) 2007
//#
//#  $Id$

#include <mwcontrol/ParameterHandler.h>
#include <mwcontrol/MWStrategySpec.h>
#include <mwcontrol/MWMultiSpec.h>

using namespace LOFAR::ACC::APS;
using namespace std;

namespace conrad { namespace cp {

  ParameterHandler::ParameterHandler (const ParameterSet& parSet)
    : itsParms (parSet)
  {}

  void ParameterHandler::getInitInfo (string& msName,
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

  int ParameterHandler::getNParts() const
  {
    return getUint ("NNode", 1);
  }

  vector<MWStrategySpec> ParameterHandler::getStrategies() const
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

  MWMultiSpec ParameterHandler::getSteps (const string& name) const
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

  string ParameterHandler::getString (const string& parm,
				      const string& defVal) const
  {
    if (itsParms.isDefined(parm)) {
      return itsParms.getString (parm);
    }
    return defVal;
  }

  double ParameterHandler::getDouble (const string& parm,
				      double defVal) const
  {
    if (itsParms.isDefined(parm)) {
      return itsParms.getDouble (parm);
    }
    return defVal;
  }

  unsigned ParameterHandler::getUint (const string& parm,
				      unsigned defVal) const
  {
    if (itsParms.isDefined(parm)) {
      return itsParms.getUint32 (parm);
    }
    return defVal;
  }

  bool ParameterHandler::getBool (const string& parm,
				  bool defVal) const
  {
    if (itsParms.isDefined(parm)) {
      return itsParms.getBool (parm);
    }
    return defVal;
  }

  vector<string> ParameterHandler::getStringVector
  (const string& parm, const vector<string>& defVal) const
  {
    if (itsParms.isDefined(parm)) {
      return itsParms.getStringVector (parm);
    }
    return defVal;
  }

  void ParameterHandler::fillString (const string& parm,
				     string& value) const
  {
    if (itsParms.isDefined(parm)) {
      value = itsParms.getString (parm);
    }
  }

  void ParameterHandler::fillDouble (const string& parm,
				     double& value) const
  {
    if (itsParms.isDefined(parm)) {
      value = itsParms.getDouble (parm);
    }
  }

  void ParameterHandler::fillUint (const string& parm,
				   unsigned& value) const
  {
    if (itsParms.isDefined(parm)) {
      value = itsParms.getUint32 (parm);
    }
  }

  void ParameterHandler::fillBool (const string& parm,
				   bool& value) const
  {
    if (itsParms.isDefined(parm)) {
      value = itsParms.getBool (parm);
    }
  }

  void ParameterHandler::fillStringVector (const string& parm,
					   vector<string>& value) const
  {
    if (itsParms.isDefined(parm)) {
      value = itsParms.getStringVector (parm);
    }
  }

}} // end namespaces
