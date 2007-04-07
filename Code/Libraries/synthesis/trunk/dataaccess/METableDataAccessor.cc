/// @file METableDataAccessor.cc
///
/// METableDataAccessor:  class to access buffered visibility data from a 
///                        table.
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#include "METableDataAccessor.h"

using namespace conrad;
using namespace casa;
using namespace synthesis;

/// The number of rows in this chunk
/// @return the number of rows in this chunk
casa::uInt METableDataAccessor::nRow() const throw()
{
  return mAntenna1.nelements();
}

// The following methods implement metadata access
   	
/// The number of spectral channels (equal for all rows)
/// @return the number of spectral channels
casa::uInt METableDataAccessor::nChannel() const throw()
{
  return mFrequency.nelements();
}

/// First antenna IDs for all rows
/// @return a vector with IDs of the first antenna corresponding
/// to each visibility (one for each row)
const casa::Vector<casa::uInt>& METableDataAccessor::antenna1() const
{
  return mAntenna1;
}

/// Second antenna IDs for all rows
/// @return a vector with IDs of the second antenna corresponding
/// to each visibility (one for each row)
const casa::Vector<casa::uInt>& METableDataAccessor::antenna2() const
{
  return mAntenna2;
}
   
/// First feed IDs for all rows
/// @return a vector with IDs of the first feed corresponding
/// to each visibility (one for each row)
const casa::Vector<casa::uInt>& METableDataAccessor::feed1() const
{
  return mFeed1;
}

/// Second feed IDs for all rows
/// @return a vector with IDs of the second feed corresponding
/// to each visibility (one for each row)
const casa::Vector<casa::uInt>& METableDataAccessor::feed2() const
{
  return mFeed2;
}

/// Position angles of the first feed for all rows
/// @return a vector with position angles (in radians) of the
/// first feed corresponding to each visibility
const casa::Vector<casa::Float>& METableDataAccessor::feed1PA() const
{
  return mFeed1PA;
}

/// Position angles of the second feed for all rows
/// @return a vector with position angles (in radians) of the
/// second feed corresponding to each visibility
const casa::Vector<casa::Float>& METableDataAccessor::feed2PA() const
{
  return mFeed2PA;
}

/// Return pointing centre directions of the first antenna/feed
/// @return a vector with direction measures (coordinate system
/// is determined by the data accessor), one direction for each
/// visibility/row
const casa::Vector<casa::MVDirection>& METableDataAccessor::pointingDir1() const
{
  return mPointingDir1;
}

/// Pointing centre directions of the second antenna/feed
/// @return a vector with direction measures (coordinate system
/// is determined by the data accessor), one direction for each
/// visibility/row
const casa::Vector<casa::MVDirection>& METableDataAccessor::pointingDir2() const
{
  return mPointingDir2;
}

/// Visibilities (a matrix is nRow x nChannel; each element is
/// a stokes vector)
/// @return a reference to nRow x nChannel matrix, containing
/// all visibility data
/// TODO:
///     a non-const version to be able to subtract the model
const casa::Matrix<casa::CStokesVector>& METableDataAccessor::visibility() const
{
  return mVisibility;
}

/// Matrix of flags
/// @return a reference to nRow x nChannel matrix with flag information
/// if True, the corresponding element is flagged.
const casa::Matrix<casa::Bool>& METableDataAccessor::flag() const
{
  return mFlag;
}

/// UVW
/// @return a reference to vector containing uvw-coordinates
/// packed into a 3-D rigid vector
const casa::Vector<casa::RigidVector<casa::Double, 3> >&
             METableDataAccessor::uvw() const
{
  return mUVW;
}

/// Noise level required for a proper weighting
/// It is assumed at this stage that the same figure is valid for
/// all spectral channels, although there could be a difference
/// between different polarizations (hence CStokesVector)
/// @return a vector (one element for each row) of CStokesVectors
///         with resulting noise figure for each polarization
///         (polarization conversion is taken into account)
const casa::Vector<casa::CStokesVector>& METableDataAccessor::noise() const
{
  return mNoise;
}

/// Timestamp for each row
/// @return a reference to vector containing timestamps for each
///         row (as Double, the frame/origin/units are specified by the
///         DataSource object)
const casa::Vector<casa::Double>& METableDataAccessor::time() const
{
  return mTime;
}

/// Frequency for each channel
/// @return a reference to vector containing frequencies for each
///         spectral channel (vector size is nChannel). Frequencies
///         are given as Doubles, the frame/units are specified by
///         the DataSource object
const casa::Vector<casa::Double>& METableDataAccessor::frequency() const
{
  return mFrequency;
}
