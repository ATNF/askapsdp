//# MWMultiStep.cc: A step consisting of several other steps.
//#
//# @copyright (c) 2007 CONRAD, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <mwcommon/MWMultiStep.h>
#include <mwcommon/MWStepFactory.h>
#include <mwcommon/MWError.h>

using namespace LOFAR;

namespace conrad { namespace cp {

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
    CONRADASSERT (vers == 1);
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
