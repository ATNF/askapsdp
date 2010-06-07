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
#include <utils/PaddingUtils.h>

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
	    itsParameter(0.0), itsDoNormalise(false), itsUseRobustness(false)
    {
    }
    
    /// @brief constructor with explicitly defined noise power
    /// @param[in] noisepower parameter of the
    /// @param[in] normalise if true, PSF is normalised during filter construction
    WienerPreconditioner::WienerPreconditioner(float noisepower, bool normalise) : 
            itsParameter(noisepower), itsDoNormalise(normalise), itsUseRobustness(false) {}

    /// @brief constructor with explicitly defined robustness
    /// @details In this version, the noise power is calculated from
    /// the robustness parameter
    /// @param[in] robustness robustness parameter (roughly matching Briggs' weighting)
    /// @note Normalisation of PSF is always used when noise power is defined via robustness
    WienerPreconditioner::WienerPreconditioner(float robustness) : itsParameter(robustness), 
            itsDoNormalise(true), itsUseRobustness(true)  {}
        
    IImagePreconditioner::ShPtr WienerPreconditioner::clone()
    {
	    return IImagePreconditioner::ShPtr(new WienerPreconditioner(*this));
    }
    
    bool WienerPreconditioner::doPreconditioning(casa::Array<float>& psf, casa::Array<float>& dirty) const
    {
      if (!itsUseRobustness && (itsParameter < 1e-6)) {
          return false;
      }
      if (itsUseRobustness) {
          ASKAPLOG_INFO_STR(logger, "Applying Wiener filter with noise power defined via robustness=" << itsParameter);
      } else {
          ASKAPLOG_INFO_STR(logger, "Applying Wiener filter with noise power=" << itsParameter);
      }

      const float maxPSFBefore = casa::max(psf);
      ASKAPLOG_INFO_STR(logger, "Peak of PSF before Wiener filtering = " << maxPSFBefore);

      if (itsDoNormalise) {
          ASKAPLOG_INFO_STR(logger, "The PSF will be normalised to 1 before filter construction");
      }

      casa::ArrayLattice<float> lpsf(psf);
      casa::ArrayLattice<float> ldirty(dirty);

      const casa::IPosition shape = lpsf.shape();

      casa::ArrayLattice<casa::Complex> scratch(shape);
      scratch.copyData(casa::LatticeExpr<casa::Complex>(toComplex(lpsf)));
       
      LatticeFFT::cfft2d(scratch, True);
       
      // Construct a Wiener filter
      const float normFactor = itsDoNormalise ? maxPSFBefore : 1.;
      const float noisePower = (itsUseRobustness ? std::pow(10., 4.*itsParameter) : itsParameter)*normFactor*normFactor;
       
      casa::ArrayLattice<casa::Complex> wienerfilter(shape);

      ASKAPLOG_INFO_STR(logger, "Effective noise power of the Wiener filter = " << noisePower);
      wienerfilter.copyData(casa::LatticeExpr<casa::Complex>(normFactor*conj(scratch)/(real(scratch*conj(scratch)) + noisePower)));

      // Apply the filter to the lpsf
       
      scratch.copyData(casa::LatticeExpr<casa::Complex> (wienerfilter * scratch));
       
      LatticeFFT::cfft2d(scratch, False);       
      lpsf.copyData(casa::LatticeExpr<float>(real(scratch)));

      const float maxPSFAfter=casa::max(psf);
      ASKAPLOG_INFO_STR(logger, "Peak of PSF after Wiener filtering  = " << maxPSFAfter); 
      psf *= maxPSFBefore/maxPSFAfter;
      ASKAPLOG_INFO_STR(logger, "Normalized to unit peak");
      
      // Apply the filter to the dirty image
      scratch.copyData(casa::LatticeExpr<casa::Complex>(toComplex(ldirty)));       
       
      LatticeFFT::cfft2d(scratch, True);
 
      scratch.copyData(casa::LatticeExpr<casa::Complex> (wienerfilter * scratch));
      LatticeFFT::cfft2d(scratch, False);

      ldirty.copyData(casa::LatticeExpr<float>(real(scratch)));
      dirty *= maxPSFBefore/maxPSFAfter;
	  
      return true;

    }

  }
}


