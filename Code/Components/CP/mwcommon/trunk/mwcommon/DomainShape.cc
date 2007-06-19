//# DomainShape.cc: Define the shape of a domain
//#
//# @copyright (c) 2007 CONRAD, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <mwcommon/DomainShape.h>

namespace conrad { namespace cp {

  DomainShape::DomainShape()
    : itsFreqSize (1e30),
      itsTimeSize (1e30)
  {}

  DomainShape::DomainShape (double freqSize, double timeSize)
    : itsFreqSize (freqSize),
      itsTimeSize (timeSize)
  {}

  LOFAR::BlobOStream& operator<< (LOFAR::BlobOStream& bs,
				  const DomainShape& ds)
  {
    bs << ds.itsFreqSize << ds.itsTimeSize;
    return bs;
  }

  LOFAR::BlobIStream& operator>> (LOFAR::BlobIStream& bs,
				  DomainShape& ds)
  {
    bs >> ds.itsFreqSize >> ds.itsTimeSize;
    return bs;
  }

  std::ostream& operator<< (std::ostream& os,
			    const DomainShape& ds)
  {
    os << ds.itsFreqSize << " Hz,  " << ds.itsTimeSize << " s";
    return os;
  }

}} // end namespaces
