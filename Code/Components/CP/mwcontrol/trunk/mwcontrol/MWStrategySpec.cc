//# MWStrategySpec.cc: The specification of a strategy
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

#include <mwcontrol/MWStrategySpec.h>
#include <mwcontrol/MWSpec.h>
#include <mwcontrol/ParameterHandlerBBS.h>
#include <askap/AskapUtil.h>
#include <ostream>

using namespace std;

namespace askap { namespace cp {

  MWStrategySpec::MWStrategySpec (const std::string& name,
				  const LOFAR::ACC::APS::ParameterSet& parset)
    : itsName (name)
  {
    ParameterHandler psh(parset.makeSubset(itsName + "."));
    // If defined, get the strategy selection.
    psh.fillStringVector("Stations", itsStations);
    // If defined, get the correlation selection (ALL, AUTO, or CROSS), and
    // type (e.g., ["XX", "XY", "YX", "YY"]
    psh.fillString("Correlation.Selection", itsCorrSelection);
    psh.fillStringVector("Correlation.Type", itsCorrType);
    // If defined, get the integration intervals in frequency (Hz) and
    // time (s).
    double deltaFreq = -1;
    double deltaTime = -1;
    psh.fillDouble("Integration.Freq", deltaFreq);
    psh.fillDouble("Integration.Time", deltaTime);
    itsIntegration = DomainShape(deltaFreq, deltaTime);
    // Get the input data column; defaults to DATA.
    itsInputData = psh.getString("InputData", "DATA");
    // Get the work domain shape which must be defined.
    double freqSize = psh.getDouble("WorkDomainSize.Freq");
    double timeSize = psh.getDouble("WorkDomainSize.Time");
    itsWorkDomainSize = DomainShape(freqSize, timeSize);
    itsStep = ParameterHandlerBBS(parset).getSteps (itsName);
  }

  void MWStrategySpec::print (std::ostream& os) const
  {
    os << "Strategy specification: " << itsName << endl
       << " Stations:          " << itsStations << endl
       << " Corr selection:    " << itsCorrSelection << endl
       << " Correlation type:  " << itsCorrType << endl
       << " Work domain size:  " << itsWorkDomainSize << endl
       << " Integration:       " << itsIntegration << endl
       << " Input data column: " << itsInputData << endl;
    itsStep.print (os, "  ");
  }

  std::ostream& operator<< (std::ostream& os, const MWStrategySpec& spec)
  {
    spec.print (os);
    return os;
  }

}} // end namespaces
