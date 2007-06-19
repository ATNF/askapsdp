//#  ParameterHandler.h: DataHolder for sending/receiving mwcontrol info
//#
//#  Copyright (C) 2007
//#
//#  $Id$

#ifndef CONRAD_MWCONTROL_PARAMETERHANDLER_H
#define CONRAD_MWCONTROL_PARAMETERHANDLER_H

#include <APS/ParameterSet.h>

namespace conrad { namespace cp {

  /// Forward Declarations
  class MWMultiSpec;
  class MWStrategySpec;


  class ParameterHandler
  {
  public:
    explicit ParameterHandler (const LOFAR::ACC::APS::ParameterSet&);

    //// Get the number of VDS parts.
    int getNParts() const;

    //// Get the initial parameters.
    void getInitInfo (std::string& msName,
		      std::string& inputColumn,
		      std::string& skyParameterDB,
		      std::string& instrumentParameterDB,
		      unsigned& subBand,
		      bool& calcUVW) const;

    //// Get all strategies specifications from the parameters.
    std::vector<MWStrategySpec> getStrategies() const;

    //// Get all step specifications of a strategy from the parameters.
    MWMultiSpec getSteps (const std::string& strategyName) const;

    //// Get a parameter value.
    //// An exception is thrown if it does not exist.
    //// @{
    std::string getString (const std::string& parm) const;
    double getDouble (const std::string& parm) const;
    unsigned getUint (const std::string& parm) const;
    bool getBool (const std::string& parm) const;
    std::vector<std::string> getStringVector (const std::string& parm) const;
    //// @}

    //// Get a parameter value.
    //// If it does not exist, the default value is used instead.
    //// @{
    std::string getString (const std::string& parm,
			   const std::string& defVal) const;
    double getDouble (const std::string& parm,
		      double defVal) const;
    unsigned getUint (const std::string& parm,
		      unsigned defVal) const;
    bool getBool (const std::string& parm,
		  bool defVal) const;
    std::vector<std::string> getStringVector
      (const std::string& parm, const std::vector<std::string>& defVal) const;
    //// @}

    //// Get a parameter value and fill \a value with it.
    //// If it does not exist, nothing is done.
    //// @{
    void fillString (const std::string& parm,
		     std::string& value) const;
    void fillDouble (const std::string& parm,
		     double& value) const;
    void fillUint (const std::string& parm,
		   unsigned& value) const;
    void fillBool (const std::string& parm,
		   bool& value) const;
    void fillStringVector (const std::string& parm,
			   std::vector<std::string>& value) const;
    //// @}

  private:
    LOFAR::ACC::APS::ParameterSet itsParms;
  };


  inline std::string ParameterHandler::getString (const std::string& parm) const
    { return itsParms.getString (parm); }
  inline double ParameterHandler::getDouble (const std::string& parm) const
    { return itsParms.getDouble (parm); }
  inline unsigned ParameterHandler::getUint (const std::string& parm) const
    { return itsParms.getUint32 (parm); }
  inline bool ParameterHandler::getBool (const std::string& parm) const
    { return itsParms.getBool (parm); }
  inline std::vector<std::string> ParameterHandler::getStringVector
  (const std::string& parm) const
    { return itsParms.getStringVector (parm); }

}} /// end namespaces

#endif
