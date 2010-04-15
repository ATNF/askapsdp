/// @file
/// @brief Base visitor class to visit an MWStep hierarchy
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
/// @author Ger van Diepen <diepen@astron.nl>


#include <mwcommon/MWStepVisitor.h>
#include <mwcommon/MWMultiStep.h>
#include <mwcommon/MWSolveStep.h>
#include <mwcommon/MWSimpleStep.h>
#include <mwcommon/MWError.h>

namespace askap { namespace mwbase {

  MWStepVisitor::~MWStepVisitor()
  {}

  void MWStepVisitor::registerVisit (const std::string& name, VisitFunc* func)
  {
    itsMap[name] = func;
  }

  void MWStepVisitor::visitMulti (const MWMultiStep& mws)
  {
    for (MWMultiStep::const_iterator it = mws.begin();
	 it != mws.end();
	 ++it) {
      (*it)->visit (*this);
    }
  }

  void MWStepVisitor::visitSolve (const MWSolveStep& step)
  {
    visit (step);
  }

  void MWStepVisitor::visitSubtract (const MWSubtractStep& step)
  {
    visitSimple (step);
  }

  void MWStepVisitor::visitCorrect (const MWCorrectStep& step)
  {
    visitSimple (step);
  }

  void MWStepVisitor::visitPredict (const MWPredictStep& step)
  {
    visitSimple (step);
  }

  void MWStepVisitor::visitSimple (const MWSimpleStep& step)
  {
    visit (step);
  }

  void MWStepVisitor::visit (const MWStep& step)
  {
    std::string name = step.className();
    std::map<std::string,VisitFunc*>::const_iterator iter = itsMap.find(name);
    if (iter == itsMap.end()) {
      visitStep (step);
    } else {
      (*iter->second)(*this, step);
    }
  }

  void MWStepVisitor::visitStep (const MWStep& step)
  {
    ASKAPTHROW (MWError,
                 "No visit function available for MWStep of type "
                 << step.className());
  }

}} // end namespaces
