//#  MWSolveSpec.cc: 
//#
//#  Copyright (C) 2007
//#
//#  $Id$

#include <mwcontrol/MWSolveSpec.h>
#include <mwcommon/MWError.h>
#include <mwcommon/ConradUtil.h>
#include <APS/ParameterSet.h>

using namespace LOFAR::ACC::APS;
using namespace std;

namespace conrad { namespace cp {

  MWSolveSpec::MWSolveSpec(const string& name, 
			   const ParameterSet& parset,
			   const MWSpec* parent)
    : MWSingleSpec(name, parset, parent)
  {
    // Create a subset of \a parset, containing only the relevant keys for
    // the current MWSingleSpec.
    ParameterSet ps(parset.makeSubset("Step." + name + ".Solve."));

    // Get the relevant parameters from the Parameter Set \a parset. 
    // \note A missing key is considered an error.
    itsMaxIter          = ps.getUint32("MaxIter");
    itsEpsilon          = ps.getDouble("Epsilon");
    itsMinConverged     = ps.getDouble("MinConverged");
    itsParms            = ps.getStringVector("Parms");
    itsExclParms        = ps.getStringVector("ExclParms");
    double bandWidth    = ps.getDouble("DomainSize.Freq");
    double timeInterval = ps.getDouble("DomainSize.Time");
    itsDomainSize       = DomainShape (bandWidth, timeInterval);
  }

  MWSolveSpec::~MWSolveSpec()
  {}

  void MWSolveSpec::visit (MWSpecVisitor& visitor) const
  {
    visitor.visitSolve (*this);
  }

  void MWSolveSpec::print(ostream& os, const string& indent) const
  {
    MWSpec::printSpec (os, indent, "Solve");
    string indent2 = indent + " ";
    os << endl << indent2 << "Solve: ";
    os << endl << indent2 << " Max nr. of iterations:  " << itsMaxIter
       << endl << indent2 << " Convergence threshold:  " << itsEpsilon
       << endl << indent2 << " Min fraction converged: " << itsMinConverged
       << endl << indent2 << " Solvable parameters:    " << itsParms
       << endl << indent2 << " Excluded parameters:    " << itsExclParms
       << endl << indent2 << " Domain size :           " << itsDomainSize;
  }

}} // end namespaces
