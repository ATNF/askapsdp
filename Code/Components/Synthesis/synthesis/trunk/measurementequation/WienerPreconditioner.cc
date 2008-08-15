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
#include <measurementequation/SynthesisParamsHelper.h>
#include <measurementequation/PaddingUtils.h>

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
    
    bool WienerPreconditioner::doPreconditioning(casa::Array<float>& psf, casa::Array<float>& dirty) const
    {
      if(itsNoisePower > 1e-06) {
	   ASKAPLOG_INFO_STR(logger, "Applying Wiener filter with noise power " << itsNoisePower);

       float maxPSFBefore=casa::max(psf);
       ASKAPLOG_INFO_STR(logger, "Peak of PSF before Wiener filtering = " << maxPSFBefore);
       casa::ArrayLattice<float> lpsf(psf);
       casa::ArrayLattice<float> ldirty(dirty);

       // we need to pad to twice the size in the image plane in order to avoid wraparound
       IPosition paddedShape = lpsf.shape();
       paddedShape(0)*=2;
       paddedShape(1)*=2;       
       casa::IPosition corner(paddedShape.nelements(),0);
       corner(0) = paddedShape(0)/4;
       corner(1) = paddedShape(1)/4;
       // set up slicer to work with inner quarter of a padded lattice
       casa::Slicer slicer(corner, lpsf.shape());
       casa::ArrayLattice<casa::Complex> scratch(paddedShape);
       scratch.set(0.);
       casa::SubLattice<casa::Complex> innerScratch(scratch, slicer, True);       
       //innerScratch.copyData(casa::LatticeExpr<casa::Complex>(toComplex(lpsf)));
       PaddingUtils::inject(scratch, lpsf);
      
       
       LatticeFFT::cfft2d(innerScratch, True);
       
       // Construct a Wiener filter
       
       casa::ArrayLattice<casa::Complex> wienerfilter(paddedShape);
       wienerfilter.set(0.);
       casa::SubLattice<casa::Complex> innerWF(wienerfilter, slicer, True);       
       casa::LatticeExpr<casa::Complex> wf(conj(innerScratch)/(innerScratch*conj(innerScratch) + itsNoisePower));
       innerWF.copyData(wf);
       // two FTs to do padding in the image plane
       LatticeFFT::cfft2d(innerWF, False);
       LatticeFFT::cfft2d(wienerfilter, True);                
              
       // Apply the filter to the lpsf
       // (reuse the ft(lpsf) currently held in 'scratch')
       
       // need to rebuild ft(lpsf) with padding, otherwise there is a scaling error
       scratch.set(0.);
       PaddingUtils::inject(scratch, lpsf);
       LatticeFFT::cfft2d(scratch, True);      
       //
       scratch.copyData(casa::LatticeExpr<casa::Complex> (wienerfilter * scratch));
       
       /*
       SynthesisParamsHelper::saveAsCasaImage("dbg.img",casa::amplitude(scratch.asArray()));       
       //SynthesisParamsHelper::saveAsCasaImage("dbg.img",lpsf.asArray());
       throw AskapError("This is a debug exception");
       */
       
       LatticeFFT::cfft2d(scratch, False);       
       PaddingUtils::extract(lpsf, scratch);
       float maxPSFAfter=casa::max(psf);
       ASKAPLOG_INFO_STR(logger, "Peak of PSF after Wiener filtering  = " << maxPSFAfter);
       psf*=maxPSFBefore/maxPSFAfter;
       ASKAPLOG_INFO_STR(logger, "Renormalizing peak to " << maxPSFBefore);
       
       // Apply the filter to the dirty image
       scratch.set(0.);
       PaddingUtils::inject(scratch, ldirty);
       //innerScratch.copyData(casa::LatticeExpr<casa::Complex>(toComplex(ldirty)));       
       
       LatticeFFT::cfft2d(scratch, True);
 
       scratch.copyData(casa::LatticeExpr<casa::Complex> (wienerfilter * scratch));
       LatticeFFT::cfft2d(scratch, False);
       PaddingUtils::extract(ldirty, scratch);
       //maxPSFBefore*=4.0;
       dirty*=maxPSFBefore/maxPSFAfter;
	  
       return true;
      }
      else {
	    return false;
      }

    }

  }
}


