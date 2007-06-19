//# MWCorrectStep.cc: Step to process the MW solve command
//#
//# @copyright (c) 2007 CONRAD, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <mwcommon/MWCorrectStep.h>
#include <mwcommon/MWStepFactory.h>
#include <mwcommon/MWError.h>

using namespace LOFAR;

namespace conrad { namespace cp {

  MWCorrectStep::MWCorrectStep()
  {}

  MWCorrectStep::~MWCorrectStep()
  {}

  void MWCorrectStep::visit (MWStepVisitor& visitor) const
  {
    visitor.visitCorrect (*this);
  }

  MWCorrectStep* MWCorrectStep::clone() const
  {
    return new MWCorrectStep(*this);
  }

  MWStep::ShPtr MWCorrectStep::create()
  {
    return MWStep::ShPtr (new MWCorrectStep());
  }

  void MWCorrectStep::registerCreate()
  {
    MWStepFactory::push_back ("MWCorrectStep", &create);
  }

  void MWCorrectStep::toBlob (BlobOStream& bs) const
  {
    bs.putStart ("MWCorrectStep", 1);
    MWStepBBS::toBlob (bs);
    bs.putEnd();
  }

  void MWCorrectStep::fromBlob (BlobIStream& bs)
  {
    int vers = bs.getStart ("MWCorrectStep");
    CONRADASSERT (vers == 1);
    MWStepBBS::fromBlob (bs);
    bs.getEnd();
  }

}} // end namespaces
