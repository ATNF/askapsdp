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

#ifndef ASKAP_ACCESSORS_TEMP_UVW_MACHINE_H
#define ASKAP_ACCESSORS_TEMP_UVW_MACHINE_H

// casa includes
#include <measures/Measures.h>
#include <measures/Measures/MDirection.h>
#include <casa/Arrays/Vector.h>
#include <measures/Measures/MCDirection.h>
#include <casa/Quanta/MVPosition.h>
#include <casa/Quanta/RotMatrix.h>


// boost includes
#include <boost/utility.hpp>

namespace askap {

namespace accessors {

/// @brief temporary replacement for UVWMachine
/// @details We suspect a bug in casacore's UVWMachine. This class is used
/// for debugging. Only uvw machine methods we're using are implemented. In the 
/// future, we will probably revert back to casacore's UVWMachine when it is fixed.
/// @ingroup dataaccess_hlp
class TempUVWMachine : private boost::noncopyable {
public:
   /// @brief construct the machine
   /// @details
   /// @param[in] in input direction
   /// @param[in] out output direction
   /// @param[in] EW east-west flag (not used)
   /// @param[in] project reprojection flag (always assumed true)
   /// @note in and out are swapped w.r.t. casa UVWMachine as this is how it is used in
   /// the current code
   TempUVWMachine(const casa::MDirection &in, const casa::MDirection &out, const bool EW = false, const bool project = true);
   
   /// @brief convert uvw
   /// @param[in] delay reference to delay buffer
   /// @param[in] uvw reference to uvw vector to update
   void convertUVW(casa::Double &delay, casa::Vector<casa::Double> &uvw) const;

   /// @brief convert uvw
   /// @param[in] uvw reference to uvw vector to update
   void convertUVW(casa::Vector<casa::Double> &uvw) const;
   
private:
   /// @brief initialise transform matrices
   void init();
      
   /// @brief direction corresponding to the old delay centre
   /// @note the reference frame corresponds to the old delay centre
   casa::MDirection itsIn;
   
   /// @brief direction corresponding to the new delay centre
   casa::MDirection itsOut;   
   
   /// @brief uvw rotation
   /// @note The uvw vector is right-multiplied by this rotation matrix, so the actual rotation matrix is transposed
   casa::RotMatrix itsUVWRotation;
   
   /// @brief phase rotation
   casa::MVPosition itsPhaseRotation;
   
   /// @brief conversion engine 
   casa::MDirection::Convert itsConv;
};

} // namespace accessors

} // namespace askap

#endif // #ifndef ASKAP_ACCESSORS_TEMP_UVW_MACHINE_H


