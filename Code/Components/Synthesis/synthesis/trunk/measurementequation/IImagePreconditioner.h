/// @file
///
/// IImagePreconditioner: Abstract Base class for Image Preconditioners
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

	/// @brief Apply preconditioning to Image Arrays
	/// @details It is applied to the PSF as well as the current residual image.
	/// @param[in] psf an array with the PSF
	/// @param[in] dirty an array with dirty image
	virtual bool doPreconditioning(casa::Array<float>& psf, casa::Array<float>& dirty);
    };

  }
}
#endif
