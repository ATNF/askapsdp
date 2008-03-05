/// @file
/// @brief Handle the BBS part of a LOFAR .parset file
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Ger van Diepen (diepen AT astron nl)
///
//# $Id$

#ifndef ASKAP_MWCONTROL_PARAMETERHANDLERBBS_H
#define ASKAP_MWCONTROL_PARAMETERHANDLERBBS_H

#include <mwcommon/ParameterHandler.h>

namespace askap { namespace cp {

  /// Forward Declarations
  class MWMultiSpec;
  class MWStrategySpec;

  /// @ingroup mwcontrol
  /// @brief Handle the BBS part of a LOFAR .parset file

  /// This class handles the processing of a LOFAR .parset file
  /// It has functions to retrieve specific info from the .parset file.
  /// These can be initial info, strategy specifications (MWStrategySpec),
  /// and step specifications (MWStep).

  class ParameterHandlerBBS: public ParameterHandler
  {
  public:
    explicit ParameterHandlerBBS (const LOFAR::ACC::APS::ParameterSet&);

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
  };

}} /// end namespaces

#endif
