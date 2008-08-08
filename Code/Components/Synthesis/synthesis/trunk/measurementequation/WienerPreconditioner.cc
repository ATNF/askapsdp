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

#include <measurementequation/WienerPreconditioner.h>

#include <askap_synthesis.h>
#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, ".measurementequation");

#include <askap/AskapError.h>

#include <casa/aips.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/Arrays/Vector.h>
#include <lattices/Lattices/SubLattice.h>
#include <lattices/Lattices/ArrayLattice.h>
#include <lattices/Lattices/LatticeFFT.h>
#include <lattices/Lattices/LatticeExpr.h>
using namespace casa;

#include <iostream>
#include <cmath>
using std::abs;

namespace askap
{
  namespace synthesis
  {

    WienerPreconditioner::WienerPreconditioner() :
	    itsNoisePower(0.0)
    {
    }
    
    WienerPreconditioner::WienerPreconditioner(const float& noisepower) :
	    itsNoisePower(noisepower)
    {
    }
        
    IImagePreconditioner::ShPtr WienerPreconditioner::clone()
    {
	    return IImagePreconditioner::ShPtr(new WienerPreconditioner(*this));
    }
    
    // Inject source into the centre quarter of the target
    void WienerPreconditioner::inject(casa::Lattice<casa::Complex>& target, casa::Lattice<float>& source) const
    {
      target.set(0.0);
      casa::IPosition corner(target.shape().nelements(),0);
      ASKAPDEBUGASSERT(corner.nelements()>=2);
      ASKAPDEBUGASSERT(target.shape()(0) == source.shape()(0)*2);
      ASKAPDEBUGASSERT(target.shape()(1) == source.shape()(1)*2);
      
      corner(0) = target.shape()(0)/4;
      corner(1) = target.shape()(1)/4;
      casa::Slicer slicer(corner, source.shape());
      casa::SubLattice<casa::Complex> inner(target, slicer, True);
      inner.copyData(casa::LatticeExpr<casa::Complex>(toComplex(source)));
      //      ASKAPLOG_INFO_STR(logger, "Injected " << source.shape() << " into " << target.shape() << " starting at " << corner);
    }

    // Extract target from the center quarter of the source 
    void WienerPreconditioner::extract(casa::Lattice<float>& target, casa::Lattice<casa::Complex>& source) const
    {
      target.set(0.0);
      casa::IPosition corner(source.shape().nelements(),0);
      ASKAPDEBUGASSERT(corner.nelements()>=2);
      ASKAPDEBUGASSERT(source.shape()(0) == target.shape()(0)*2);
      ASKAPDEBUGASSERT(source.shape()(1) == target.shape()(1)*2);
      corner(0) = source.shape()(0)/4;
      corner(1) = source.shape()(1)/4;
      casa::Slicer slicer(corner, target.shape());
      casa::SubLattice<casa::Complex> inner(source, slicer, True);
      target.copyData(casa::LatticeExpr<float>(real(inner)));
      //      ASKAPLOG_INFO_STR(logger, "Extracted " << target.shape() << " from " << source.shape() << " starting at " << corner);
    }

    bool WienerPreconditioner::doPreconditioning(casa::Array<float>& psf, casa::Array<float>& dirty) const
    {
      if(itsNoisePower > 1e-06) {
	   ASKAPLOG_INFO_STR(logger, "Applying Wiener filter with noise power " << itsNoisePower);

       float maxPSFBefore=casa::max(psf);
       ASKAPLOG_INFO_STR(logger, "Peak of PSF before Wiener filtering = " << maxPSFBefore);
       casa::ArrayLattice<float> lpsf(psf);
       casa::ArrayLattice<float> ldirty(dirty);
       
       // Setup work arrays. We need to pad to twice the size in order to avoid
       // wraparound.
       IPosition paddedShape = lpsf.shape();
       paddedShape(0)*=2;
       paddedShape(1)*=2;
       casa::ArrayLattice<casa::Complex> scratch(paddedShape);
       scratch.set(0.0);
       inject(scratch, lpsf);
       
       LatticeFFT::cfft2d(scratch, True);
       
       // Construct a Wiener filter
       casa::IPosition corner(paddedShape.nelements(),0);
       corner(0) = paddedShape(0)/4;
       corner(1) = paddedShape(1)/4;
       casa::Slicer slicer(corner, lpsf.shape());
       casa::SubLattice<casa::Complex> inner(scratch, slicer, True);      
       casa::ArrayLattice<casa::Complex> wienerfilter(scratch.shape());
       wienerfilter.set(0.);
       casa::SubLattice<casa::Complex> innerWF(wienerfilter, slicer, True);             
       casa::LatticeExpr<casa::Complex> wf(conj(inner)/(inner*conj(inner) + itsNoisePower));
       innerWF.copyData(wf);
              
       // Apply the filter to the lpsf
       // (reuse the ft(lpsf) currently held in 'scratch')
       scratch.copyData(casa::LatticeExpr<casa::Complex> (wienerfilter * scratch));
       LatticeFFT::cfft2d(scratch, False);       
       extract(lpsf, scratch);
       float maxPSFAfter=casa::max(psf);
       ASKAPLOG_INFO_STR(logger, "Peak of PSF after Wiener filtering  = " << maxPSFAfter);
       psf*=maxPSFBefore/maxPSFAfter;
       ASKAPLOG_INFO_STR(logger, "Renormalizing peak to " << maxPSFBefore);
       
       // Apply the filter to the dirty image
       scratch.set(0.);
       inject(scratch, ldirty);
       LatticeFFT::cfft2d(scratch, True);
 
       scratch.copyData(casa::LatticeExpr<casa::Complex> (wienerfilter * scratch));
       LatticeFFT::cfft2d(scratch, False);
       extract(ldirty, scratch);
       maxPSFBefore*=4.0;
       dirty*=maxPSFBefore/maxPSFAfter;
	  
       return true;
      }
      else {
	    return false;
      }

    }

  }
}


