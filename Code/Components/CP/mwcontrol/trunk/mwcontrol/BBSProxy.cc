//#  BBSProxy.cc: 
//#
//#  Copyright (C) 2007
//#
//#  $Id$

#include <mwcontrol/BBSProxy.h>
#include <mwcommon/MasterControl.h>
#include <Blob/BlobIStream.h>

namespace conrad { namespace cp {

  BBSProxy::~BBSProxy()
  {}

  void BBSProxy::process (int operation, int streamId,
			  LOFAR::BlobIStream& in,
			  LOFAR::BlobString& out)
  {
    if (operation == MasterControl::Init) {
      std::string msName, msSuffix, colName, skyDB, instDB;
      LOFAR::uint32 subBand;
      bool calcUVW;
      in >> msName >> msSuffix >> colName >> skyDB >> instDB
	 >> subBand >> calcUVW;
      setInitInfo (msName+msSuffix, colName, skyDB, instDB,
		   subBand, calcUVW);
    } else {
      doProcess (operation, streamId, in, out);
    }
  }

}} // end namespaces
