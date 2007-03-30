/// @file METableDataAccessor.h
///
/// METableDataAccessor:  class to access buffered visibility data from a 
///                        table.
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///
#ifndef IEQ_TABLE_DATA_ACCESSOR_H
#define IEQ_TABLE_DATA_ACCESSOR_H

#include "MEDataAccessor.h"

namespace conrad {
/// METableDataAccessor: one of possible implementations of the
/// class to access buffered visibility data.
/// stubbed behavior at this stage
class METableDataAccessor : public MEDataAccessor
{
     /// The number of rows in this chunk
     /// @return the number of rows in this chunk
     virtual casa::uInt nRow() const throw();

     // The following methods implement metadata access
		
     /// The number of spectral channels (equal for all rows)
     /// @return the number of spectral channels
     virtual casa::uInt nChannel() const throw();

     /// First antenna IDs for all rows
     /// @return a vector with IDs of the first antenna corresponding
     /// to each visibility (one for each row)
     virtual const casa::Vector<casa::uInt>& antenna1() const;

     /// Second antenna IDs for all rows
     /// @return a vector with IDs of the second antenna corresponding
     /// to each visibility (one for each row)
     virtual const casa::Vector<casa::uInt>& antenna2() const;
	
     /// First feed IDs for all rows
     /// @return a vector with IDs of the first feed corresponding
     /// to each visibility (one for each row)
     virtual const casa::Vector<casa::uInt>& feed1() const;
     
     /// Second feed IDs for all rows
     /// @return a vector with IDs of the second feed corresponding
     /// to each visibility (one for each row)
     virtual const casa::Vector<casa::uInt>& feed2() const;
     
     /// Position angles of the first feed for all rows
     /// @return a vector with position angles (in radians) of the
     /// first feed corresponding to each visibility
     virtual const casa::Vector<casa::Float>& feed1PA() const;
     
     /// Position angles of the second feed for all rows
     /// @return a vector with position angles (in radians) of the
     /// second feed corresponding to each visibility
     virtual const casa::Vector<casa::Float>& feed2PA() const;
     
     /// Return pointing centre directions of the first antenna/feed
     /// @return a vector with direction measures (coordinate system
     /// is determined by the data accessor), one direction for each
     /// visibility/row
     virtual const casa::Vector<casa::MVDirection>& pointingDir1() const;
     
     /// Pointing centre directions of the second antenna/feed
     /// @return a vector with direction measures (coordinate system
     /// is determined by the data accessor), one direction for each
     /// visibility/row
     virtual const casa::Vector<casa::MVDirection>& pointingDir2() const;
     
     /// Visibilities (a matrix is nRow x nChannel; each element is
     /// a stokes vector)
     /// @return a reference to nRow x nChannel matrix, containing
     /// all visibility data
     /// TODO:
     ///     a non-const version to be able to subtract the model
     virtual const casa::Matrix<casa::CStokesVector>& visibility() const;
     
     /// Matrix of flags
     /// @return a reference to nRow x nChannel matrix with flag information
     /// if True, the corresponding element is flagged.
     virtual const casa::Matrix<casa::Bool>& flag() const;
     
     /// UVW
     /// @return a reference to vector containing uvw-coordinates
     /// packed into a 3-D rigid vector
     virtual const casa::Vector<casa::RigidVector<casa::Double, 3> >&
             uvw() const;
     
     /// Noise level required for a proper weighting
     /// It is assumed at this stage that the same figure is valid for
     /// all spectral channels, although there could be a difference
     /// between different polarizations (hence CStokesVector)
     /// @return a vector (one element for each row) of CStokesVectors
     ///         with resulting noise figure for each polarization
     ///         (polarization conversion is taken into account)
     virtual const casa::Vector<casa::CStokesVector>& noise() const;
     
     /// Timestamp for each row
     /// @return a reference to vector containing timestamps for each
     ///         row (as Doubles, the frame/units are specified by the
     ///         DataSource object)
     virtual const casa::Vector<casa::Double>& time() const;
     
     /// Frequency for each channel
     /// @return a reference to vector containing frequencies for each
     ///         spectral channel (vector size is nChannel). Frequencies
     ///         are given as Doubles, the frame/units are specified by
     ///         the DataSource object
     virtual const casa::Vector<casa::Double>& frequency() const;
private:
     /// cached results which are filled from an appropriate table
     /// when necessary (they probably have to be moved to DataSource)
     mutable casa::Vector<casa::uInt> mAntenna1;
     mutable casa::Vector<casa::uInt> mAntenna2;
     mutable casa::Vector<casa::uInt> mFeed1;
     mutable casa::Vector<casa::uInt> mFeed2;
     mutable casa::Vector<casa::Float> mFeed1PA;
     mutable casa::Vector<casa::Float> mFeed2PA;
     mutable casa::Vector<casa::MVDirection> mPointingDir1;
     mutable casa::Vector<casa::MVDirection> mPointingDir2;
     mutable casa::Matrix<casa::CStokesVector> mVisibility;
     mutable casa::Matrix<casa::Bool> mFlag;
     mutable casa::Vector<casa::RigidVector<casa::Double, 3> > mUVW;
     mutable casa::Vector<casa::CStokesVector> mNoise;
     mutable casa::Vector<casa::Double> mTime;
     mutable casa::Vector<casa::Double> mFrequency;
};


} // namespace conrad

#endif // #ifndef IEQ_TABLE_DATA_ACCESSOR_H






