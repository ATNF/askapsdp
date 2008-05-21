/// @file
///
/// IImagePreconditioner: Abstract Base class for Image Preconditioners
///
/// (c) 2007 ASKAP, All Rights Reserved.
/// @author Urvashi Rau <rurvashi@aoc.nrao.edu>
///
#ifndef SYNIMAGEPRECONDITIONER_H_
#define SYNIMAGEPRECONDITIONER_H_

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <fitting/Axes.h>

#include <boost/shared_ptr.hpp>
#include <dataaccess/SharedIter.h>
namespace askap
{
  namespace synthesis
  {
    /// @brief Base class for image-based preconditioners for the normal eqns
    ///
    /// @details It takes the normal equations and preconditions them
    /// by operating on the PSF and current Residual images
    /// @ingroup measurementequation
    class IImagePreconditioner 
    {
      public:
	/// Shared pointer definition
	typedef boost::shared_ptr<IImagePreconditioner> ShPtr;
	
        /// @brief Constructor from parameters.
        /// The parameters named image* will be interpreted as images and
        /// solutions formed by the method described.
        IImagePreconditioner();
        virtual ~IImagePreconditioner();

	// @brief Apply preconditioning to Image Arrays
	// It is applied to the PSF as well as the current residual image.
	virtual bool doPreconditioning(casa::Array<float>& psf, casa::Array<float>& dirty);
    };

  }
}
#endif
