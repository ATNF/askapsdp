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
#ifndef ASKAP_CP_VISCHUNK_H
#define ASKAP_CP_VISCHUNK_H

// ASKAPsoft includes
#include "casa/aips.h"
#include "casa/Arrays/Vector.h"
#include "casa/Arrays/Matrix.h"
#include "casa/Arrays/Cube.h"
#include "casa/Quanta/MVDirection.h"
#include "measures/Measures/MDirection.h"
#include "scimath/Mathematics/RigidVector.h"
#include "measures/Measures/Stokes.h"
#include "boost/shared_ptr.hpp"

namespace askap {

    namespace cp {

        class VisChunk
        {
            public:
                /// Constructor
                VisChunk();

                /// Destructor
                ~VisChunk();

                /// The number of rows in this chunk
                /// @return the number of rows in this chunk
                casa::uInt& nRow();

                // The following methods implement metadata access

                /// The number of spectral channels (equal for all rows)
                /// @return the number of spectral channels
                casa::uInt& nChannel();

                /// The number of polarization products (equal for all rows)
                /// @return the number of polarization products (can be 1,2 or 4)
                casa::uInt& nPol();

                /// First antenna IDs for all rows
                /// @return a vector with IDs of the first antenna corresponding
                /// to each visibility (one for each row)
                casa::Vector<casa::uInt>& antenna1();

                /// Second antenna IDs for all rows
                /// @return a vector with IDs of the second antenna corresponding
                /// to each visibility (one for each row)
                casa::Vector<casa::uInt>& antenna2();

                /// First feed IDs for all rows
                /// @return a vector with IDs of the first feed corresponding
                /// to each visibility (one for each row)
                casa::Vector<casa::uInt>& feed1();

                /// Second feed IDs for all rows
                /// @return a vector with IDs of the second feed corresponding
                /// to each visibility (one for each row)
                casa::Vector<casa::uInt>& feed2();

                /// Position angles of the first feed for all rows
                /// @return a vector with position angles (in radians) of the
                /// first feed corresponding to each visibility
                casa::Vector<casa::Float>& feed1PA();

                /// Position angles of the second feed for all rows
                /// @return a vector with position angles (in radians) of the
                /// second feed corresponding to each visibility
                casa::Vector<casa::Float>& feed2PA();

                /// Return pointing centre directions of the first antenna/feed
                /// @return a vector with direction measures (coordinate system
                /// is set via IDataConverter), one direction for each
                /// visibility/row
                casa::Vector<casa::MVDirection>& pointingDir1();

                /// Pointing centre directions of the second antenna/feed
                /// @return a vector with direction measures (coordinate system
                /// is is set via IDataConverter), one direction for each
                /// visibility/row
                casa::Vector<casa::MVDirection>& pointingDir2();

                /// pointing direction for the centre of the first antenna 
                /// @details The same as pointingDir1, if the feed offsets are zero
                /// @return a vector with direction measures (coordinate system
                /// is is set via IDataConverter), one direction for each
                /// visibility/row
                casa::Vector<casa::MVDirection>& dishPointing1();

                /// pointing direction for the centre of the first antenna 
                /// @details The same as pointingDir2, if the feed offsets are zero
                /// @return a vector with direction measures (coordinate system
                /// is is set via IDataConverter), one direction for each
                /// visibility/row
                casa::Vector<casa::MVDirection>& dishPointing2();

                /// VisChunk (a cube is nRow x nChannel x nPol; each element is
                /// a complex visibility)
                /// @return a reference to nRow x nChannel x nPol cube, containing
                /// all visibility data
                casa::Cube<casa::Complex>& visibility();

                /// Cube of flags corresponding to the output of visibility() 
                /// @return a reference to nRow x nChannel x nPol cube with flag 
                ///         information. If True, the corresponding element is flagged.
                casa::Cube<casa::Bool>& flag();

                /// UVW
                /// @return a reference to vector containing uvw-coordinates
                /// packed into a 3-D rigid vector
                casa::Vector<casa::RigidVector<casa::Double, 3> >& uvw();

                /// Timestamp for each row
                /// @return a timestamp for this buffer (it is always the same
                ///         for all rows. 
                casa::Double time();

                /// Frequency for each channel
                /// @return a reference to vector containing frequencies for each
                ///         spectral channel (vector size is nChannel).
                casa::Vector<casa::Double>& frequency();

                /// @brief polarisation type for each product
                /// @return a reference to vector containing polarisation types for
                /// each product in the visibility cube (nPol() elements).
                /// @note All rows of the accessor have the same structure of the visibility
                /// cube, i.e. polarisation types returned by this method are valid for all rows.
                casa::Vector<casa::Stokes::StokesTypes>& stokes();

                /// @brief Shared pointer typedef
                typedef boost::shared_ptr<VisChunk> ShPtr;

            private:

                /// Number of rows
                casa::uInt itsNumberOfRows;

                /// Number of channels
                casa::uInt itsNumberOfChannels;

                /// Number of polarisations
                casa::uInt itsNumberOfPolarisations;

                /// Antenna1
                casa::Vector<casa::uInt> itsAntenna1;
                /// Antenna2
                casa::Vector<casa::uInt> itsAntenna2;

                /// Feed1
                casa::Vector<casa::uInt> itsFeed1;
                /// Feed2
                casa::Vector<casa::uInt> itsFeed2;

                /// Feed1 position angle
                casa::Vector<casa::Float> itsFeed1PA;
                /// Feed2 position angle
                casa::Vector<casa::Float> itsFeed2PA;

                /// Pointing direction of the first antenna/feed
                casa::Vector<casa::MVDirection> itsPointingDir1;
                /// Pointing direction of the second antenna/feed
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

                /// Time
                casa::Double itsTime;

                /// Frequency
                casa::Vector<casa::Double> itsFrequency;

                /// Stokes
                casa::Vector<casa::Stokes::StokesTypes> itsStokes;
        };

    } // end of namespace cp

} // end of namespace askap

#endif
