/// @file
/// @brief Helper method(s) to work with casa images
/// @details This file contains methods which are largely used for debugging. This is the reason
/// why we want to have them at the high enough level. It is envisaged that methods will be moved
/// here from SynthesisParamsHelper as required.
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

// own includes
#include <utils/ImageUtils.h>
#include <askap/AskapError.h>
#include <profile/AskapProfiler.h>



// casa includes
#include <casa/Arrays/Array.h>
#include <images/Images/PagedImage.h>
#include <lattices/Lattices/ArrayLattice.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/LinearCoordinate.h>


namespace askap {

namespace scimath {

/// @brief save a 2D array as a CASA image
/// @details This method is intended to be used largely for debugging. To save image from
/// parameter class use saveImageParameter method
/// @param[in] imagename name of the output image file
/// @param[in] arr input array
/// @ingroup utils
void saveAsCasaImage(const std::string &imagename, const casa::Array<casa::Float> &arr)
{
   ASKAPDEBUGTRACE("saveAsCasaImage");
   size_t nDim = arr.shape().nonDegenerate().nelements();
   casa::Vector<casa::String> names(2);
   ASKAPASSERT(nDim>=2);
   names[0]="x"; names[1]="y";
   casa::Vector<double> increment(2 ,1.);
      
   casa::Matrix<double> xform(2,2,0.);
   xform.diagonal() = 1.;
   casa::LinearCoordinate linear(names, casa::Vector<casa::String>(2,"pixel"),
           casa::Vector<double>(2,0.),increment, xform, casa::Vector<double>(2,0.));
      
   casa::CoordinateSystem coords;
   coords.addCoordinate(linear);
      
   for (size_t dim=2; dim<nDim; ++dim) {
        casa::Vector<casa::String> addname(1);
        addname[0]="addaxis"+utility::toString<size_t>(dim-1);
        casa::Matrix<double> xform(1,1,1.);
        casa::LinearCoordinate lc(addname, casa::Vector<casa::String>(1,"pixel"),
        casa::Vector<double>(1,0.), casa::Vector<double>(1,1.),xform, 
            casa::Vector<double>(1,0.));
        coords.addCoordinate(lc);
   }
   casa::PagedImage<casa::Float> result(casa::TiledShape(arr.nonDegenerate().shape()), coords, imagename);
   casa::ArrayLattice<casa::Float> lattice(arr.nonDegenerate());
   result.copyData(lattice);
}


} // namespace scimath

} // namespace askap