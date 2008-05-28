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


