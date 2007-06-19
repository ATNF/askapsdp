//# MWSpec2Step.cc: Convert MWSpec objects to a MWMultiStep
//#
//# Copyright (C) 2007
//#
//# $Id$

#include <mwcontrol/MWSpec2Step.h>
#include <mwcontrol/MWStrategySpec.h>
#include <mwcontrol/MWSolveSpec.h>
#include <mwcontrol/MWCorrectSpec.h>
#include <mwcontrol/MWSubtractSpec.h>
#include <mwcontrol/MWPredictSpec.h>
#include <mwcommon/WorkDomainSpec.h>
#include <mwcommon/MWSolveStep.h>
#include <mwcommon/MWCorrectStep.h>
#include <mwcommon/MWSubtractStep.h>
#include <mwcommon/MWPredictStep.h>

using namespace std;


namespace conrad { namespace cp {

  MWSpec2Step::~MWSpec2Step()
  {}

  WorkDomainSpec MWSpec2Step::convertStrategy (const MWStrategySpec& spec)
  {
    WorkDomainSpec wds;
    wds.setShape           (spec.getWorkDomainSize());
    wds.setFreqIntegration (spec.getIntegration().getFreqSize());
    wds.setTimeIntegration (spec.getIntegration().getTimeSize());
    wds.setInColumn        (spec.getInputData());
    wds.setAntennaNames    (spec.getStations());
    wds.setAutoCorr        (spec.getCorrSelection() == "AUTO");
    vector<bool> corr(4, false);
    const vector<string>& corrTypes = spec.getCorrType();
    for (unsigned i=0; i<corrTypes.size(); ++i) {
      if (corrTypes[i] == "XX") {
	corr[0] = true;
      } else if (corrTypes[i] == "XY") {
	corr[1] = true;
      } else if (corrTypes[i] == "YX") {
	corr[2] = true;
      } else if (corrTypes[i] == "YY") {
	corr[3] = true;
      }
    }
    wds.setCorr (corr);
    return wds;
  }

  void MWSpec2Step::visitSolve (const MWSolveSpec& spec)
  {
    MWSolveStep step;
    setCommon (spec, step);
    step.setParmPatterns (spec.getParms());
    step.setExclPatterns (spec.getExclParms());
    step.setDomainShape (spec.getDomainShape());
    step.setMaxIter (spec.getMaxIter());
    step.setEpsilon (spec.getEpsilon());
    step.setFraction (spec.getMinConverged());
    itsSteps.push_back (step);
  }

  void MWSpec2Step::visitCorrect (const MWCorrectSpec& spec)
  {
    MWCorrectStep step;
    setCommon (spec, step);
    itsSteps.push_back (step);
  }

  void MWSpec2Step::visitSubtract (const MWSubtractSpec& spec)
  {
    MWCorrectStep step;
    setCommon (spec, step);
    itsSteps.push_back (step);
  }

  void MWSpec2Step::visitPredict (const MWPredictSpec& spec)
  {
    MWCorrectStep step;
    setCommon (spec, step);
    itsSteps.push_back (step);
  }

  void MWSpec2Step::setCommon (const MWSingleSpec& spec, MWStepBBS& step) const
  {
    step.set (spec.getStation1(),
	      spec.getStation2(),
	      spec.getIntegration(),
	      spec.getCorrType(),
	      spec.getCorrSelection(),
	      spec.getSources(),
	      spec.getExtraSources(),
	      spec.getInstrumentModel(),
	      spec.getOutputData());
  }
  
}} // end namespaces
