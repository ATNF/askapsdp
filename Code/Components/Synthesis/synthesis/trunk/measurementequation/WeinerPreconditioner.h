/// @file
///
/// WeinerPreconditioner: Precondition the normal equations
///                       by applying a Weiner filter
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
    /// @brief Precondition the normal equations via a Weiner filter
    ///
    /// @details It constructs a Weiner filter from the PSF 
    /// and applies it to the PSF and current Residual image
    /// @ingroup measurementequation
    class WeinerPreconditioner : public IImagePreconditioner
    {
      public:

        /// @brief Constructor from parameters.
        /// The parameters named image* will be interpreted as images and
        /// solutions formed by the method described.
        WeinerPreconditioner();
        WeinerPreconditioner(float& noisepower);
        ~WeinerPreconditioner();

        /// @brief Clone this object
        virtual IImagePreconditioner::ShPtr clone();
        
	// @brief Apply preconditioning to Image Arrays
	// It is applied to the PSF as well as the current residual image.
	bool doPreconditioning(casa::Array<float>& psf, casa::Array<float>& dirty);

      private:
	// Noise Power Spectrum
	float itsNoisePower;
    };

  }
}
#endif
