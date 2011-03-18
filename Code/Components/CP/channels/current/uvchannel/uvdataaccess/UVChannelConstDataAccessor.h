/// @file UVChannelConstDataAccessor.h
/// @brief an implementation of IConstDataAccessor.
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
///
#ifndef ASKAP_CP_CHANNELS_UVCHANNEL_CONST_DATA_ACCESSOR_H
#define ASKAP_CP_CHANNELS_UVCHANNEL_CONST_DATA_ACCESSOR_H

// ASKAPsoft includes
#include "boost/shared_ptr.hpp"
#include "casa/aipstype.h"
#include "casa/Arrays/Cube.h"
#include "casa/Quanta/MVDirection.h"
#include "scimath/Mathematics/RigidVector.h"
#include "cpcommon/VisChunk.h"
#include "dataaccess/IConstDataAccessor.h"
#include "dataaccess/DataAccessError.h"
#include "dataaccess/UVWRotationHandler.h"

namespace askap {
namespace cp {
namespace channels {

/// @brief An implementation of the IConstDataAccessor interface for the
/// visibility stream.
/// @ingroup uvdataaccess
class UVChannelConstDataAccessor : virtual public askap::accessors::IConstDataAccessor {
    public:
        explicit UVChannelConstDataAccessor(const boost::shared_ptr<askap::cp::common::VisChunk> chunk);

        virtual casa::uInt nRow() const throw();

        virtual casa::uInt nChannel() const throw();

        virtual casa::uInt nPol() const throw();

        virtual const casa::Vector<casa::MVDirection>& pointingDir1() const;

        virtual const casa::Vector<casa::MVDirection>& pointingDir2() const;

        virtual const casa::Vector<casa::MVDirection>& dishPointing1() const;

        virtual const casa::Vector<casa::MVDirection>& dishPointing2() const;

        virtual const casa::Cube<casa::Complex>& visibility() const;

        virtual const casa::Cube<casa::Bool>& flag() const;

        virtual const casa::Vector<casa::RigidVector<casa::Double, 3> >&uvw() const;

        virtual const casa::Vector<casa::RigidVector<casa::Double, 3> >& rotatedUVW(
            const casa::MDirection &tangentPoint) const;

        virtual const casa::Vector<casa::Double>& uvwRotationDelay(
            const casa::MDirection &tangentPoint, const casa::MDirection &imageCentre) const;

        virtual const casa::Vector<casa::Double>& frequency() const;

        virtual casa::Double time() const;

        virtual const casa::Vector<casa::uInt>& antenna1() const;

        virtual const casa::Vector<casa::uInt>& antenna2() const;

        virtual const casa::Vector<casa::uInt>& feed1() const;

        virtual const casa::Vector<casa::uInt>& feed2() const;

        virtual const casa::Vector<casa::Float>& feed1PA() const;

        virtual const casa::Vector<casa::Float>& feed2PA() const;

        virtual const casa::Cube<casa::Complex>& noise() const;

        virtual const casa::Vector<casa::Double>& velocity() const;

        virtual const casa::Vector<casa::Stokes::StokesTypes>& stokes() const;

    private:

        // The primary data structure that this accessor is wrapping
        boost::shared_ptr<askap::cp::common::VisChunk> itsChunk;

        // UVW Rotation handler
        askap::accessors::UVWRotationHandler itsRotatedUVW;

        // Temporary cube to contain noise. For now the noise cube is
        // all 1.0. This needs to be addressed in future.
        casa::Cube<casa::Complex> itsNoise;
};

} // namespace channels
} // namespace cp
} // namespace askap

#endif
