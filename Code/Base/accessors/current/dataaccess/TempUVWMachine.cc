/// @file
/// @brief temporary replacement for UVWMachine
/// @details We suspect a bug in casacore's UVWMachine. This class is used
/// for debugging. Only uvw machine methods we're using are implemented. In the 
/// future, we will probably revert back to casacore's UVWMachine when it is fixed.
/// The code is heavily based on casacore's UVWMachine code.
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

#include <dataaccess/TempUVWMachine.h>
#include <casa/Quanta/Euler.h>

namespace askap {

namespace accessors {

/// @brief construct the machine
/// @details
/// @param[in] in input direction
/// @param[in] out output direction
/// @note in and out are swapped w.r.t. casa UVWMachine as this is how it is used in
/// the current code
TempUVWMachine::TempUVWMachine(const casa::MDirection &in, const casa::MDirection &out, const bool, const bool) : itsIn(in), itsOut(out)
{
  const casa::MDirection::Ref outref = out.getRef();
  itsConv = casa::MDirection::Convert(in, outref);
  init();
}

/// @brief convert uvw
/// @param[in] delay reference to delay buffer
/// @param[in] uvw reference to uvw vector to update
void TempUVWMachine::convertUVW(casa::Double &delay, casa::Vector<casa::Double> &uvw) const
{
  casa::MVPosition tmp = itsUVWRotation * casa::MVPosition(uvw);
  delay = itsPhaseRotation * tmp;
  // reprojection comes here
  // tmp *= itsProjRotation;
  uvw = tmp.getValue();
}

/// @brief convert uvw
/// @param[in] uvw reference to uvw vector to update
void TempUVWMachine::convertUVW(casa::Vector<casa::Double> &uvw) const
{
  casa::Double delay;
  convertUVW(delay, uvw);
}


/// @brief initialise transform matrices
void TempUVWMachine::init()
{
  // The first rotation is from the uvw coordinate system corresponding to the input 
  // frame (pole towards in-direction and X-axis west) into the standard XYZ frame.
  // This rotation is composed of rotation around x-axis (90-lat) followed by
  // rotation around z-axis over (90-long).
  //const casa::RotMatrix rot1(casa::Euler(casa::C::pi_2 - itsIn.getValue().get()(1), 1,
  //        casa::C::pi_2 - itsIn.getValue().get()(0), 3));
  const casa::RotMatrix rot1(casa::Euler(casa::C::pi_2 - itsIn.getValue().get()(0), 3,
            casa::C::pi_2 - itsIn.getValue().get()(1), 1));
  // define axes
  const casa::MVDirection mVz(0.,0.,1.);
  const casa::MVDirection mVy(0.,1.,0.);
  const casa::MVDirection mVx(1.,0.,0.);
  // obtain rotation matrix from the old to the new reference frame
  casa::RotMatrix rot2;
  rot2.set(itsConv(mVx).getValue().getValue(),
           itsConv(mVy).getValue().getValue(),
           itsConv(mVz).getValue().getValue());           
  rot2.transpose(); // RotMatrix::set fills rows with the given vectors, we need columns.
  // (assuming transformation between two frames is orthogonal, this would express new basis via the old one
  // as for other rotX matrices).
  
  
  // The final rotation is from the standard XYZ frame into the uvw coordinate system
  // corresponding to the output frame (pole towards out-direction)
  //const casa::RotMatrix rot3(casa::Euler(-casa::C::pi_2 + itsOut.getValue().get()(0), 3,
  //         itsOut.getValue().get()(1) - casa::C::pi_2, 1));
  const casa::RotMatrix rot3(casa::Euler(itsOut.getValue().get()(1) - casa::C::pi_2, 1,
              -casa::C::pi_2 + itsOut.getValue().get()(0), 3));
  // reprojection will come here
  // the order of multiplication is reversed in the following statement to account for the fact that
  // rotX matrices express the new basis via the old one, i.e. instead of right-multiplying by the matrix
  // expressing the old basis via the new one we left-multiply by the reverse transform. The reverse 
  // amounts to transposition for orthogonal transformations such as rotations, but transpose requires the
  // order to be reversed.
  itsUVWRotation = rot1 * rot2 * rot3;
  // to compute associated delay change we need to convert the direction increment vector into the 
  // target uvw frame (i.e. elements become l,m,n instead of dX, dY and dZ)
  // itsConv() gives the old delay centre in the new coordinates
  itsPhaseRotation = (casa::MVPosition(itsOut.getValue()) - casa::MVPosition(itsConv().getValue())) * rot3;
}

} // namespace accessors

} // namespace askap
