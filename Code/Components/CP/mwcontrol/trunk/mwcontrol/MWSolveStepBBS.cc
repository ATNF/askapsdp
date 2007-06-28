//# MWSolveStepBBS.cc: StepBBS to process the MW solve command
//#
//# @copyright (c) 2007 CONRAD, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <mwcontrol/MWSolveStepBBS.h>
#include <mwcommon/MWStepFactory.h>
#include <mwcommon/MWError.h>
#include <Blob/BlobArray.h>

using namespace LOFAR;

namespace conrad { namespace cp {

  MWSolveStepBBS::MWSolveStepBBS()
    : itsMaxIter(10), itsEpsilon(1e-5), itsFraction(0.95)
  {}

  MWSolveStepBBS::~MWSolveStepBBS()
  {}

  MWSolveStepBBS* MWSolveStepBBS::clone() const
  {
    return new MWSolveStepBBS(*this);
  }

  MWStep::ShPtr MWSolveStepBBS::create()
  {
    return MWStep::ShPtr (new MWSolveStepBBS());
  }

  void MWSolveStepBBS::registerCreate()
  {
    MWStepFactory::push_back ("MWSolveStepBBS", &create);
  }

  std::string MWSolveStepBBS::className() const
  {
    static std::string name("MWSolveStepBBS");
    return name;
  }

  void MWSolveStepBBS::visit (MWStepVisitor& visitor) const
  {
    visitor.visitSolve (*this);
  }

  void MWSolveStepBBS::toBlob (BlobOStream& bs) const
  {
    bs.putStart (className(), 1);
    itsProp.toBlob (bs);
    bs << itsParmPatterns << itsExclPatterns << itsShape;
    bs << itsMaxIter << itsEpsilon << itsFraction;
    bs.putEnd();
  }

  void MWSolveStepBBS::fromBlob (BlobIStream& bs)
  {
    int vers = bs.getStart (className());
    CONRADASSERT (vers == 1);
    itsProp.fromBlob (bs);
    bs >> itsParmPatterns >> itsExclPatterns >> itsShape;
    bs >> itsMaxIter >> itsEpsilon >> itsFraction;
    bs.getEnd();
  }

}} // end namespaces
