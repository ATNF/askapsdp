/// @file
///
/// @brief apply gaussian taper
/// @details This preconditioner applies gaussian taper in the uv-space to the normal
/// equations.
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

#include <measurementequation/GaussianTaperPreconditioner.h>

namespace askap {

namespace synthesis {


/// @brief set up the preconditioner 
/// @details This constructor just sets the taper size. The size is full width at
/// half maximum expressed in the units of uv-cell.
/// @param[in] majFWHM full width at half maximum of the major axis in the uv-plane 
/// (given as a fraction of the uv-cell size).
/// @param[in] minFWHM full width at half maximum of the minor axis in the uv-plane 
/// (given as a fraction of the uv-cell size).
/// @param[in] pa position angle in radians
GaussianTaperPreconditioner::GaussianTaperPreconditioner(double majFWHM, double minFWHM, double pa) :
     itsMajorAxis(majFWHM/sqrt(8.*log(2.))), itsMinorAxis(minFWHM/sqrt(8.*log(2.))),itsPA(pa) {}
   
/// @brief set up the preconditioner for the circularly symmetric taper 
/// @details This constructor just sets the taper size, same for both axis.
/// The size is full width at half maximum expressed in the units of uv-cell.
/// @param[in] fwhm full width at half maximum of the taper in the uv-plane
/// (given as a fraction of the uv-cell size).
GaussianTaperPreconditioner::GaussianTaperPreconditioner(double fwhm) : 
     itsMajorAxis(fwhm/sqrt(8.*log(2.))), itsPA(0.) 
{
  itsMinorAxis = itsMajorAxis;
}
   
/// @brief Clone this object
/// @return shared pointer to a cloned copy
IImagePreconditioner::ShPtr GaussianTaperPreconditioner::clone()
{
  return IImagePreconditioner::ShPtr(new GaussianTaperPreconditioner(*this));
}
        
/// @brief Apply preconditioning to Image Arrays
/// @details This is the actual method, which does preconditioning.
/// It is applied to the PSF as well as the current residual image.
/// @param[in] psf array with PSF
/// @param[in] dirty array with dirty image
/// @return true if psf and dirty have been altered
bool GaussianTaperPreconditioner::doPreconditioning(casa::Array<float>& psf, casa::Array<float>& dirty) const
{
  return false;
}

} // namespace synthesis

} // namespace askap

