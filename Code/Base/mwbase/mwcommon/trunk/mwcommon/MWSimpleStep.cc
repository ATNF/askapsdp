//# MWSubtractStep.cc: Step to process the MW solve command
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
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <mwcommon/MWSimpleStep.h>

namespace askap { namespace mwbase {

  MWSimpleStep::~MWSimpleStep()
  {}

  void MWSimpleStep::visit (MWStepVisitor& visitor) const
  {
    visitor.visitSimple (*this);
  }


  MWSubtractStep::~MWSubtractStep()
  {}

  void MWSubtractStep::visit (MWStepVisitor& visitor) const
  {
    visitor.visitSubtract (*this);
  }


  MWCorrectStep::~MWCorrectStep()
  {}

  void MWCorrectStep::visit (MWStepVisitor& visitor) const
  {
    visitor.visitCorrect (*this);
  }


  MWPredictStep::~MWPredictStep()
  {}

  void MWPredictStep::visit (MWStepVisitor& visitor) const
  {
    visitor.visitPredict (*this);
  }

}} // end namespaces
