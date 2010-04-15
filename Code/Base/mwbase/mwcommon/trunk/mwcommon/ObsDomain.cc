/// @file
/// @brief Define the shape of a domain
///
/// @copyright (c) 2007 CSIRO
/// Australia Telescope National Facility (ATNF)
/// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
/// PO Box 76, Epping NSW 1710, Australia
/// atnf-enquiries@csiro.au
///
/// This file is part of the ASKAP software distribution.
///
/// The ASKAP software distribution is free software: you can redistribute it
/// and/or modify it under the terms of the GNU General Public License as
/// published by the Free Software Foundation; either version 2 of the License,
/// or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
///
/// @author Ger van Diepen <diepen@astron.nl>


#include <mwcommon/ObsDomain.h>

namespace askap { namespace mwbase {

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
