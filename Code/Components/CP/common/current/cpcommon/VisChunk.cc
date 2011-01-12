/// @file VisChunk.cc
///
/// @copyright (c) 2010 CSIRO
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
///

// Include own header file first
#include "VisChunk.h"

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "casa/aips.h"
#include "casa/Quanta/MVEpoch.h"
#include "casa/Quanta/MVDirection.h"
#include "casa/Arrays/Array.h"
#include "casa/Arrays/Vector.h"
#include "casa/Arrays/Matrix.h"
#include "casa/Arrays/Cube.h"
#include "scimath/Mathematics/RigidVector.h"
#include "measures/Measures/Stokes.h"
#include "Blob/BlobOStream.h"
#include "Blob/BlobIStream.h"
#include "Blob/BlobArray.h"
#include "Blob/BlobSTL.h"

// Local includes
#include "CasaBlobUtils.h"

// Using
using namespace askap::cp::common;

VisChunk::VisChunk(const casa::uInt nRow,
                   const casa::uInt nChannel,
                   const casa::uInt nPol)
        : itsNumberOfRows(nRow), itsNumberOfChannels(nChannel),
        itsNumberOfPolarisations(nPol),
        itsTime(-1),
        itsInterval(-1),
        itsAntenna1(nRow),
        itsAntenna2(nRow),
        itsBeam1(nRow),
        itsBeam2(nRow),
        itsBeam1PA(nRow),
        itsBeam2PA(nRow),
        itsPointingDir1(nRow),
        itsPointingDir2(nRow),
        itsDishPointing1(nRow),
        itsDishPointing2(nRow),
        itsVisibility(nRow, nChannel, nPol),
        itsFlag(nRow, nChannel, nPol),
        itsUVW(nRow),
        itsFrequency(nChannel),
        itsStokes(nPol)
{
}

VisChunk::~VisChunk()
{
}

casa::uInt VisChunk::nRow() const
{
    return itsNumberOfRows;
}

casa::uInt VisChunk::nChannel() const
{
    return itsNumberOfChannels;
}

casa::uInt VisChunk::nPol() const
{
    return itsNumberOfPolarisations;
}

casa::Vector<casa::uInt>& VisChunk::antenna1()
{
    return itsAntenna1;
}

const casa::Vector<casa::uInt>& VisChunk::antenna1() const
{
    return itsAntenna1;
}

casa::Vector<casa::uInt>& VisChunk::antenna2()
{
    return itsAntenna2;
}

const casa::Vector<casa::uInt>& VisChunk::antenna2() const
{
    return itsAntenna2;
}

casa::Vector<casa::uInt>& VisChunk::beam1()
{
    return itsBeam1;
}

const casa::Vector<casa::uInt>& VisChunk::beam1() const
{
    return itsBeam1;
}

casa::Vector<casa::uInt>& VisChunk::beam2()
{
    return itsBeam2;
}

const casa::Vector<casa::uInt>& VisChunk::beam2() const
{
    return itsBeam2;
}

casa::Vector<casa::Float>& VisChunk::beam1PA()
{
    return itsBeam1PA;
}

const casa::Vector<casa::Float>& VisChunk::beam1PA() const
{
    return itsBeam1PA;
}

casa::Vector<casa::Float>& VisChunk::beam2PA()
{
    return itsBeam2PA;
}

const casa::Vector<casa::Float>& VisChunk::beam2PA() const
{
    return itsBeam2PA;
}

casa::Vector<casa::MVDirection>& VisChunk::pointingDir1()
{
    return itsPointingDir1;
}

const casa::Vector<casa::MVDirection>& VisChunk::pointingDir1() const
{
    return itsPointingDir1;
}

casa::Vector<casa::MVDirection>& VisChunk::pointingDir2()
{
    return itsPointingDir2;
}

const casa::Vector<casa::MVDirection>& VisChunk::pointingDir2() const
{
    return itsPointingDir2;
}

casa::Vector<casa::MVDirection>& VisChunk::dishPointing1()
{
    return itsDishPointing1;
}

const casa::Vector<casa::MVDirection>& VisChunk::dishPointing1() const
{
    return itsDishPointing1;
}

casa::Vector<casa::MVDirection>& VisChunk::dishPointing2()
{
    return itsDishPointing2;
}

const casa::Vector<casa::MVDirection>& VisChunk::dishPointing2() const
{
    return itsDishPointing2;
}

casa::Cube<casa::Complex>& VisChunk::visibility()
{
    return itsVisibility;
}

const casa::Cube<casa::Complex>& VisChunk::visibility() const
{
    return itsVisibility;
}

casa::Cube<casa::Bool>& VisChunk::flag()
{
    return itsFlag;
}

const casa::Cube<casa::Bool>& VisChunk::flag() const
{
    return itsFlag;
}

casa::Vector<casa::RigidVector<casa::Double, 3> >& VisChunk::uvw()
{
    return itsUVW;
}

const casa::Vector<casa::RigidVector<casa::Double, 3> >& VisChunk::uvw() const
{
    return itsUVW;
}

casa::MVEpoch& VisChunk::time()
{
    return itsTime;
}

const casa::MVEpoch& VisChunk::time() const
{
    return itsTime;
}

casa::Double& VisChunk::interval()
{
    return itsInterval;
}

const casa::Double& VisChunk::interval() const
{
    return itsInterval;
}

casa::Vector<casa::Double>& VisChunk::frequency()
{
    return itsFrequency;
}

const casa::Vector<casa::Double>& VisChunk::frequency() const
{
    return itsFrequency;
}

casa::Vector<casa::Stokes::StokesTypes>& VisChunk::stokes()
{
    return itsStokes;
}

const casa::Vector<casa::Stokes::StokesTypes>& VisChunk::stokes() const
{
    return itsStokes;
}

void VisChunk::resize(const casa::Cube<casa::Complex>& visibility,
        const casa::Cube<casa::Bool>& flag,
        const casa::Vector<casa::Double>& frequency)
{
    if ((visibility.nrow() != itsNumberOfRows) && (flag.nrow() != itsNumberOfRows)) {
        ASKAPTHROW(AskapError,
                "New cubes must have the same number of rows as the existing cubes");
    }

    if ((visibility.nplane() != itsNumberOfPolarisations) && (flag.nplane() != itsNumberOfPolarisations)) {
        ASKAPTHROW(AskapError,
                "New cubes must have the same number of polarisations as the existing cubes");
    }

    const casa::uInt newNChan = visibility.ncolumn();
    if (newNChan != flag.ncolumn() || newNChan != frequency.size()) {
        ASKAPTHROW(AskapError, "Number of channels must be equal for all input containers");
    }

    itsVisibility.assign(visibility);
    itsFlag.assign(flag);
    itsFrequency.assign(frequency);

    itsNumberOfChannels = newNChan;
}

/////////////////////////////////////////////////////////////////////
// Serializers
/////////////////////////////////////////////////////////////////////
void VisChunk::writeToBlob(LOFAR::BlobOStream& os) const
{
    os << itsNumberOfRows;
    os << itsNumberOfChannels;
    os << itsNumberOfPolarisations;
    os << itsTime;
    os << itsInterval;
    os << itsAntenna1;
    os << itsAntenna2;
    os << itsBeam1;
    os << itsBeam2;
    os << itsBeam1PA;
    os << itsBeam2PA;
    os << itsPointingDir1;
    os << itsPointingDir2;
    os << itsDishPointing1;
    os << itsDishPointing2;
    os << itsVisibility;
    os << itsFlag;
    os << itsUVW;
    os << itsFrequency;
    os << itsStokes;
}

void VisChunk::readFromBlob(LOFAR::BlobIStream& is)
{
    is >> itsNumberOfRows;
    is >> itsNumberOfChannels;
    is >> itsNumberOfPolarisations;
    is >> itsTime;
    is >> itsInterval;
    is >> itsAntenna1;
    is >> itsAntenna2;
    is >> itsBeam1;
    is >> itsBeam2;
    is >> itsBeam1PA;
    is >> itsBeam2PA;
    is >> itsPointingDir1;
    is >> itsPointingDir2;
    is >> itsDishPointing1;
    is >> itsDishPointing2;
    is >> itsVisibility;
    is >> itsFlag;
    is >> itsUVW;
    is >> itsFrequency;
    is >> itsStokes;
}
