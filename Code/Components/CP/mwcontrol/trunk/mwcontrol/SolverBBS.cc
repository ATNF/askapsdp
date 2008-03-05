//#  SolverBBS.cc: Solver BBSler of distributed VDS processing
//#
//#  Copyright (C) 2007
//#
//#  $Id$

#include <mwcontrol/SolverBBS.h>
#include <BBSKernel/Solver.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobString.h>

using namespace LOFAR::BBS;


namespace askap { namespace cp {

  SolverBBS::SolverBBS()
    : itsSolver (0)
  {}

  SolverBBS::~SolverBBS()
  {
    delete itsSolver;
  }

  WorkerProxy::ShPtr SolverBBS::create()
  {
    return WorkerProxy::ShPtr (new SolverBBS());
  }

  void SolverBBS::setInitInfo (const std::string&,
			       const std::string&,
			       const std::string&,
			       const std::string&,
			       unsigned int,
			       bool)
  {
    delete itsSolver;
    itsSolver = 0;
    itsSolver = new Solver();
  }

  int SolverBBS::doProcess (int operation, int streamId,
                            LOFAR::BlobIStream& in,
                            LOFAR::BlobOStream& out)
  {
    return operation;
  }

}} // end namespaces
