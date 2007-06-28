//# MWCorrectStepBBS.cc
//#
//# @copyright (c) 2007 CONRAD, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <mwcontrol/MWCorrectStepBBS.h>
#include <mwcommon/MWStepFactory.h>
#include <mwcommon/MWError.h>

using namespace LOFAR;

namespace conrad { namespace cp {

  MWCorrectStepBBS::MWCorrectStepBBS()
  {}

  MWCorrectStepBBS::~MWCorrectStepBBS()
  {}

  MWCorrectStepBBS* MWCorrectStepBBS::clone() const
  {
    return new MWCorrectStepBBS(*this);
  }

  MWStep::ShPtr MWCorrectStepBBS::create()
  {
    return MWStep::ShPtr (new MWCorrectStepBBS());
  }

  void MWCorrectStepBBS::registerCreate()
  {
    MWStepFactory::push_back ("MWCorrectStepBBS", &create);
  }

  std::string MWCorrectStepBBS::className() const
  {
    static std::string name("MWCorrectStepBBS");
    return name;
  }

  void MWCorrectStepBBS::visit (MWStepVisitor& visitor) const
  {
    visitor.visitCorrect (*this);
  }

  void MWCorrectStepBBS::toBlob (BlobOStream& bs) const
  {
    bs.putStart (className(), 1);
    itsProp.toBlob (bs);
    bs.putEnd();
  }

  void MWCorrectStepBBS::fromBlob (BlobIStream& bs)
  {
    int vers = bs.getStart (className());
    CONRADASSERT (vers == 1);
    itsProp.fromBlob (bs);
    bs.getEnd();
  }

}} // end namespaces
