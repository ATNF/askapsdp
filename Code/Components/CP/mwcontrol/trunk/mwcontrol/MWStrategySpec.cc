//# MWStrategySpec.cc: The specification of a strategy
//#
//# Copyright (C) 2007

#include <mwcontrol/MWStrategySpec.h>
#include <mwcontrol/MWSpec.h>
#include <mwcontrol/ParameterHandler.h>
#include <mwcommon/ConradUtil.h>
#include <ostream>

using namespace std;

namespace conrad { namespace cp {

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
    itsStep = ParameterHandler(parset).getSteps (itsName);
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
