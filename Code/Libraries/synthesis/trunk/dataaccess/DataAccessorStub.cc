/// @file DataAccessorStub.h
///
/// DataAccessorStub:  a stub to debug the code, which uses DataAccessor
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

// STD includes
#include <vector>

// CASA includes
#include <casa/aips.h>
#include <casa/Arrays/Matrix.h>
#include <measures/Measures/MPosition.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/MVPosition.h>

// own includes
#include <dataaccess/DataAccessorStub.h>

using std::vector;


using namespace conrad;
using namespace casa;
using namespace synthesis;

/// apparently this template is not compiled in to the casacore libraries,
/// manual instantiation is required
//template Vector<MVDirection>;

DataAccessorStub::DataAccessorStub(const bool fill)
{
	if(fill) {
	    	/// Antenna locations for MIRANdA 30: Long, Lat, X, Y
	    uint nAnt(30);
		double antloc[30][4]={{117.49202058,-26.605799881,100.958023,559.849243},
			{117.494256673,-26.6060206276,324.567261,584.537170},
			{117.492138828,-26.6048061779,112.782784,448.715179},
			{117.490509466,-26.6064154537,-50.153362,628.693848},
			{117.491968542,-26.6032064197,95.754150,269.800934},
			{117.490679338,-26.6041085045,-33.166206,370.688568},
			{117.493698582,-26.5967173819,268.758209,-455.922058},
			{117.494563491,-26.5982339642,355.249115,-286.310059},
			{117.494296106,-26.5999215991,328.510620,-97.567841},
			{117.496887152,-26.5996276291,587.615234,-130.444946},
			{117.495216406,-26.6020274124,420.540619,137.942749},
			{117.497394175,-26.6012778782,638.317505,54.116112},
			{117.495892042,-26.6031307413,488.104187,261.337189},
			{117.498414438,-26.6041107522,740.343750,370.939941},
			{117.500240665,-26.6055840564,922.966492,535.711792},
			{117.500137190,-26.6059345560,912.619019,574.911072},
			{117.499183013,-26.6062126479,817.201294,606.012390},
			{117.496685358,-26.6061741003,567.435791,601.701294},
			{117.496086521,-26.6064049437,507.552063,627.518433},
			{117.495766946,-26.6057995355,475.594604,559.810608},
			{117.491797669,-26.6098746485,78.666885,1015.564331},
			{117.489620509,-26.6104123521,-139.049057,1075.700195},
			{117.489067099,-26.6064599406,-194.390121,633.669189},
			{117.483440786,-26.6089367492,-757.021423,910.671265},
			{117.483634163,-26.6069021547,-737.683655,683.125671},
			{117.484600363,-26.6042107834,-641.063660,382.127258},
			{117.485728514,-26.6027311538,-528.248596,216.647995},
			{117.484634236,-26.5990365257,-637.676392,-196.552948},
			{117.488195463,-26.5986065217,-281.553711,-244.643860},
			{117.488435693,-26.5949733943,-257.530670,-650.966675}};
		
		vector<casa::MPosition> mPos;
		mPos.resize(nAnt);
		for (uint iant1=0;iant1<nAnt;iant1++) {
			mPos[iant1]=casa::MPosition(casa::MVPosition(casa::Quantity(6400000, "m"), 
											casa::Quantity(antloc[iant1][0], "deg"),
											casa::Quantity(antloc[iant1][1],"deg")),
											casa::MPosition::WGS84);
		}

      uint nRows(nAnt*(nAnt-1)/2);
      uint nChan(8);
      this->itsFrequency.resize(nChan);
      for (uint chan=0;chan<nChan;chan++) this->itsFrequency(chan)=1.400e9-20e6*chan;
      uint nPol(1);
      this->itsVisibility.resize(nRows, nChan, nPol);
      this->itsVisibility.set(casa::Complex(0.0, 0.0));
      this->itsTime.resize(nRows);
      this->itsUVW.resize(nRows);
      this->itsAntenna1.resize(nRows);
      this->itsAntenna2.resize(nRows);
      this->itsFeed1.resize(nRows);
      this->itsFeed2.resize(nRows);
      uint row=0;
      for (uint iant1=0;iant1<nAnt;iant1++) {
	      for (uint iant2=iant1+1;iant2<nAnt;iant2++) {
		      this->itsAntenna1(row)=iant1;
		      this->itsAntenna2(row)=iant2;
		      this->itsFeed1(row)=0;
		      this->itsFeed2(row)=0;
		      this->itsTime(row)=0.0;
			  this->itsUVW(row)=casa::RigidVector<casa::Double, 3>(0.0, 0.0, 0.0);
			  for (uint dim=0;dim<3;dim++) this->itsUVW(row)(dim)=mPos[iant1].get("m").getValue()(dim)-mPos[iant2].get("m").getValue()(dim);
			  row++;
	      }
      }
	}
}

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
