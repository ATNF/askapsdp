//# MWMultiStep.cc: A step consisting of several other steps.
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

#include <mwcommon/MWMultiStep.h>
#include <mwcommon/MWStepFactory.h>
#include <mwcommon/MWError.h>

using namespace LOFAR;

namespace askap { namespace mwbase {

  MWMultiStep::~MWMultiStep()
  {}

  MWMultiStep* MWMultiStep::clone() const
  {
    return new MWMultiStep(*this);
  }

  void MWMultiStep::push_back (const MWStep& step)
  {
    itsSteps.push_back (MWStep::ShPtr(step.clone()));
  }

  MWStep::ShPtr MWMultiStep::create()
  {
    return MWStep::ShPtr (new MWMultiStep());
  }

  void MWMultiStep::registerCreate()
  {
    MWStepFactory::push_back ("MWMultiStep", &create);
  }

  std::string MWMultiStep::className() const
  {
    static std::string name("MWMultiStep");
    return name;
  }

  void MWMultiStep::visit (MWStepVisitor& visitor) const
  {
    visitor.visitMulti (*this);
  }

  void MWMultiStep::toBlob (BlobOStream& bs) const
  {
    bs.putStart ("MWMultiStep", 1);
    bs << static_cast<uint32>(itsSteps.size());
    for (std::list<MWStep::ShPtr>::const_iterator iter=itsSteps.begin();
 	 iter!=itsSteps.end();
 	 ++iter) {
      (*iter)->toBlob (bs);
    }
    bs.putEnd();
  }

  void MWMultiStep::fromBlob (BlobIStream& bs)
  {
    int vers = bs.getStart ("MWMultiStep");
    ASKAPASSERT (vers == 1);
    uint32 nr;
    bs >> nr;
    for (uint32 i=0; i<nr; ++i) {
      MWStep::ShPtr step = MWStepFactory::create (bs.getNextType());
      step->fromBlob (bs);
      itsSteps.push_back (step);
    }
    bs.getEnd();
  }

}} // end namespaces
