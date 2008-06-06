//# DomainShape.cc: Define the shape of a domain
//#
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
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <mwcommon/DomainShape.h>

namespace askap { namespace cp {

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
