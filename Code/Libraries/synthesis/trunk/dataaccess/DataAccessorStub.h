/// @file DataAccessorStub.h
///
/// DataAccessorStub:  a stub to debug the code, which uses DataAccessor
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///
#ifndef DATA_ACCESSOR_STUB_H
#define DATA_ACCESSOR_STUB_H

#include "IFlagDataAccessor.h"

namespace conrad {
	

namespace synthesis {

/// A stubbed implementation of the data accessor
struct DataAccessorStub : public IFlagDataAccessor
{
	/// Default version can fill with MIRANdA data
	DataAccessorStub(const bool fill=false);
	
     /// The number of rows in this chunk
     /// @return the number of rows in this chunk
     virtual casa::uInt nRow() const throw();

     // The following methods implement metadata access
		
     /// The number of spectral channels (equal for all rows)
     /// @return the number of spectral channels
     virtual casa::uInt nChannel() const throw();

     /// The number of polarization products (equal for all rows)
     /// @return the number of polarization products (can be 1,2 or 4)
     virtual casa::uInt nPol() const throw();


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

     /// Visibilities (a cube is nRow x nChannel x nPol; each element is
     /// a complex visibility)
     /// @return a reference to nRow x nChannel x nPol cube, containing
     /// all visibility data
     /// TODO:
     ///     a non-const version to be able to subtract the model
     virtual const casa::Cube<casa::Complex>& visibility() const;

     /// Read-write visibilities (a cube is nRow x nChannel x nPol; 
     /// each element is a complex visibility)
     /// 
     /// @return a reference to nRow x nChannel x nPol cube, containing
     /// all visibility data
     ///
     virtual casa::Cube<casa::Complex>& rwVisibility();


     /// Cube of flags corresponding to the output of visibility() 
     /// @return a reference to nRow x nChannel x nPol cube with flag 
     ///         information. If True, the corresponding element is flagged.
     virtual const casa::Cube<casa::Bool>& flag() const;

     /// Non-const access to the cube of flags.
     /// @return a reference to nRow x nChannel x nPol cube with the flag
     ///         information. If True, the corresponding element is flagged.
     virtual casa::Cube<casa::Bool>& rwFlag();

     
     /// Noise level required for a proper weighting
     /// @return a reference to nRow x nChannel x nPol cube with
     ///         complex noise estimates
     virtual const casa::Cube<casa::Complex>& noise() const;

     /// UVW
     /// @return a reference to vector containing uvw-coordinates
     /// packed into a 3-D rigid vector
     virtual const casa::Vector<casa::RigidVector<casa::Double, 3> >&
             uvw() const;
         
     /// Timestamp for each row
     /// @return a reference to vector containing timestamps for each
     ///         row (as Double, the frame/origin/units are specified by the
     ///         DataSource object)
     virtual const casa::Vector<casa::Double>& time() const;
     
     /// Frequency for each channel
     /// @return a reference to vector containing frequencies for each
     ///         spectral channel (vector size is nChannel). Frequencies
     ///         are given as Doubles, the frame/units are specified by
     ///         the DataSource object
     virtual const casa::Vector<casa::Double>& frequency() const;

     /// Velocity for each channel
     /// @return a reference to vector containing velocities for each
     ///         spectral channel (vector size is nChannel). Velocities
     ///         are given as Doubles, the frame/units are specified by
     ///         the DataSource object (via IDataConverter).
     virtual const casa::Vector<casa::Double>& velocity() const;

     
//private: // to be able to change stubbed data directly, if necessary
     /// cached results which are filled from an appropriate table
     /// when necessary (they probably have to be moved to DataSource)
     mutable casa::Vector<casa::uInt> itsAntenna1;
     mutable casa::Vector<casa::uInt> itsAntenna2;
     mutable casa::Vector<casa::uInt> itsFeed1;
     mutable casa::Vector<casa::uInt> itsFeed2;
     mutable casa::Vector<casa::Float> itsFeed1PA;
     mutable casa::Vector<casa::Float> itsFeed2PA;
     mutable casa::Vector<casa::MVDirection> itsPointingDir1;
     mutable casa::Vector<casa::MVDirection> itsPointingDir2;
     mutable casa::Cube<casa::Complex> itsVisibility;
     mutable casa::Cube<casa::Bool> itsFlag;
     mutable casa::Vector<casa::RigidVector<casa::Double, 3> > itsUVW;
     mutable casa::Cube<casa::Complex> itsNoise;
     mutable casa::Vector<casa::Double> itsTime;
     mutable casa::Vector<casa::Double> itsFrequency;
     mutable casa::Vector<casa::Double> itsVelocity;
};


} // namespace synthesis

} // namespace conrad

#endif // #ifndef DATA_ACCESSOR_STUB_H






