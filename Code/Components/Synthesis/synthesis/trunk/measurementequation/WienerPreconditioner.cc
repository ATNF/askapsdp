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
    
    WienerPreconditioner::WienerPreconditioner(float& noisepower) :
	    itsNoisePower(noisepower)
    {
    }
    
    WienerPreconditioner::~WienerPreconditioner() 
    {
    }
    
    WienerPreconditioner::ShPtr WienerPreconditioner::clone()
    {
	    return IImagePreconditioner::ShPtr(new WienerPreconditioner(*this));
    }
    
    bool WienerPreconditioner::doPreconditioning(casa::Array<float>& psf, casa::Array<float>& dirty)
    {
      if(itsNoisePower > 1e-06)
      {
	ASKAPLOG_INFO_STR(logger, "Applying Wiener filter with noise power " << itsNoisePower);

       casa::ArrayLattice<float> lpsf(psf);
       casa::ArrayLattice<float> ldirty(dirty);
       
       // Setup work arrays.
       IPosition valShape = lpsf.shape();
       casa::ArrayLattice<casa::Complex> weinerfilter(valShape);
       casa::ArrayLattice<casa::Complex> scratch(valShape);
       
       // Construct a Wiener filter
       scratch.copyData(casa::LatticeExpr<casa::Complex>(toComplex(lpsf)));
       LatticeFFT::cfft2d(scratch, True);
       casa::LatticeExpr<casa::Complex> wf(conj(scratch)/(scratch*conj(scratch) + itsNoisePower));
       weinerfilter.copyData(wf);
       
       // Apply the filter to the lpsf
       // (reuse the ft(lpsf) currently held in 'scratch')
       scratch.copyData(casa::LatticeExpr<casa::Complex> (weinerfilter * scratch));
       LatticeFFT::cfft2d(scratch, False);
       lpsf.copyData(casa::LatticeExpr<float> ( real(scratch) ));
       
       // Apply the filter to the dirty image
       scratch.copyData(casa::LatticeExpr<casa::Complex>(toComplex(ldirty)));
       LatticeFFT::cfft2d(scratch, True);
       scratch.copyData(casa::LatticeExpr<casa::Complex> (weinerfilter * scratch));
       LatticeFFT::cfft2d(scratch, False);
       ldirty.copyData(casa::LatticeExpr<float> ( real(scratch) ));
	  
       // Renormalize the PSF and dirty image
       float maxpsf = max(psf);
       psf/=maxpsf;
       dirty/=maxpsf;

       return true;
      }
      else
      {
	return false;
      }

    }

  }
}


