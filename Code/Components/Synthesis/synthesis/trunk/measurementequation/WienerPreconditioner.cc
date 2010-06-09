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

    /// @brief copy constructor
    /// @param[in] other object to copy from
    WienerPreconditioner::WienerPreconditioner(const WienerPreconditioner &other) :
          itsParameter(other.itsParameter), itsDoNormalise(other.itsDoNormalise),
          itsUseRobustness(other.itsUseRobustness)
    {
        if (other.itsTaper) {
            boost::shared_ptr<IImagePreconditioner> taper = other.itsTaper->clone();
            itsTaper = boost::dynamic_pointer_cast<GaussianTaperPreconditioner>(taper);
            ASKAPCHECK(itsTaper, "Dynamic cast failed, it is not supposed to happen");
        }
    }
        
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
      // we may need to transform PSF before filter is constructed
      // use reference semantics of casa arrays,, so there is no copying overhead if the psf passed unchanged
      casa::Array<float> psf4filter(psf);
            
      if (itsTaper) {
          // now we have to copy, because the values are going to be changed by the taper
          psf4filter.assign(psf.copy());
          itsTaper->applyTaper(psf4filter);
          const float maxPSFAfterTaper = casa::max(psf4filter);
          ASKAPLOG_INFO_STR(logger, "Peak of PSF after Gaussian tapering = " << maxPSFAfterTaper);
          ASKAPCHECK(maxPSFAfterTaper > 0., "Peak of PSF after Gaussian tapering is supposed to be positive");
          ASKAPLOG_INFO_STR(logger, "Renormalising PSF back to have peak = "<< maxPSFBefore);
          psf4filter *= maxPSFBefore / maxPSFAfterTaper;          
      }
      
      casa::ArrayLattice<float> lpsf4filter(psf4filter);      
      casa::ArrayLattice<float> ldirty(dirty);
      const casa::IPosition shape = lpsf4filter.shape();

      casa::ArrayLattice<casa::Complex> scratch(shape);
      scratch.copyData(casa::LatticeExpr<casa::Complex>(toComplex(lpsf4filter)));
       
      LatticeFFT::cfft2d(scratch, True);
       
      // Construct a Wiener filter
      const float normFactor = itsDoNormalise ? maxPSFBefore : 1.;
      const float noisePower = (itsUseRobustness ? std::pow(10., 4.*itsParameter) : itsParameter)*normFactor*normFactor;
       
      casa::ArrayLattice<casa::Complex> wienerfilter(shape);

      ASKAPLOG_INFO_STR(logger, "Effective noise power of the Wiener filter = " << noisePower);
      wienerfilter.copyData(casa::LatticeExpr<casa::Complex>(normFactor*conj(scratch)/(real(scratch*conj(scratch)) + noisePower)));

      casa::ArrayLattice<float> lpsf(psf);          
      // Apply the filter to scratch equal to FT(lpsf) if there is no tapering, otherwise regenerate FT of psf before
      // applying the filter
      if (itsTaper) {
          scratch.copyData(casa::LatticeExpr<casa::Complex>(toComplex(lpsf)));       
          LatticeFFT::cfft2d(scratch, True);          
      }   
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

    /// @brief static factory method to create preconditioner from a parset
    /// @details
    /// @param[in] parset subset of parset file (with preconditioner.Wiener. removed)
    /// @return shared pointer
    boost::shared_ptr<WienerPreconditioner> WienerPreconditioner::createPreconditioner(const LOFAR::ParameterSet &parset) 
    {
      ASKAPCHECK(parset.isDefined("noisepower") != parset.isDefined("robustness"), 
           "Exactly one parameter, either noisepower or robustness parameter must be given. You gave either none or both of them.");

      boost::shared_ptr<WienerPreconditioner> result;
      if (parset.isDefined("noisepower")) {
          const float noisepower = parset.getFloat("noisepower");
          const bool normalise = parset.getBool("normalise",false);
          result.reset(new WienerPreconditioner(noisepower,normalise));
      } else {
      
          ASKAPDEBUGASSERT(parset.isDefined("robustness"));
     
          const float robustness = parset.getFloat("robustness");
          ASKAPCHECK((robustness >= -2.00001) && (robustness <= 2.0001), 
                     "Robustness parameter is supposed to be between -2 and 2, you have = "<<robustness);
          ASKAPCHECK(!parset.isDefined("normalise"), 
                     "Normalise option of the Wiener preconditioner is not compatible with the "
                     "preconditioner definition via robustness (as normalisation of PSF is always done in this case)");
          result.reset(new WienerPreconditioner(robustness));
      }
      ASKAPASSERT(result);
      // configure PSF tapering
      if (parset.isDefined("psftaper")) {
          const double fwhm = parset.getDouble("psftaper");
          result->configurePSFTaper(fwhm);
      }
      //
      
      return result;
    }

    /// @brief assignment operator, to ensure it is not called
    WienerPreconditioner& WienerPreconditioner::operator=(const WienerPreconditioner &other) 
    {
      ASKAPTHROW(AskapError, "Assignment operator is not supposed to be used");
      return *this;
    }

    /// @brief configure PSF tapering 
    /// @details PSF can be tapered before filter is constructed. This mode is intended to reduce the
    /// effect of the uv-coverage gap at shortest baselines (wiener filter tries to deconcolve it).
    /// @param[in] fwhm full width at half maximum of the taper in the uv-plane
    /// (given as a fraction of the uv-cell size).
    /// @note Gaussian taper is set up in the uv-space. Size is given as FWHM expressed
    /// as fractions of uv-cell size. The relation between FWHMs in fourier and image plane is 
    /// uvFWHM = (Npix*cellsize / FWHM) * (4*log(2)/pi), where Npix is the number of pixels
    /// cellsize and FWHM are image-plane cell size and FWHM in angular units.
    void WienerPreconditioner::configurePSFTaper(double fwhm)
    {
      itsTaper.reset(new GaussianTaperPreconditioner(fwhm)); 
    }


  }
}


