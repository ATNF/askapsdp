//# MWPredictStepBBS.cc
//#
//# @copyright (c) 2007 ASKAP, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <mwcontrol/MWPredictStepBBS.h>
#include <mwcommon/MWStepFactory.h>
#include <mwcommon/MWError.h>

using namespace LOFAR;

namespace askap { namespace cp {

  MWPredictStepBBS::MWPredictStepBBS()
  {}

  MWPredictStepBBS::~MWPredictStepBBS()
  {}

  MWPredictStepBBS* MWPredictStepBBS::clone() const
  {
    return new MWPredictStepBBS(*this);
  }

  MWStep::ShPtr MWPredictStepBBS::create()
  {
    return MWStep::ShPtr (new MWPredictStepBBS());
  }

  void MWPredictStepBBS::registerCreate()
  {
    MWStepFactory::push_back ("MWPredictStepBBS", &create);
  }

  std::string MWPredictStepBBS::className() const
  {
    static std::string name("MWPredictStepBBS");
    return name;
  }

  void MWPredictStepBBS::visit (MWStepVisitor& visitor) const
  {
    visitor.visitPredict (*this);
  }

  void MWPredictStepBBS::toBlob (BlobOStream& bs) const
  {
    bs.putStart (className(), 1);
    itsProp.toBlob (bs);
    bs.putEnd();
  }

  void MWPredictStepBBS::fromBlob (BlobIStream& bs)
  {
    int vers = bs.getStart (className());
    ASKAPASSERT (vers == 1);
    itsProp.fromBlob (bs);
    bs.getEnd();
  }

}} // end namespaces
