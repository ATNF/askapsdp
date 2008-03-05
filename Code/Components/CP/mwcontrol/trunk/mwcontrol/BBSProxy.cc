//#  BBSProxy.cc: 
//#
//#  Copyright (C) 2007
//#
//#  $Id$

#include <mwcontrol/BBSProxy.h>
#include <mwcommon/MasterControl.h>
#include <Blob/BlobIStream.h>

namespace askap { namespace cp {

  BBSProxy::~BBSProxy()
  {}

  int BBSProxy::process (int operation, int streamId,
                         LOFAR::BlobIStream& in,
                         LOFAR::BlobOStream& out)
  {
    int resOper = operation;
    if (operation == MasterControl::Init) {
      std::string msName, msSuffix, colName, skyDB, instDB;
      LOFAR::uint32 subBand;
      bool calcUVW;
      in >> msName >> msSuffix >> colName >> skyDB >> instDB
	 >> subBand >> calcUVW;
      setInitInfo (msName+msSuffix, colName, skyDB, instDB,
		   subBand, calcUVW);
    } else {
      resOper = doProcess (operation, streamId, in, out);
    }
    return resOper;
  }

}} // end namespaces
