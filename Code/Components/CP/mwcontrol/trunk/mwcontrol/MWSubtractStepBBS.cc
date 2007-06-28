//# MWSubtractStepBBS.cc
//#
//# @copyright (c) 2007 CONRAD, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <mwcontrol/MWSubtractStepBBS.h>
#include <mwcommon/MWStepFactory.h>
#include <mwcommon/MWError.h>

using namespace LOFAR;

namespace conrad { namespace cp {

  MWSubtractStepBBS::MWSubtractStepBBS()
  {}

  MWSubtractStepBBS::~MWSubtractStepBBS()
  {}

  MWSubtractStepBBS* MWSubtractStepBBS::clone() const
  {
    return new MWSubtractStepBBS(*this);
  }

  MWStep::ShPtr MWSubtractStepBBS::create()
  {
    return MWStep::ShPtr (new MWSubtractStepBBS());
  }

  void MWSubtractStepBBS::registerCreate()
  {
    MWStepFactory::push_back ("MWSubtractStepBBS", &create);
  }

  std::string MWSubtractStepBBS::className() const
  {
    static std::string name("MWSubtractStepBBS");
    return name;
  }

  void MWSubtractStepBBS::visit (MWStepVisitor& visitor) const
  {
    visitor.visitSubtract (*this);
  }

  void MWSubtractStepBBS::toBlob (BlobOStream& bs) const
  {
    bs.putStart (className(), 1);
    itsProp.toBlob (bs);
    bs.putEnd();
  }

  void MWSubtractStepBBS::fromBlob (BlobIStream& bs)
  {
    int vers = bs.getStart (className());
    CONRADASSERT (vers == 1);
    itsProp.fromBlob (bs);
    bs.getEnd();
  }

}} // end namespaces
