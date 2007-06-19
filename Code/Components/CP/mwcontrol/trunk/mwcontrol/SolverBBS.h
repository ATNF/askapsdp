//#  SolverBBS.h: 
//#
//#  Copyright (C) 2007
//#
//#  $Id$

#ifndef CONRAD_MWCONTROL_SOLVERBBS_H
#define CONRAD_MWCONTROL_SOLVERBBS_H

#include <mwcontrol/SolverProxy.h>

//# Forward Declarations.
namespace LOFAR { namespace BBS {
  class Solver;
}}


namespace conrad { namespace cp {

  class SolverBBS: public SolverProxy
  {
  public:
    SolverBBS();

    ~SolverBBS();

    /// Create a new object.
    static WorkerProxy::ShPtr create();

    virtual void setInitInfo (const std::string& measurementSet,
			      const std::string& inputColumn,
			      const std::string& skyParameterDB,
			      const std::string& instrumentParameterDB,
			      unsigned int subBand,
			      bool calcUVW);

    virtual void doProcess (int operation, int streamId,
			    LOFAR::BlobIStream& in,
			    LOFAR::BlobString& out);

  private:
    LOFAR::BBS::Solver* itsSolver;
  };

}} /// end namespaces

#endif
