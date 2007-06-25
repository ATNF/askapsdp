//#  PredifferBBS.cc: Prediffer BBSler of distributed VDS processing
//#
//#  Copyright (C) 2007
//#
//#  $Id$

#include <mwcontrol/PredifferBBS.h>
#include <BBSKernel/Prediffer.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobString.h>

using namespace LOFAR::BBS;


namespace conrad { namespace cp {

  PredifferBBS::PredifferBBS()
    : itsPrediffer (0)
  {}

  PredifferBBS::~PredifferBBS()
  {
    delete itsPrediffer;
  }

  WorkerProxy::ShPtr PredifferBBS::create()
  {
    return WorkerProxy::ShPtr (new PredifferBBS());
  }

  void PredifferBBS::setInitInfo (const std::string& measurementSet,
				  const std::string& inputColumn,
				  const std::string& skyParameterDB,
				  const std::string& instrumentParameterDB,
				  unsigned int subBand,
				  bool calcUVW)
  {
    delete itsPrediffer;
    itsPrediffer = 0;
    itsPrediffer = new Prediffer (measurementSet, inputColumn,
				  skyParameterDB, instrumentParameterDB, "",
				  subBand, calcUVW);
  }

  int PredifferBBS::doProcess (int operation, int streamId,
                               LOFAR::BlobIStream& in,
                               LOFAR::BlobOStream& out)
  {
    return operation;
  }

}} // end namespaces
