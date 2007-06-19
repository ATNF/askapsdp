//#  PredifferBBS.h: 
//#
//#  Copyright (C) 2007
//#
//#  $Id$

#ifndef CONRAD_MWCONTROL_PREDIFFERBBS_H
#define CONRAD_MWCONTROL_PREDIFFERBBS_H

#include <mwcontrol/PredifferProxy.h>

//# Forward Declarations.
namespace LOFAR { namespace BBS {
  class Prediffer;
}}


namespace conrad { namespace cp {

  class PredifferBBS: public PredifferProxy
  {
  public:
    PredifferBBS();

    ~PredifferBBS();

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
    LOFAR::BBS::Prediffer* itsPrediffer;
  };

}} /// end namespaces

#endif
