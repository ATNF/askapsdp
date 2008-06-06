/// @file
/// @brief Handle the BBS part of a LOFAR .parset file
///
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
