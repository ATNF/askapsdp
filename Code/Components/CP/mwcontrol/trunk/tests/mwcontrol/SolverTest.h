//#  SolverTest.h: 
//#
//#  Copyright (C) 2007
//#
//#  $Id$

#ifndef CONRAD_MWCONTROL_SOLVERTEST_H
#define CONRAD_MWCONTROL_SOLVERTEST_H

#include <mwcontrol/SolverProxy.h>


namespace conrad { namespace cp {

  class SolverTest: public SolverProxy
  {
  public:
    SolverTest();

    ~SolverTest();

    // Create a new object.
    static WorkerProxy::ShPtr create();

    virtual void setInitInfo (const std::string& measurementSet,
			      const std::string& inputColumn,
			      const std::string& skyParameterDB,
			      const std::string& instrumentParameterDB,
			      unsigned int subBand,
			      bool calcUVW);

    virtual int doProcess (int operation, int streamId,
                           LOFAR::BlobIStream& in,
                           LOFAR::BlobOStream& out);

  private:
    int itsMaxIter;
    int itsNrIter;
  };

}} // end namespaces

#endif
