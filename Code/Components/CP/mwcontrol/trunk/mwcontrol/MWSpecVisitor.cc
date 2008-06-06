//# MWSpecVisitor.cc: Base visitor class to visit an MWSpec hierarchy
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
//# $Id$

#include <mwcontrol/MWSpecVisitor.h>
#include <mwcontrol/MWMultiSpec.h>
#include <mwcommon/MWError.h>

namespace askap { namespace cp {

  MWSpecVisitor::~MWSpecVisitor()
  {}

  void MWSpecVisitor::visitMulti (const MWMultiSpec& mws)
  {
    for (MWMultiSpec::const_iterator it = mws.begin();
	 it != mws.end();
	 ++it) {
      (*it)->visit (*this);
    }
  }

  void MWSpecVisitor::visitSolve (const MWSolveSpec&)
  {
    ASKAPTHROW (MWError,
	       "visitSolve not implemented in derived MWSpecVisitor class");
  }

  void MWSpecVisitor::visitSubtract (const MWSubtractSpec&)
  {
    ASKAPTHROW (MWError,
	       "visitSubtract not implemented in derived MWSpecVisitor class");
  }

  void MWSpecVisitor::visitCorrect (const MWCorrectSpec&)
  {
    ASKAPTHROW (MWError,
	       "visitCorrect not implemented in derived MWSpecVisitor class");
  }

  void MWSpecVisitor::visitPredict (const MWPredictSpec&)
  {
    ASKAPTHROW (MWError,
	       "visitPredict not implemented in derived MWSpecVisitor class");
  }

}} // end namespaces
