//#  MWSpec.cc: 
//#
//#  Copyright (C) 2007
//#
//#  $Id$

#include <mwcontrol/MWSpec.h>
#include <mwcontrol/MWCorrectSpec.h>
#include <mwcontrol/MWPredictSpec.h>
////#include <mwcontrol/MWRefitSpec.h>
////#include <mwcontrol/MWShiftSpec.h>
#include <mwcontrol/MWSolveSpec.h>
#include <mwcontrol/MWSubtractSpec.h>
#include <mwcontrol/MWMultiSpec.h>
#include <mwcommon/ParameterHandler.h>
#include <mwcommon/MWError.h>
#include <mwcommon/ConradUtil.h>
#include <APS/ParameterSet.h>
#include <ostream>

using namespace LOFAR::ACC::APS;
using namespace std;

namespace conrad { namespace cp {


  MWSpec::~MWSpec()
  {}

  string MWSpec::fullName() const
  {
    string name;
    if (itsParent) {
      name = itsParent->fullName() + ".";
    }
    name += itsName;
    return name;
  }

  MWSpec::ShPtr MWSpec::create (const string& name,
				const ParameterSet& parset,
				const MWSpec* parent)
  {
    MWSpec* spec;
    // If \a parset contains a key <tt>Step.<em>name</em>.Steps</tt>, then
    // \a name is a MWMultiSpec, otherwise it is a SingleSpec.
    if (parset.isDefined("Step." + name + ".Steps")) {
      spec = new MWMultiSpec(name, parset, parent);
    } else {
      // We'll have to figure out what kind of SingleSpec we must
      // create. The key "Operation" contains this information.
      string oper = toUpper(parset.getString("Step." + name + ".Operation"));
      if (oper == "SOLVE") {
	spec = new MWSolveSpec(name, parset, parent);
      } else if (oper == "SUBTRACT") {
	spec = new MWSubtractSpec(name, parset, parent);
      } else if (oper == "CORRECT") {
	spec = new MWCorrectSpec(name, parset, parent);
      } else if (oper == "PREDICT") {
	spec = new MWPredictSpec(name, parset, parent);
      ////      } else if (oper == "SHIFT") {
      ////	spec = new MWShiftSpec(name, parset, parent);
      ////      } else if (oper == "REFIT") {
      ////	spec = new MWRefitSpec(name, parset, parent);
      } else {
	CONRADTHROW (MWError, "Operation \"" << oper << 
		     "\" is not a valid Step operation");
      }
    }
    return MWSpec::ShPtr (spec);
  }

  MWSpec::MWSpec()
    : itsParent (0)
  {}

  MWSpec::MWSpec (const string& name, 
		  const ParameterSet& parset,
		  const MWSpec* parent)
  {
    // Copy the data members from the parent, if present, so that they have
    // sensible default values.
    if (parent) *this = *parent;
    // We must reset these values, because they were overwritten by the
    // previous data copy.
    itsName = name;
    itsParent = parent;
    // Overrride default values for data members of the current MWSpec, if
    // they're specified in \a parset.
    setParms(parset.makeSubset("Step." + name + "."));
  }

  void MWSpec::printSpec (ostream& os, const string& indent,
			  const string& type) const
  {
    os << indent << type << " specification: " << itsName;
    string indent2 = indent + " ";
    os << endl << indent2 << "Full name: " << fullName()
       << endl << indent2 << "Station1:         " << itsStation1
       << endl << indent2 << "Station2:         " << itsStation2
       << endl << indent2 << "Corr selection:   " << itsCorrSelection
       << endl << indent2 << "Correlation type: " << itsCorrType
       << endl << indent2 << "Integration:      " << itsIntegration
       << endl << indent2 << "Sources:          " << itsSources
       << endl << indent2 << "Extra sources:    " << itsExtraSources
       << endl << indent2 << "Instrument model: " << itsInstrumentModel;
  }

  void MWSpec::setParms (const ParameterSet& ps)
  {
    ParameterHandler psh(ps);
    // If defined, get the baseline selection for this step.
    psh.fillStringVector("Baselines.Station1", itsStation1);
    psh.fillStringVector("Baselines.Station2", itsStation2);
    // If defined, get the correlation selection (ALL, AUTO, or CROSS), and
    // type (e.g., ["XX", "XY", "YX", "YY"]
    psh.fillString("Correlation.Selection", itsCorrSelection);
    psh.fillStringVector("Correlation.Type", itsCorrType);
    // If defined, get the integration intervals in frequency (Hz) and
    // time (s).
    double deltaFreq = itsIntegration.getFreqSize();
    double deltaTime = itsIntegration.getTimeSize();
    psh.fillDouble("Integration.Freq", deltaFreq);
    psh.fillDouble("Integration.Time", deltaTime);
    itsIntegration = DomainShape(deltaFreq, deltaTime);
    // If defined, get the sources for the current patch.
    psh.fillStringVector("Sources", itsSources);
    // If defined, get the extra source, outside the current patch.
    psh.fillStringVector("ExtraSources", itsExtraSources);
    // If defined, get the instrument model part(s) used.
    psh.fillStringVector("InstrumentModel", itsInstrumentModel);
  }

  ostream& operator<< (ostream& os, const MWSpec& bs)
  {
    bs.print(os, string());
    return os;
  }

}} // end namespaces
