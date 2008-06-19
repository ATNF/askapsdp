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

#include <askap_synthesis.h>
#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, ".measurementequation");

#include <askap/AskapError.h>

#include <lattices/Lattices/ArrayLattice.h>
#include <lattices/Lattices/LatticeFFT.h>
#include <lattices/Lattices/LatticeExpr.h>
#include <casa/BasicSL/Constants.h>
#include <casa/Arrays/ArrayMath.h>
#include <scimath/Mathematics/SquareMatrix.h>
#include <scimath/Mathematics/RigidVector.h>


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
  ASKAPLOG_INFO_STR(logger, "Applying Gaussian taper "<<itsMajorAxis*sqrt(8.*log(2.))<<" x "<<
                    itsMinorAxis*sqrt(8.*log(2.))<<" uv cells at position angle "<<itsPA/M_PI*180.<<" degrees");
  ASKAPDEBUGASSERT(psf.shape().isEqual(dirty.shape()));
  
  applyTaper(psf);
  applyTaper(dirty);
    
  return true;
}

/// @brief a helper method to apply the taper to one given array
/// @details We need exactly the same operation for psf and dirty image. This method
/// encapsulates the code which is actually doing the job. It is called twice from
/// doPreconditioning.
/// @param[in] image an image to apply the taper to
void GaussianTaperPreconditioner::applyTaper(casa::Array<float> &image) const
{
  casa::ArrayLattice<float> lattice(image);
  
  // Setup work arrays.
  const casa::IPosition shape = lattice.shape();
  casa::ArrayLattice<casa::Complex> scratch(shape);
  
  if (!shape.isEqual(itsTaperCache.shape())) {
      initTaperCache(shape);
  }
  
  // fft to transform the image into uv-domain
  scratch.copyData(casa::LatticeExpr<casa::Complex>(toComplex(lattice)));
  casa::LatticeFFT::cfft2d(scratch, true);
  
  // apply the taper
  casa::ArrayLattice<casa::Complex> taper(itsTaperCache);
  scratch.copyData(casa::LatticeExpr<casa::Complex> (taper * scratch));
  
  // transform back to the image domain
  casa::LatticeFFT::cfft2d(scratch, false);
  lattice.copyData(casa::LatticeExpr<float> ( real(scratch) ));
}

/// @brief a helper method to build the lattice representing the taper
/// @details applyTaper can be reused many times for the same taper. This 
/// method populates the cached array with proper values corresponding
/// to the taper.
/// @param[in] shape shape of the required array
void GaussianTaperPreconditioner::initTaperCache(const casa::IPosition &shape) const
{
  ASKAPDEBUGASSERT(shape.nelements() >= 2);

#ifdef ASKAP_DEBUG
  // if shape is exactly 2, nonDegenerate(2) would throw an exception. Hence, we need
  // a special check to avoid this.
  if (shape.nelements() > 2) {
     ASKAPASSERT(shape.nonDegenerate(2).nelements() == 2);
  }
#endif  

  itsTaperCache.resize(shape);
  const casa::Int nx = shape[0];
  const casa::Int ny = shape[1];
  casa::IPosition index(shape.nelements(),0);
  casa::SquareMatrix<casa::Double, 2> rotation(casa::SquareMatrix<casa::Double, 2>::General);
  // rotation direction is flipped here as we rotate the gaussian, not
  // the coordinate

  rotation(0,0) = rotation(1,1) = sin(itsPA);
  rotation(1,0) = cos(itsPA);
  rotation(0,1) = -rotation(1,0);
  
  // the following formula introduces some error if position angle is not 0
  // may be we need just to sum values?
  //const double normFactor = 2.*M_PI*itsMajorAxis*itsMinorAxis*erf(double(nx)/(2.*sqrt(2.)*itsMajorAxis))*
  //            erf(double(ny)/(2.*sqrt(2.)*itsMinorAxis));
  double sum = 0.;            
  for (index[0] = 0; index[0]<nx; ++index[0]) {
       for (index[1] = 0; index[1]<ny; ++index[1]) {
            casa::RigidVector<casa::Double, 2> offset;
            offset(0) = (double(index[0])-double(nx)/2.);
            offset(1) = (double(index[1])-double(ny)/2.);
            // operator* is commented out in RigidVector due to
            // problems with some compilers. We have to use operator*= instead.
            // according to manual it is equivalent to v=Mv, rather than to v=v*M
            offset *= rotation;
            const double taperingFactor = exp(-casa::square(offset(0)/itsMajorAxis)/2.-
                       casa::square(offset(1)/itsMinorAxis)/2.);
            sum += taperingFactor;
            itsTaperCache(index) = taperingFactor;
       }
  }
  //std::cout<<"normFactor/sum: "<<normFactor/sum<<std::endl;
  itsTaperCache *= casa::Complex(sum,0.);
}

} // namespace synthesis

} // namespace askap

