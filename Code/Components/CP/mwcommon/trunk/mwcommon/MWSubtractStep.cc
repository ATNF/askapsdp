//# MWSubtractStep.cc: Step to process the MW solve command
//#
//# @copyright (c) 2007 CONRAD, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <mwcommon/MWSubtractStep.h>
#include <mwcommon/MWStepFactory.h>
#include <mwcommon/MWError.h>

using namespace LOFAR;

namespace conrad { namespace cp {

  MWSubtractStep::MWSubtractStep()
  {}

  MWSubtractStep::~MWSubtractStep()
  {}

  MWSubtractStep* MWSubtractStep::clone() const
  {
    return new MWSubtractStep(*this);
  }

  MWStep::ShPtr MWSubtractStep::create()
  {
    return MWStep::ShPtr (new MWSubtractStep());
  }

  void MWSubtractStep::registerCreate()
  {
    MWStepFactory::push_back ("MWSubtractStep", &create);
  }

  void MWSubtractStep::visit (MWStepVisitor& visitor) const
  {
    visitor.visitSubtract (*this);
  }

  void MWSubtractStep::toBlob (BlobOStream& bs) const
  {
    bs.putStart ("MWSubtractStep", 1);
    MWStepBBS::toBlob (bs);
    bs.putEnd();
  }

  void MWSubtractStep::fromBlob (BlobIStream& bs)
  {
    int vers = bs.getStart ("MWSubtractStep");
    CONRADASSERT (vers == 1);
    MWStepBBS::fromBlob (bs);
    bs.getEnd();
  }

}} // end namespaces
