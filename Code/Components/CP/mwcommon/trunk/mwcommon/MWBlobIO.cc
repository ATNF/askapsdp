//# MWBlobIO.cc: 
//#
//# @copyright (c) 2007 CONRAD, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$


#include <mwcommon/MWBlobIO.h>
#include <mwcommon/MWError.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>

namespace conrad { namespace cp {

  MWBlobOut::MWBlobOut (LOFAR::BlobString& buf, int operation, int streamId)
    : itsBuf    (buf),
      itsStream (itsBuf)
  {
    itsStream.putStart ("mw", 1);
    itsStream << static_cast<LOFAR::int32>(operation)
	      << static_cast<LOFAR::int32>(streamId);
  }


  MWBlobIn::MWBlobIn (const LOFAR::BlobString& buf)
    : itsBuf    (buf),
      itsStream (itsBuf)
  {
    int version = itsStream.getStart ("mw");
    CONRADASSERT (version==1);
    itsStream >> itsOper >> itsStreamId;
  }


}} //end namespaces
