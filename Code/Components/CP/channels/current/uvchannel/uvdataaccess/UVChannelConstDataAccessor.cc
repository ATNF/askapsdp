/// @file UVChannelConstDataAccessor.cc
/// @brief
///
/// @copyright (c) 2011 CSIRO
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
#include "UVChannelConstDataAccessor.h"

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "casa/aipstype.h"
#include "casa/Arrays/Cube.h"
#include "casa/Quanta/MVDirection.h"
#include "scimath/Mathematics/RigidVector.h"
#include "cpcommon/VisChunk.h"

// Local includes
#include "uvchannel/uvdataaccess/UVChannelConstDataAccessor.h"

// Using
using namespace askap;
using namespace askap::synthesis;
using namespace askap::cp::channels;
using namespace askap::cp::common;

UVChannelConstDataAccessor::UVChannelConstDataAccessor(const askap::cp::common::VisChunk& chunk)
        : itsChunk(chunk)
{
}

casa::uInt UVChannelConstDataAccessor::nRow() const throw()
{
    return itsChunk.nRow();
}

casa::uInt UVChannelConstDataAccessor::nChannel() const throw()
{
    return itsChunk.nChannel();
}

casa::uInt UVChannelConstDataAccessor::nPol() const throw()
{
    return itsChunk.nPol();
}

const casa::Cube<casa::Complex>& UVChannelConstDataAccessor::visibility() const
{
    return itsChunk.visibility();
}

const casa::Cube<casa::Bool>& UVChannelConstDataAccessor::flag() const
{
    return itsChunk.flag();
}

const casa::Vector<casa::RigidVector<casa::Double, 3> >&
UVChannelConstDataAccessor::uvw() const
{
    return itsChunk.uvw();
}

const casa::Vector<casa::RigidVector<casa::Double, 3> >& UVChannelConstDataAccessor::rotatedUVW(
    const casa::MDirection &tangentPoint) const
{
    return itsRotatedUVW.uvw(*this, tangentPoint);
}

const casa::Vector<casa::Double>& UVChannelConstDataAccessor::uvwRotationDelay(
    const casa::MDirection &tangentPoint, const casa::MDirection &imageCentre) const
{
    return itsRotatedUVW.delays(*this, tangentPoint, imageCentre);
}

const casa::Vector<casa::Double>& UVChannelConstDataAccessor::frequency() const
{
    return itsChunk.frequency();
}

casa::Double UVChannelConstDataAccessor::time() const
{
    return itsChunk.time().get();
}

const casa::Vector<casa::uInt>& UVChannelConstDataAccessor::antenna1() const
{
    return itsChunk.antenna1();
}

const casa::Vector<casa::uInt>& UVChannelConstDataAccessor::antenna2() const
{
    return itsChunk.antenna2();
}

const casa::Vector<casa::uInt>& UVChannelConstDataAccessor::feed1() const
{
    return itsChunk.beam1();
}

const casa::Vector<casa::uInt>& UVChannelConstDataAccessor::feed2() const
{
    return itsChunk.beam2();
}

const casa::Vector<casa::Float>& UVChannelConstDataAccessor::feed1PA() const
{
    return itsChunk.beam1PA();
}

const casa::Vector<casa::Float>& UVChannelConstDataAccessor::feed2PA() const
{
    return itsChunk.beam2PA();
}

const casa::Vector<casa::MVDirection>& UVChannelConstDataAccessor::pointingDir1() const
{
    return itsChunk.pointingDir1();
}

const casa::Vector<casa::MVDirection>& UVChannelConstDataAccessor::pointingDir2()  const
{
    return itsChunk.pointingDir1();
}

const casa::Vector<casa::MVDirection>& UVChannelConstDataAccessor::dishPointing1() const
{
    return itsChunk.dishPointing1();
}

const casa::Vector<casa::MVDirection>& UVChannelConstDataAccessor::dishPointing2() const
{
    return itsChunk.dishPointing2();
}

const casa::Cube<casa::Complex>& UVChannelConstDataAccessor::noise() const
{
    ASKAPTHROW(AskapError, "UVChannelConstDataAccessor::noise() not implemented");
}

const casa::Vector<casa::Double>& UVChannelConstDataAccessor::velocity() const
{
    ASKAPTHROW(AskapError, "UVChannelConstDataAccessor::velocity() not implemented");
}

const casa::Vector<casa::Stokes::StokesTypes>& UVChannelConstDataAccessor::stokes() const
{
    ASKAPTHROW(AskapError, "UVChannelConstDataAccessor::stokes() not implemented");
}
