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
#include "casa/aips.h"
#include "casa/Quanta/MVEpoch.h"
#include "casa/Arrays/Vector.h"
#include "casa/Arrays/Matrix.h"
#include "casa/Arrays/Cube.h"
#include "measures/Measures/MDirection.h"
#include "scimath/Mathematics/RigidVector.h"
#include "measures/Measures/Stokes.h"

// Using
using namespace askap::cp;

VisChunk::VisChunk()
        : itsNumberOfRows(0), itsNumberOfChannels(0), itsNumberOfPolarisations(0)
{
}

VisChunk::VisChunk(const casa::uInt nRow,
                   const casa::uInt nChannel,
                   const casa::uInt nPol)
        : itsNumberOfRows(nRow), itsNumberOfChannels(nChannel),
        itsNumberOfPolarisations(nPol),
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

casa::uInt& VisChunk::nChannel()
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

casa::Vector<casa::MDirection>& VisChunk::pointingDir1()
{
    return itsPointingDir1;
}

const casa::Vector<casa::MDirection>& VisChunk::pointingDir1() const
{
    return itsPointingDir1;
}

casa::Vector<casa::MDirection>& VisChunk::pointingDir2()
{
    return itsPointingDir2;
}

const casa::Vector<casa::MDirection>& VisChunk::pointingDir2() const
{
    return itsPointingDir2;
}

casa::Vector<casa::MDirection>& VisChunk::dishPointing1()
{
    return itsDishPointing1;
}

const casa::Vector<casa::MDirection>& VisChunk::dishPointing1() const
{
    return itsDishPointing1;
}

casa::Vector<casa::MDirection>& VisChunk::dishPointing2()
{
    return itsDishPointing2;
}

const casa::Vector<casa::MDirection>& VisChunk::dishPointing2() const
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
