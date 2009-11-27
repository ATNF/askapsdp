/// @file
/// @brief all logic behind uvw rotations and associated delays
/// @details
/// This class does all operations with UVW Machines to handle rotations and associated delays.
/// It is inherited from UVWMachineCache and uses it to maintain the cache. This class is
/// intended to manage accessor fields corresponding to uvw rotation as the functionality of
/// the CachedAccessorField template is not sufficient (can't pass parameters, which affect caching)
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

#include <dataaccess/UVWRotationHandler.h>
#include <askap/AskapError.h>

using namespace askap;
using namespace askap::synthesis;

/// @brief construct the handler
/// @details Set up basic parameters of the underlying machine cache.
/// @param[in] cacheSize a number of uvw machines in the cache (default is 1)
/// @param[in] tolerance pointing direction tolerance in radians, exceeding which leads 
/// to initialisation of a new UVW Machine and recompute of the rotated uvws/delays
UVWRotationHandler::UVWRotationHandler(size_t cacheSize, double tolerance) :
         UVWMachineCache(cacheSize, tolerance), itsValid(false) {}
         

/// @brief invalidate the cache
/// @details A call to this method invalidates the cache (for each accessor row) of rotated
/// uvws and delays. Nothing is done for uvw machines as UVWMachineCache takes care of this.
void UVWRotationHandler::invalidate() const
{
   itsValid = false;
}


/// @brief obtain rotated uvws
/// @details
/// Use parameters in the given accessor to compute rotated uvws
/// @param[in] acc const reference to the input accessor (need phase centre info, uvw, etc)
/// @param[in] tangent direction to the tangent point
/// @return const reference to rotated uvws
/// @note the method doesn't monitor a change to the accessor. It expects that invalidate 
/// is called explicitly when recalculation is needed (i.e. iterator moved to the next iteration, etc)
const casa::Vector<casa::RigidVector<casa::Double, 3> >& UVWRotationHandler::uvw(const IConstDataAccessor &acc, 
               const casa::MDirection &tangent) const
{
  ASKAPCHECK(tangent.getRef().getType() == casa::MDirection::J2000,
      "This is a cautionary assertion because a number of places in the code implicitly assume J2000 for "
      "tangent point and image centre. UVWRotationHandler works for any frame in theory, but one needs to deliver "
      "frame information to UVWMachines as well as to invalidate cache when say the time changes if it is required for conversion. "
      "This work has not been done and is beyond the scope for ASKAP.");
  if (!itsValid || !compare(tangent, itsTangentPoint)) {
     // have to fill itsRotatedUVW
     const casa::uInt nSamples = acc.nRow();
     itsRotatedUVWs.resize(nSamples);
     itsDelays.resize(nSamples);
     itsTangentPoint = tangent;
     itsImageCentre = tangent;
     itsValid = true;
     // just copy rotation code from TableVisGridder for a moment
     const casa::Vector<casa::RigidVector<double, 3> >& uvwVector = acc.uvw();
     const casa::Vector<casa::MVDirection>& pointingDir1Vector =
                        acc.pointingDir1();
     for (casa::uInt row=0; row<nSamples; ++row) {
          const casa::RigidVector<double, 3> &uvwRow = uvwVector(row);
          casa::Vector<double> uvwBuffer(3);
          /// @todo Decide what to do about pointingDir1!=pointingDir2
          for (int i=0; i<3; ++i) {
               uvwBuffer(i) = uvwRow(i);
          }
  
          /// @note we actually pass MVDirection as MDirection. The code had just been 
          /// copied, so this bug had been here for a while. It means that J2000 is
          /// hard coded in the next line (quite implicitly).
          const casa::UVWMachine& uvwm = machine(pointingDir1Vector(row),itsTangentPoint);
          uvwm.convertUVW(itsDelays(row), uvwBuffer);
          // the following line has been commented out to account for swapped arguments in the uvw machine call
          //itsDelays(row) *= -1;
          
          for (int i=0; i<3; ++i) {
               itsRotatedUVWs(row)(i) = uvwBuffer(i);
          } 
                   
     }
  }
  return itsRotatedUVWs;
}               

/// @brief obtain delays corresponding to rotation
/// @details
/// Use parameters in the given accessor to compute delays. This method calls rotatedUVWs and does
/// some extra job on the delays if tangent != imageCentre
/// @param[in] acc const reference to the input accessor (need phase centre info, uvw, etc)
/// @param[in] tangent direction to the tangent point
/// @param[in] imageCentre direction to the image centre
/// @return const reference to delay vector
/// @note the method doesn't monitor a change to the accessor. It expects that invalidate 
/// is called explicitly when recalculation is needed (i.e. iterator moved to the next iteration, etc)
const casa::Vector<casa::Double>& UVWRotationHandler::delays(const IConstDataAccessor &acc, 
               const casa::MDirection &tangent, const casa::MDirection &imageCentre) const
{
  const casa::Vector<casa::RigidVector<casa::Double, 3> >& uvwBuffer = uvw(acc, tangent);
  ASKAPDEBUGASSERT(itsDelays.nelements() == acc.nRow());

  ASKAPCHECK(imageCentre.getRef().getType() == casa::MDirection::J2000,
      "This is a cautionary assertion because a number of places in the code implicitly assume J2000 for "
      "tangent point and image centre. UVWRotationHandler works for any frame in theory, but one needs to deliver "
      "frame information to UVWMachines as well as to invalidate cache when say the time changes if it is required for conversion. "
      "This work has not been done and is beyond the scope for ASKAP.");
  
  if (!compare(itsImageCentre, imageCentre)) {
      // we have to apply extra shift
      ASKAPCHECK(itsImageCentre.getRef().getType() == imageCentre.getRef().getType(),
                 "image centres in UVWRotationHandler::delays are not supposed to be in different frames");
                 
      const casa::MVDirection oldCentre(itsImageCentre.getValue());
      const casa::MVDirection newCentre(imageCentre.getValue());
      // offsets to get the new image centre
      const double dl = sin(newCentre.getLong()-oldCentre.getLong())*cos(newCentre.getLat());
      const double dm = sin(newCentre.getLat())*cos(oldCentre.getLat()) - 
              cos(newCentre.getLat())*sin(oldCentre.getLat())
                   *cos(newCentre.getLong()-oldCentre.getLong());
      
      const casa::uInt nSamples = itsDelays.nelements();
      ASKAPDEBUGASSERT(nSamples == uvwBuffer.nelements());
      for (casa::uInt row=0; row<nSamples; ++row) {
           itsDelays(row) += uvwBuffer(row)(0)*dl + uvwBuffer(row)(1)*dm;
      }
      // now delays are recalculated to correspond to a new image centre
      itsImageCentre = imageCentre;
  } 
  return itsDelays;
}

