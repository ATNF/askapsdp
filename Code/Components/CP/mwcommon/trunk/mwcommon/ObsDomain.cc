//# ObsDomain.cc: Define the shape of a domain
//#
//# @copyright (c) 2007 CONRAD, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <mwcommon/ObsDomain.h>

namespace conrad { namespace cp {

  ObsDomain::ObsDomain()
    : itsStartFreq (-1),
      itsEndFreq   (1e30),
      itsStartTime (-1),
      itsEndTime   (1e30)
  {}

  ObsDomain::ObsDomain (const ObsDomain& fullDomain,
			const DomainShape& workDomainShape)
  {
    // Construct the first work domain from the full observation domain
    // and the work domain shape.
    double freqLen = workDomainShape.getFreqSize();
    double timeLen = workDomainShape.getTimeSize();
    itsStartFreq = fullDomain.getStartFreq();
    itsEndFreq   = std::min(fullDomain.getEndFreq(),
			    itsStartFreq+freqLen);
    itsStartTime = fullDomain.getStartTime();
    itsEndTime   = std::min(fullDomain.getEndTime(),
			    itsStartTime+timeLen);
  }

  void ObsDomain::setFreq (double startFreq, double endFreq)
  {
    itsStartFreq = startFreq;
    itsEndFreq   = endFreq;
  }
  
  void ObsDomain::setTime (double startTime, double endTime)
  {
    itsStartTime = startTime;
    itsEndTime   = endTime;
  }

  bool ObsDomain::getNextWorkDomain (ObsDomain& workDomain,
				     const DomainShape& workDomainShape) const
  {
    double freqLen = workDomainShape.getFreqSize();
    double timeLen = workDomainShape.getTimeSize();
    // First time?
    if (workDomain.getStartFreq() < 0) {
      workDomain.setFreq (itsStartFreq,
			  std::min(itsEndFreq, itsStartFreq+freqLen));
      workDomain.setTime (itsStartTime,
			  std::min(itsEndTime, itsStartTime+timeLen));
      return true;
    }
    // Increment in frequency if possible.
    double sfreq = workDomain.getStartFreq() + freqLen;
    if (sfreq < itsEndFreq) {
      workDomain.setFreq (sfreq, std::min(itsEndFreq, sfreq+freqLen));
      return true;
    }
    double stime = workDomain.getStartTime() + timeLen;
    if (stime < itsEndTime) {
      // Reset work domain (for freq) and set times.
      workDomain = ObsDomain(*this, workDomainShape);
      workDomain.setTime (stime, std::min(itsEndTime, stime+timeLen));
      return true;
    }
    return false;
  }

  LOFAR::BlobOStream& operator<< (LOFAR::BlobOStream& bs,
				  const ObsDomain& domain)
  {
    bs << domain.itsStartFreq << domain.itsEndFreq
       << domain.itsStartTime << domain.itsEndTime;
    return bs;
  }

  LOFAR::BlobIStream& operator>> (LOFAR::BlobIStream& bs,
				  ObsDomain& domain)
  {
    bs >> domain.itsStartFreq >> domain.itsEndFreq
       >> domain.itsStartTime >> domain.itsEndTime;
    return bs;
  }

  std::ostream& operator<< (std::ostream& os,
			    const ObsDomain& domain)
  {
    os << '[' << domain.itsStartFreq << " Hz, " << domain.itsEndFreq
       << " Hz, " << domain.itsStartTime << ", " << domain.itsEndTime << ']';
    return os;
  }


}} // end namespaces
