//# MWSolveStep.cc: Step to process the MW solve command
//#
//# @copyright (c) 2007 CONRAD, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <mwcommon/MWSolveStep.h>
#include <mwcommon/MWStepFactory.h>
#include <mwcommon/MWError.h>
#include <Blob/BlobArray.h>

using namespace LOFAR;

namespace conrad { namespace cp {

  MWSolveStep::MWSolveStep()
    : itsMaxIter(10), itsEpsilon(1e-5), itsFraction(0.95)
  {}

  MWSolveStep::~MWSolveStep()
  {}

  MWSolveStep* MWSolveStep::clone() const
  {
    return new MWSolveStep(*this);
  }

  MWStep::ShPtr MWSolveStep::create()
  {
    return MWStep::ShPtr (new MWSolveStep());
  }

  void MWSolveStep::registerCreate()
  {
    MWStepFactory::push_back ("MWSolveStep", &create);
  }

  void MWSolveStep::visit (MWStepVisitor& visitor) const
  {
    visitor.visitSolve (*this);
  }

  void MWSolveStep::toBlob (BlobOStream& bs) const
  {
    bs.putStart ("MWSolveStep", 1);
    MWStepBBS::toBlob (bs);
    bs << itsParmPatterns << itsExclPatterns << itsShape;
    bs << itsMaxIter << itsEpsilon << itsFraction;
    bs.putEnd();
  }

  void MWSolveStep::fromBlob (BlobIStream& bs)
  {
    int vers = bs.getStart ("MWSolveStep");
    CONRADASSERT (vers == 1);
    MWStepBBS::fromBlob (bs);
    bs >> itsParmPatterns >> itsExclPatterns >> itsShape;
    bs >> itsMaxIter >> itsEpsilon >> itsFraction;
    bs.getEnd();
  }

}} // end namespaces
