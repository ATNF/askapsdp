/// @file DataAccessorStub.h
///
/// DataAccessorStub:  a stub to debug the code, which uses DataAccessor
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#include "DataAccessorStub.h"

using namespace conrad;
using namespace casa;
using namespace synthesis;

/// The number of rows in this chunk
/// @return the number of rows in this chunk
casa::uInt DataAccessorStub::nRow() const throw()
{
  return itsVisibility.nrow();
}

// The following methods implement metadata access
   	
/// The number of spectral channels (equal for all rows)
/// @return the number of spectral channels
casa::uInt DataAccessorStub::nChannel() const throw()
{
  return itsVisibility.ncolumn();
}

/// The number of polarization products (equal for all rows)
/// @return the number of polarization products (can be 1,2 or 4)
casa::uInt DataAccessorStub::nPol() const throw()
{
 return itsVisibility.nplane();
}


/// First antenna IDs for all rows
/// @return a vector with IDs of the first antenna corresponding
/// to each visibility (one for each row)
const casa::Vector<casa::uInt>& DataAccessorStub::antenna1() const
{
  return itsAntenna1;
}

/// Second antenna IDs for all rows
/// @return a vector with IDs of the second antenna corresponding
/// to each visibility (one for each row)
const casa::Vector<casa::uInt>& DataAccessorStub::antenna2() const
{
  return itsAntenna2;
}
   
/// First feed IDs for all rows
/// @return a vector with IDs of the first feed corresponding
/// to each visibility (one for each row)
const casa::Vector<casa::uInt>& DataAccessorStub::feed1() const
{
  return itsFeed1;
}

/// Second feed IDs for all rows
/// @return a vector with IDs of the second feed corresponding
/// to each visibility (one for each row)
const casa::Vector<casa::uInt>& DataAccessorStub::feed2() const
{
  return itsFeed2;
}

/// Position angles of the first feed for all rows
/// @return a vector with position angles (in radians) of the
/// first feed corresponding to each visibility
const casa::Vector<casa::Float>& DataAccessorStub::feed1PA() const
{
  return itsFeed1PA;
}

/// Position angles of the second feed for all rows
/// @return a vector with position angles (in radians) of the
/// second feed corresponding to each visibility
const casa::Vector<casa::Float>& DataAccessorStub::feed2PA() const
{
  return itsFeed2PA;
}

/// Return pointing centre directions of the first antenna/feed
/// @return a vector with direction measures (coordinate system
/// is determined by the data accessor), one direction for each
/// visibility/row
const casa::Vector<casa::MVDirection>& DataAccessorStub::pointingDir1() const
{
  return itsPointingDir1;
}

/// Pointing centre directions of the second antenna/feed
/// @return a vector with direction measures (coordinate system
/// is determined by the data accessor), one direction for each
/// visibility/row
const casa::Vector<casa::MVDirection>& DataAccessorStub::pointingDir2() const
{
  return itsPointingDir2;
}

/// Visibilities (a cube is nRow x nChannel x nPol; each element is
/// a complex visibility)
/// @return a reference to nRow x nChannel x nPol cube, containing
/// all visibility data
/// TODO:
///     a non-const version to be able to subtract the model
const casa::Cube<casa::Complex>& DataAccessorStub::visibility() const
{
  return itsVisibility;
}

/// Read-write visibilities (a cube is nRow x nChannel x nPol; 
/// each element is a complex visibility)
/// 
/// @return a reference to nRow x nChannel x nPol cube, containing
/// all visibility data
///
casa::Cube<casa::Complex>& DataAccessorStub::rwVisibility()
{
  return itsVisibility;
}

/// Cube of flags corresponding to the output of visibility() 
/// @return a reference to nRow x nChannel x nPol cube with flag 
///         information. If True, the corresponding element is flagged.
const casa::Cube<casa::Bool>& DataAccessorStub::flag() const
{
  return itsFlag;
}

/// Non-const access to the cube of flags.
/// @return a reference to nRow x nChannel x nPol cube with the flag
///         information. If True, the corresponding element is flagged.
casa::Cube<casa::Bool>& DataAccessorStub::rwFlag()
{
  return itsFlag;
}

/// UVW
/// @return a reference to vector containing uvw-coordinates
/// packed into a 3-D rigid vector
const casa::Vector<casa::RigidVector<casa::Double, 3> >&
             DataAccessorStub::uvw() const
{
  return itsUVW;
}

/// Noise level required for a proper weighting
/// @return a reference to nRow x nChannel x nPol cube with
///         complex noise estimates
const casa::Cube<casa::Complex>& DataAccessorStub::noise() const
{
  return itsNoise;
}

/// Timestamp for each row
/// @return a reference to vector containing timestamps for each
///         row (as Double, the frame/origin/units are specified by the
///         DataSource object)
const casa::Vector<casa::Double>& DataAccessorStub::time() const
{
  return itsTime;
}

/// Frequency for each channel
/// @return a reference to vector containing frequencies for each
///         spectral channel (vector size is nChannel). Frequencies
///         are given as Doubles, the frame/units are specified by
///         the DataSource object
const casa::Vector<casa::Double>& DataAccessorStub::frequency() const
{
  return itsFrequency;
}


/// Velocity for each channel
/// @return a reference to vector containing velocities for each
///         spectral channel (vector size is nChannel). Velocities
///         are given as Doubles, the frame/units are specified by
///         the DataSource object (via IDataConverter).
const casa::Vector<casa::Double>& DataAccessorStub::velocity() const
{
  return itsVelocity;
}
