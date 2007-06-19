//# MWPredictStep.cc: Step to process the MW solve command
//#
//# @copyright (c) 2007 CONRAD, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <mwcommon/MWPredictStep.h>
#include <mwcommon/MWStepFactory.h>
#include <mwcommon/MWError.h>

using namespace LOFAR;

namespace conrad { namespace cp {

  MWPredictStep::MWPredictStep()
  {}

  MWPredictStep::~MWPredictStep()
  {}

  MWPredictStep* MWPredictStep::clone() const
  {
    return new MWPredictStep(*this);
  }

  MWStep::ShPtr MWPredictStep::create()
  {
    return MWStep::ShPtr (new MWPredictStep());
  }

  void MWPredictStep::registerCreate()
  {
    MWStepFactory::push_back ("MWPredictStep", &create);
  }

  void MWPredictStep::visit (MWStepVisitor& visitor) const
  {
    visitor.visitPredict (*this);
  }

  void MWPredictStep::toBlob (BlobOStream& bs) const
  {
    bs.putStart ("MWPredictStep", 1);
    MWStepBBS::toBlob (bs);
    bs.putEnd();
  }

  void MWPredictStep::fromBlob (BlobIStream& bs)
  {
    int vers = bs.getStart ("MWPredictStep");
    CONRADASSERT (vers == 1);
    MWStepBBS::fromBlob (bs);
    bs.getEnd();
  }

}} // end namespaces
