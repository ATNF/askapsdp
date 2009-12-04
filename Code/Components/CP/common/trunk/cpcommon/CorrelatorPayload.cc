/// @file CorrelatorPayload.cc
///
/// @copyright (c) 2009 CSIRO
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
/// @author Ben Humphreys <ben.humphreys@csiro.au>

// Include own header file first
#include <cpcommon/CorrelatorPayload.h>

// ASKAPsoft includes
#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobArray.h>
#include <Blob/BlobSTL.h>
#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Cube.h>
#include <measures/Measures/Stokes.h>

// Using
using namespace askap::cp;

/////////////////////////////////////////////////////////////////////
// Serializers
/////////////////////////////////////////////////////////////////////
LOFAR::BlobOStream& operator<<(LOFAR::BlobOStream &os, const CorrelatorPayload& obj)
{
    os << static_cast<LOFAR::TYPES::uint64>(obj.timestamp);
    os << static_cast<LOFAR::TYPES::uint32>(obj.coarseChannel);
    os << static_cast<LOFAR::TYPES::uint32>(obj.nRow);
    os << static_cast<LOFAR::TYPES::uint32>(obj.nChannel);
    os << static_cast<LOFAR::TYPES::uint32>(obj.nPol);
    os << obj.antenna1;
    os << obj.antenna2;
    os << obj.beam1;
    os << obj.beam2;

    // Polarisations is an enum, need to convert to int first
    casa::Vector<int> polarisations;
    polarisations.resize(obj.polarisations.size());
    for (unsigned int i = 0; i < obj.polarisations.size(); ++i) {
        polarisations(i) = obj.polarisations(i);
    }
    os << polarisations;

    os << obj.vis;
    os << obj.nSamples;
    os << static_cast<LOFAR::TYPES::uint32>(obj.nominalNSamples);

    return os;
}

LOFAR::BlobIStream& operator>>(LOFAR::BlobIStream &is, CorrelatorPayload& obj)
{
    // timestamp
    LOFAR::TYPES::uint64 timestamp;
    is >> timestamp;
    obj.timestamp = timestamp;

    // coarseChannel
    LOFAR::TYPES::uint32 coarseChannel;
    is >> coarseChannel;
    obj.coarseChannel = coarseChannel;
  
    // nRow 
    LOFAR::TYPES::uint32 nRow; 
    is >> nRow;
    obj.nRow = nRow;

    // nChannel
    LOFAR::TYPES::uint32 nChannel; 
    is >> nChannel;
    obj.nChannel = nChannel;

    // nPol
    LOFAR::TYPES::uint32 nPol; 
    is >> nPol;
    obj.nPol = nPol;

    // antenna1
    is >> obj.antenna1;

    // antenna2
    is >> obj.antenna2;

    // beam1
    is >> obj.beam1;

    // beam1
    is >> obj.beam2;

    // Polarisations is an enum, need to convert from int
    casa::Vector<int> polarisations;
    is >> polarisations;
    const unsigned int polVecLength = polarisations.size();
    obj.polarisations.resize(polVecLength);
    for (unsigned int i = 0; i < polVecLength; ++i) {
        obj.polarisations(i) = casa::Stokes::type(polarisations(i));
    }

    // vis
    is >> obj.vis;

    // nSamples
    is >> obj.nSamples;

    // nominalNSamples
    LOFAR::TYPES::uint32 nominalNSamples; 
    is >> nominalNSamples;
    obj.nominalNSamples = nominalNSamples;

    return is;
}
