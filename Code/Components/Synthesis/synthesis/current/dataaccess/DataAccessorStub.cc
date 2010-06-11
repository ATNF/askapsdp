/// @file DataAccessorStub.cc
/// @brief a stub to debug the code, which uses DataAccessor
///
/// @copyright (c) 2007 CSIRO
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
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

// STD includes
#include <vector>

// CASA includes
#include <casa/aips.h>
#include <casa/Arrays/Matrix.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MDirection.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/MVPosition.h>
#include <casa/Quanta/MVDirection.h>

// own includes
#include <dataaccess/DataAccessorStub.h>
#include <dataaccess/DataAccessError.h>

using std::vector;

using namespace casa;

namespace askap
{

  namespace synthesis
  {

    DataAccessorStub::DataAccessorStub(const bool fill)
    {
      if (fill)
      {
        /// Antenna locations for MIRANdA 30: Long, Lat, X, Y
        uint nAnt(30);
        double antloc[30][4]=
          {
            { 117.49202058, -26.605799881, 100.958023, 559.849243 },
            { 117.494256673, -26.6060206276, 324.567261, 584.537170 },
            { 117.492138828, -26.6048061779, 112.782784, 448.715179 },
            { 117.490509466, -26.6064154537, -50.153362, 628.693848 },
            { 117.491968542, -26.6032064197, 95.754150, 269.800934 },
            { 117.490679338, -26.6041085045, -33.166206, 370.688568 },
            { 117.493698582, -26.5967173819, 268.758209, -455.922058 },
            { 117.494563491, -26.5982339642, 355.249115, -286.310059 },
            { 117.494296106, -26.5999215991, 328.510620, -97.567841 },
            { 117.496887152, -26.5996276291, 587.615234, -130.444946 },
            { 117.495216406, -26.6020274124, 420.540619, 137.942749 },
            { 117.497394175, -26.6012778782, 638.317505, 54.116112 },
            { 117.495892042, -26.6031307413, 488.104187, 261.337189 },
            { 117.498414438, -26.6041107522, 740.343750, 370.939941 },
            { 117.500240665, -26.6055840564, 922.966492, 535.711792 },
            { 117.500137190, -26.6059345560, 912.619019, 574.911072 },
            { 117.499183013, -26.6062126479, 817.201294, 606.012390 },
            { 117.496685358, -26.6061741003, 567.435791, 601.701294 },
            { 117.496086521, -26.6064049437, 507.552063, 627.518433 },
            { 117.495766946, -26.6057995355, 475.594604, 559.810608 },
            { 117.491797669, -26.6098746485, 78.666885, 1015.564331 },
            { 117.489620509, -26.6104123521, -139.049057, 1075.700195 },
            { 117.489067099, -26.6064599406, -194.390121, 633.669189 },
            { 117.483440786, -26.6089367492, -757.021423, 910.671265 },
            { 117.483634163, -26.6069021547, -737.683655, 683.125671 },
            { 117.484600363, -26.6042107834, -641.063660, 382.127258 },
            { 117.485728514, -26.6027311538, -528.248596, 216.647995 },
            { 117.484634236, -26.5990365257, -637.676392, -196.552948 },
            { 117.488195463, -26.5986065217, -281.553711, -244.643860 },
            { 117.488435693, -26.5949733943, -257.530670, -650.966675 } };

        vector<casa::MPosition> mPos;
        mPos.resize(nAnt);
        for (uint iant1=0; iant1<nAnt; iant1++)
        {
          mPos[iant1]=casa::MPosition(casa::MVPosition(casa::Quantity(6400000,
              "m"), casa::Quantity(antloc[iant1][0], "deg"), casa::Quantity(
              antloc[iant1][1], "deg")), casa::MPosition::WGS84);
        }

        uint nRows(nAnt*(nAnt-1)/2);
        uint nChan(8);
        this->itsFrequency.resize(nChan);
        for (uint chan=0; chan<nChan; chan++)
          this->itsFrequency(chan)=1.400e9-20e6*chan;
        uint nPol(1);
        itsVisibility.resize(nRows, nChan, nPol);
        itsVisibility.set(casa::Complex(0.0, 0.0));
        itsFlag.resize(nRows, nChan, nPol);
        itsFlag.set(casa::False);
        itsTime=0;
        itsUVW.resize(nRows);
        itsUVWRotationDelay.resize(nRows);
        itsUVWRotationDelay = 0.;
        itsAntenna1.resize(nRows);
        itsAntenna2.resize(nRows);
        itsFeed1.resize(nRows);
        itsFeed2.resize(nRows);
        itsPointingDir1.resize(nRows);
        itsPointingDir2.resize(nRows);
        itsDishPointing1.resize(nRows);
        itsDishPointing2.resize(nRows);
        uint row=0;
        for (uint iant1=0; iant1<nAnt; iant1++)
        {
          for (uint iant2=iant1+1; iant2<nAnt; iant2++)
          {
            ASKAPDEBUGASSERT(row<nRows);
            itsAntenna1(row)=iant1;
            itsAntenna2(row)=iant2;
            itsFeed1(row)=0;
            itsFeed2(row)=0;
            itsPointingDir1(row)=casa::MVDirection(casa::Quantity(0, "deg"), casa::Quantity(0, "deg"));
            itsPointingDir2(row)=casa::MVDirection(casa::Quantity(0, "deg"), casa::Quantity(0, "deg"));
            itsDishPointing1(row)=casa::MVDirection(casa::Quantity(0, "deg"), casa::Quantity(0, "deg"));
            itsDishPointing2(row)=casa::MVDirection(casa::Quantity(0, "deg"), casa::Quantity(0, "deg"));
            itsUVW(row)=casa::RigidVector<casa::Double, 3>(0.0, 0.0, 0.0);
            for (uint dim=0;dim<3;dim++) {
                 itsUVW(row)(dim)=mPos[iant1].get("m").getValue()(dim)-mPos[iant2].get("m").getValue()(dim);
            }
            row++;
          }
        }
        itsStokes.resize(nPol);
        itsStokes[0] = casa::Stokes::I;
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
                
                /// pointing direction for the centre of the first antenna 
                /// @details The same as pointingDir1, if the feed offsets are zero
                /// @return a vector with direction measures (coordinate system
                /// is is set via IDataConverter), one direction for each
                /// visibility/row
                const casa::Vector<casa::MVDirection>& DataAccessorStub::dishPointing1() const
                {
                  return itsDishPointing1;
                }

                /// pointing direction for the centre of the first antenna 
                /// @details The same as pointingDir2, if the feed offsets are zero
                /// @return a vector with direction measures (coordinate system
                /// is is set via IDataConverter), one direction for each
                /// visibility/row
                const casa::Vector<casa::MVDirection>& DataAccessorStub::dishPointing2() const
                {
                  return itsDishPointing2;
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
		          ASKAPASSERT(itsFlag.shape() == itsVisibility.shape());
                  return itsFlag;
                }

                /// Non-const access to the cube of flags.
                /// @return a reference to nRow x nChannel x nPol cube with the flag
                ///         information. If True, the corresponding element is flagged.
                casa::Cube<casa::Bool>& DataAccessorStub::rwFlag()
                {
                  throw DataAccessLogicError("Not yet implemented");
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
                
                /// @brief uvw after rotation
                /// @details This method calls UVWMachine to rotate baseline coordinates 
                /// for a new tangent point. Delays corresponding to this correction are
                /// returned by a separate method.
                /// @param[in] tangentPoint tangent point to rotate the coordinates to
                /// @return uvw after rotation to the new coordinate system for each row
                const casa::Vector<casa::RigidVector<casa::Double, 3> >&
	            DataAccessorStub::rotatedUVW(const casa::MDirection &tangentPoint) const
	            {
	              return itsUVW;
	            }
	         
                /// @brief delay associated with uvw rotation
                /// @details This is a companion method to rotatedUVW. It returns delays corresponding
                /// to the baseline coordinate rotation. An additional delay corresponding to the 
                /// translation in the tangent plane can also be applied using the image 
                /// centre parameter. Set it to tangent point to apply no extra translation.
                /// @param[in] tangentPoint tangent point to rotate the coordinates to
                /// @param[in] imageCentre image centre (additional translation is done if imageCentre!=tangentPoint)
                /// @return delays corresponding to the uvw rotation for each row
                const casa::Vector<casa::Double>& DataAccessorStub::uvwRotationDelay(
	               const casa::MDirection &tangentPoint, const casa::MDirection &imageCentre) const
	            {
	              return itsUVWRotationDelay;
	            }
                

                /// Noise level required for a proper weighting
                /// @return a reference to nRow x nChannel x nPol cube with
                ///         complex noise estimates
                const casa::Cube<casa::Complex>& DataAccessorStub::noise() const
                {
                  return itsNoise;
                }

                /// @return a timestamp for this buffer (it is always the same
                ///         for all rows. The timestamp is returned as 
                ///         Double w.r.t. the origin specified by the 
                ///         DataSource object and in that reference frame
                casa::Double DataAccessorStub::time() const
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
                
                /// @brief polarisation type for each product
                /// @return a reference to vector containing polarisation types for
                /// each product in the visibility cube (nPol() elements).
                /// @note All rows of the accessor have the same structure of the visibility
                /// cube, i.e. polarisation types returned by this method are valid for all rows.
                const casa::Vector<casa::Stokes::StokesTypes>& DataAccessorStub::stokes() const
                {
                  return itsStokes;
                }

              } // namespace synthesis

            } // namespace askap
