/// @file
///
/// WienerPreconditioner: Precondition the normal equations
///                       by applying a Wiener filter
///
/// (c) 2007 ASKAP, All Rights Reserved.
/// @author Urvashi Rau <rurvashi@aoc.nrao.edu>
///
#ifndef SYN_WEINER_PRECONDITIONER_H_
#define SYN_WEINER_PRECONDITIONER_H_

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <fitting/Axes.h>

#include <boost/shared_ptr.hpp>

#include <measurementequation/IImagePreconditioner.h>

namespace askap
{
  namespace synthesis
  {
    /// @brief Precondition the normal equations via a Wiener filter
    ///
    /// @details It constructs a Wiener filter from the PSF 
    /// and applies it to the PSF and current Residual image
    /// @ingroup measurementequation
    class WienerPreconditioner : public IImagePreconditioner
    {
      public:

        /// @brief Constructor from parameters.
        /// The parameters named image* will be interpreted as images and
        /// solutions formed by the method described.
        WienerPreconditioner();
        
        /// @brief constructor with explicitly defined noise poiwer
        /// @param[in] noisepower parameter of the 
        WienerPreconditioner(float& noisepower);
        
        /// @brief destructor
        ~WienerPreconditioner();

        /// @brief Clone this object
        /// @return shared pointer to a cloned copy
        virtual IImagePreconditioner::ShPtr clone();
        
	    /// @brief Apply preconditioning to Image Arrays
	    /// @details This is the actual method, which does preconditioning.
	    /// It is applied to the PSF as well as the current residual image.
	    /// @param[in] psf array with PSF
	    /// @param[in] dirty array with dirty image
	    bool doPreconditioning(casa::Array<float>& psf, casa::Array<float>& dirty);

      private:
  	    /// @brief Noise Power Spectrum
	    float itsNoisePower;
    };

  }
}
#endif
