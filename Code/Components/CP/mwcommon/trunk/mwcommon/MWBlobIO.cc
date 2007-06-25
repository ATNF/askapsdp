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

  MWBlobOut::MWBlobOut (LOFAR::BlobString& buf, int operation, int streamId,
                        int workerId)
    : itsBuf    (buf),
      itsStream (itsBuf)
  {
    itsStream.putStart ("mw", 1);
    itsOperOffset = buf.size();
    itsStream << static_cast<LOFAR::int32>(operation)
	      << static_cast<LOFAR::int32>(streamId)
	      << static_cast<LOFAR::int32>(workerId);
    CONRADASSERT (buf.size() == itsOperOffset + 3*sizeof(LOFAR::int32));
    itsTimeOffset = buf.size();
    // Put empty times. They will be set later by setTimes.
    itsStream << float(0) << float(0) << float(0) << double(0);
    CONRADASSERT (buf.size() == itsTimeOffset
                                + 3*sizeof(float) + sizeof(double));
  }

  void MWBlobOut::setOperation (int operation)
  {
    using LOFAR::uchar;
    LOFAR::int32 oper = operation;
    uchar* ptr = const_cast<uchar*>(itsBuf.getBuffer()) + itsOperOffset;
    // Use memcpy, because in buffer it might be unaligned.
    memcpy (ptr, &oper, sizeof(LOFAR::int32));
  }

  void MWBlobOut::setTimes (const casa::Timer& low, const LOFAR::NSTimer& high)
  {
    using LOFAR::uchar;
    float t[3];
    t[0] = low.real();
    t[1] = low.system();
    t[2] = low.user();
    uchar* ptr = const_cast<uchar*>(itsBuf.getBuffer()) + itsTimeOffset;
    // Use memcpy, because in buffer it might be unaligned.
    memcpy (ptr, t, 3*sizeof(float));
    double d = high.getElapsed();
    memcpy (ptr + 3*sizeof(float), &d, sizeof(double));
  }


  MWBlobIn::MWBlobIn (const LOFAR::BlobString& buf)
    : itsBuf    (buf),
      itsStream (itsBuf)
  {
    int version = itsStream.getStart ("mw");
    CONRADASSERT (version==1);
    itsStream >> itsOper >> itsStreamId >> itsWorkerId
              >> itsElapsedTime >> itsSystemTime >> itsUserTime >> itsPrecTime;
  }


}} //end namespaces
