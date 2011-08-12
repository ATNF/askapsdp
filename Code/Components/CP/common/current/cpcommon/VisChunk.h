/// @file VisChunk.h
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
#ifndef ASKAP_CP_INGEST_VISCHUNK_H
#define ASKAP_CP_INGEST_VISCHUNK_H

// ASKAPsoft includes
#include "casa/aips.h"
#include "casa/Quanta/MVEpoch.h"
#include "casa/Quanta/MVDirection.h"
#include "casa/Arrays/Vector.h"
#include "casa/Arrays/Matrix.h"
#include "casa/Arrays/Cube.h"
#include "scimath/Mathematics/RigidVector.h"
#include "measures/Measures/Stokes.h"
#include "measures/Measures/MDirection.h"
#include "boost/shared_ptr.hpp"
#include "Blob/BlobOStream.h"
#include "Blob/BlobIStream.h"
#include "fitting/ISerializable.h"

namespace askap {
namespace cp {
namespace common {

class VisChunk : public ISerializable {
    public:
        /// @brief Constructor.
        /// Construct a VisChunk where its containers are created with
        /// the dimensions specified.
        ///
        /// @param[in] nRow containers with a nRow dimension will be created
        ///                 with this size for that dimension.
        /// @param[in] nChannel containers with a nChannel dimension will
        ///                     be created with this size for that dimension.
        /// @param[in] nPol containers with a nPol dimension will be created
        ///                 with this size for that dimension.
        VisChunk(const casa::uInt nRow,
                 const casa::uInt nChannel,
                 const casa::uInt nPol);

        /// Destructor
        ~VisChunk();

        /// The number of rows in this chunk
        /// @return the number of rows in this chunk
        casa::uInt nRow() const;

        // The following methods implement metadata access

        /// The number of spectral channels (equal for all rows)
        /// @return the number of spectral channels
        casa::uInt nChannel() const;

        /// The number of polarization products (equal for all rows)
        /// @return the number of polarization products (can be 1,2 or 4)
        casa::uInt nPol() const;

        /// Timestamp for this correlator integration
        /// @return a timestamp for this buffer. Absolute time expressed as
        /// seconds since MJD=0 UTC.
        casa::MVEpoch& time();

        /// @copydoc VisChunk::time()
        const casa::MVEpoch& time() const;

        /// Data sampling interval.
        /// Units: Seconds
        casa::Double& interval();

        /// @copydoc VisChunk::interval()
        const casa::Double& interval() const;

        /// Scan index number (zero based).
        casa::uInt& scan();

        /// @copydoc VisChunk::scan()
        const casa::uInt& scan() const;

        /// First antenna IDs for all rows
        ///
        /// @note Antenna ID is zero based
        ///
        /// @return a vector with IDs of the first antenna corresponding
        /// to each visibility (one for each row)
        casa::Vector<casa::uInt>& antenna1();

        /// @copydoc VisChunk::antenna1()
        const casa::Vector<casa::uInt>& antenna1() const;

        /// Second antenna IDs for all rows
        ///
        /// @note Antenna ID is zero based
        ///
        /// @return a vector with IDs of the second antenna corresponding
        /// to each visibility (one for each row)
        casa::Vector<casa::uInt>& antenna2();

        /// @copydoc VisChunk::antenna2()
        const casa::Vector<casa::uInt>& antenna2() const;

        /// First beam IDs for all rows
        ///
        /// @note beam ID is zero based
        ///
        /// @return a vector with IDs of the first beam corresponding
        /// to each visibility (one for each row)
        casa::Vector<casa::uInt>& beam1();

        /// @copydoc VisChunk::beam1()
        const casa::Vector<casa::uInt>& beam1() const;

        /// Second beam IDs for all rows
        ///
        /// @note beam ID is zero based.
        ///
        /// @return a vector with IDs of the second beam corresponding
        /// to each visibility (one for each row)
        casa::Vector<casa::uInt>& beam2();

        /// @copydoc VisChunk::beam2()
        const casa::Vector<casa::uInt>& beam2() const;

        /// Position angles of the first beam for all rows
        /// @return a vector with position angles of the
        /// first beam corresponding to each visibility
        /// Units: Radians
        casa::Vector<casa::Float>& beam1PA();

        /// @copydoc VisChunk::beam1PA()
        const casa::Vector<casa::Float>& beam1PA() const;

        /// Position angles of the second beam for all rows
        /// @return a vector with position angles of the
        /// second beam corresponding to each visibility
        /// Units: Radians
        casa::Vector<casa::Float>& beam2PA();

        /// @copydoc VisChunk::beamsPA()
        const casa::Vector<casa::Float>& beam2PA() const;

        /// Return pointing centre directions of the first antenna/beam
        /// @return a vector with direction measures (coordinate system
        /// is set via IDataConverter), one direction for each
        /// visibility/row
        casa::Vector<casa::MVDirection>& pointingDir1();

        /// @copydoc VisChunk::pointingDir1()
        const casa::Vector<casa::MVDirection>& pointingDir1() const;

        /// Pointing centre directions of the second antenna/beam
        /// @return a vector with direction measures
        ///  one direction for each visibility/row
        casa::Vector<casa::MVDirection>& pointingDir2();

        /// @copydoc VisChunk::pointingDir2()
        const casa::Vector<casa::MVDirection>& pointingDir2() const;

        /// pointing direction for the centre of the first antenna
        /// @details The same as pointingDir1, if the beam offsets are zero
        /// @return a vector with direction measures,
        /// one direction for each visibility/row
        casa::Vector<casa::MVDirection>& dishPointing1();

        /// @copydoc VisChunk::dishPointing1
        const casa::Vector<casa::MVDirection>& dishPointing1() const;

        /// pointing direction for the centre of the first antenna
        /// @details The same as pointingDir2, if the beam offsets are zero
        /// @return a vector with direction measures one direction for each
        /// visibility/row
        casa::Vector<casa::MVDirection>& dishPointing2();

        /// @copydoc VisChunk::dishPointing2()
        const casa::Vector<casa::MVDirection>& dishPointing2() const;

        /// VisChunk (a cube is nRow x nChannel x nPol; each element is
        /// a complex visibility)
        /// @return a reference to nRow x nChannel x nPol cube, containing
        /// all visibility data
        casa::Cube<casa::Complex>& visibility();

        /// @copydoc VisChunk::visibility()
        const casa::Cube<casa::Complex>& visibility() const;

        /// Cube of flags corresponding to the output of visibility()
        /// @return a reference to nRow x nChannel x nPol cube with flag
        ///         information. If True, the corresponding element is flagged.
        casa::Cube<casa::Bool>& flag();

        /// @copydoc VisChunk::flag()
        const casa::Cube<casa::Bool>& flag() const;

        /// UVW
        /// @return a reference to vector containing uvw-coordinates
        /// packed into a 3-D rigid vector
        casa::Vector<casa::RigidVector<casa::Double, 3> >& uvw();

        /// @copydoc VisChunk::uvw()
        const casa::Vector<casa::RigidVector<casa::Double, 3> >& uvw() const;

        /// Frequency for each channel.
        /// Units: Hz
        /// @return a reference to vector containing frequencies for each
        ///         spectral channel (vector size is nChannel).
        casa::Vector<casa::Double>& frequency();

        /// @copydoc VisChunk::frequency()
        const casa::Vector<casa::Double>& frequency() const;

        /// Channel width of each spectral channel.
        /// All spectral channels in the frequency vector have a channel
        /// width which can be derived from frequency() by differencing,
        /// however is stored here for efficiency.
        /// Units: Hz
        /// @return  a refernece to the channel width of each spectral channel.
        casa::Double& channelWidth();

        /// @copydoc VisChunk::channelWidth()
        const casa::Double& channelWidth() const;

        /// @brief polarisation type for each product
        /// @return a reference to vector containing polarisation types for
        /// each product in the visibility cube (nPol() elements).
        /// @note All rows of the accessor have the same structure of the visibility
        /// cube, i.e. polarisation types returned by this method are valid for all rows.
        casa::Vector<casa::Stokes::StokesTypes>& stokes();

        /// @copydoc VisChunk::stokes()
        const casa::Vector<casa::Stokes::StokesTypes>& stokes() const;

        /// @brief direction reference frame for all MVDirection instances
        /// in this class.
        /// @return a reference to the MDirection:Ref.
        casa::MDirection::Ref& directionFrame();

        /// @copydoc VisChunk::directionFrame()
        const casa::MDirection::Ref& directionFrame() const;

        /// Allows the VisChunk's nChannel dimension to be resized.
        /// This allows resizing in the nChannel dimension only, and by
        /// allowing new visibility, flag and frequency containers to
        /// be assigned.
        ///
        /// @note This exists to support the channel averaging task.
        ///
        /// The following conditions must be met otherwise an
        /// AskapError exception is thrown:
        /// @li The visibility and flag cubes must have the same number
        ///     of rows and polarisations as the existing cubes.
        /// @li The visibility and flag cubes and the frequency vector
        ///     must have the same size channel dimension.
        ///
        /// @throw AskapError If one of the above mentioned conditions
        ///     not met.
        ///
        /// @param[in] visibility the new visibility cube to assign.
        /// @param[in] flag  the new flag cube to assign.
        /// @param[in] frequency the new frequency vector to assign.
        void resize(const casa::Cube<casa::Complex>& visibility,
                    const casa::Cube<casa::Bool>& flag,
                    const casa::Vector<casa::Double>& frequency);

        // Serializer functions

        /// @brief write the object to a blob stream
        /// @param[in] os the output stream
        virtual void writeToBlob(LOFAR::BlobOStream& os) const;

        /// @brief read the object from a blob stream
        /// @param[in] is the input stream
        virtual void readFromBlob(LOFAR::BlobIStream& is);

        /// @brief Shared pointer typedef
        typedef boost::shared_ptr<VisChunk> ShPtr;

    private:

        /// Number of rows
        casa::uInt itsNumberOfRows;

        /// Number of channels
        casa::uInt itsNumberOfChannels;

        /// Number of polarisations
        casa::uInt itsNumberOfPolarisations;

        /// Time
        casa::MVEpoch itsTime;

        /// Interval
        casa::Double itsInterval;

        /// Scan Index
        casa::uInt itsScan;

        /// Antenna1
        casa::Vector<casa::uInt> itsAntenna1;

        /// Antenna2
        casa::Vector<casa::uInt> itsAntenna2;

        /// Beam1
        casa::Vector<casa::uInt> itsBeam1;

        /// Beam2
        casa::Vector<casa::uInt> itsBeam2;

        /// Beam1 position angle
        casa::Vector<casa::Float> itsBeam1PA;

        /// Beam2 position angle
        casa::Vector<casa::Float> itsBeam2PA;

        /// Pointing direction of the first antenna/beam
        casa::Vector<casa::MVDirection> itsPointingDir1;

        /// Pointing direction of the second antenna/beam
        casa::Vector<casa::MVDirection> itsPointingDir2;

        /// Pointing direction of the centre of the first antenna
        casa::Vector<casa::MVDirection> itsDishPointing1;

        /// Pointing direction of the centre of the second antenna
        casa::Vector<casa::MVDirection> itsDishPointing2;

        /// Visibility
        casa::Cube<casa::Complex> itsVisibility;

        /// Flag
        casa::Cube<casa::Bool> itsFlag;

        /// UVW
        casa::Vector<casa::RigidVector<casa::Double, 3> > itsUVW;

        /// Frequency
        casa::Vector<casa::Double> itsFrequency;

        /// Channel Width
        casa::Double itsChannelWidth;

        /// Stokes
        casa::Vector<casa::Stokes::StokesTypes> itsStokes;

        /// Direction frame
        casa::MDirection::Ref itsDirectionFrame;
};

} // end of namespace common
} // end of namespace cp
} // end of namespace askap

#endif
