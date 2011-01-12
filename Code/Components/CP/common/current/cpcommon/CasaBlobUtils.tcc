/// @file CasaBlobUtils.tcc
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

#ifndef ASKAP_CP_COMMON_CASABLOBUTILS_H
#error "Don't include this file directly, include CasaBlobUtils.h instead"
#endif

// ASKAPsoft includes
#include "casa/aips.h"
#include "casa/Quanta/MVEpoch.h"
#include "casa/Quanta/MVDirection.h"
#include "measures/Measures/MDirection.h"
#include "measures/Measures/Stokes.h"
#include "scimath/Mathematics/RigidVector.h"
#include "Blob/BlobOStream.h"
#include "Blob/BlobIStream.h"

namespace LOFAR {

        LOFAR::BlobOStream& operator<<(LOFAR::BlobOStream& os, const casa::MVEpoch& obj)
        {
            os << obj.get();
            return os;
        }

        LOFAR::BlobIStream& operator>>(LOFAR::BlobIStream& is, casa::MVEpoch& obj)
        {
            casa::Double time;
            is >> time;
            obj = casa::MVEpoch(time);
            return is;
        }

        LOFAR::BlobOStream& operator<<(LOFAR::BlobOStream& os, const casa::MDirection& obj)
        {
            os << obj.getAngle().getValue()(0);
            os << obj.getAngle().getValue()(1);
            os << obj.getAngle().getUnit();
            os << obj.getRefString();
            return os;
        }

        LOFAR::BlobIStream& operator>>(LOFAR::BlobIStream& is, casa::MDirection& obj)
        {
            casa::Double coord1;
            casa::Double coord2;
            casa::String ref;
            casa::String unit;
            is >> coord1;
            is >> coord2;
            is >> unit;
            is >> ref;
            casa::MDirection dir(casa::Quantity(coord1, unit),
                                 casa::Quantity(coord2, unit));
            dir.setRefString(ref);
            obj = dir;
            return is;
        }

        LOFAR::BlobOStream& operator<<(LOFAR::BlobOStream& os, const casa::MVDirection& obj)
        {
            os << obj.get();
            return os;
        }

        LOFAR::BlobIStream& operator>>(LOFAR::BlobIStream& is, casa::MVDirection& obj)
        {
            casa::Vector<casa::Double> vec;
            is >> vec;
            casa::MVDirection md(vec);
            obj = md;
            return is;
        }

        LOFAR::BlobOStream& operator<<(LOFAR::BlobOStream& os, const casa::Stokes::StokesTypes& obj)
        {
            os << static_cast<int>(obj);
            return os;
        }

        LOFAR::BlobIStream& operator>>(LOFAR::BlobIStream& is, casa::Stokes::StokesTypes& obj)
        {
            int stokes;
            is >> stokes;
            obj = static_cast<casa::Stokes::StokesTypes>(stokes);

            return is;
        }

        template<class T, int n>
        LOFAR::BlobOStream& operator<<(LOFAR::BlobOStream& os, const casa::RigidVector<T, n>& obj)
        {
            for (int i = 0; i < n; ++i) {
            os << obj(i);
            }
            return os;
        }

        template<class T, int n>
        LOFAR::BlobIStream& operator>>(LOFAR::BlobIStream& is, casa::RigidVector<T, n>& obj)
        {
            for (int i = 0; i < n; ++i) {
                is >> obj(i);
            }
            return is;
        }


} // End namespace LOFAR
