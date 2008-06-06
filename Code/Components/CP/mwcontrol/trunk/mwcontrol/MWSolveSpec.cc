//#  MWSolveSpec.cc: 
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
//#
//#  $Id$

#include <mwcontrol/MWSolveSpec.h>
#include <mwcommon/MWError.h>
#include <askap/AskapUtil.h>
#include <APS/ParameterSet.h>

using namespace LOFAR::ACC::APS;
using namespace std;

namespace askap { namespace cp {

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
