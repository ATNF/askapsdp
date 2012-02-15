/// @file
/// @brief temporary replacement for UVWMachine
/// @details We suspect a bug in casacore's UVWMachine. This class is used
/// for debugging. Only uvw machine methods we're using are implemented. In the 
/// future, we will probably revert back to casacore's UVWMachine when it is fixed.
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
{}

/// @brief convert uvw
/// @param[in] delay reference to delay buffer
/// @param[in] uvw reference to uvw vector to update
void TempUVWMachine::convertUVW(casa::Double &delay, casa::Vector<casa::Double> &uvw) const
{
  // temporary
  delay=0;
  uvw.set(0.);
}


/// @brief initialise transform matrices
void TempUVWMachine::init()
{
}

} // namespace accessors

} // namespace askap
