/// @file
///
/// ImageSolver: This solver calculates the dirty image (or equivalent)
/// for all parameters called image*
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
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef SYNIMAGESOLVER_H_
#define SYNIMAGESOLVER_H_

#include <fitting/Solver.h>
#include <fitting/ImagingNormalEquations.h>
#include <fitting/DesignMatrix.h>
#include <fitting/Params.h>

#include <measurementequation/IImagePreconditioner.h>
#include <boost/shared_ptr.hpp>

#include <map>
using std::map;

namespace askap
{
  namespace synthesis
  {
    /// @brief Base class for solvers of images
    ///
    /// @details This solver takes the normal equations and simply divides
    /// the data vector by the diagonal of the normal matrix. This
    /// is analogous to making the dirty image or a linear mosaic
    /// of dirty images.
    /// @ingroup measurementequation
    class ImageSolver : public askap::scimath::Solver
    {
      public:
        /// @brief typedef of the shared pointer to ImageSolver
	    typedef boost::shared_ptr<ImageSolver> ShPtr;
	
        /// @brief Constructor from parameters.
        /// The parameters named image* will be interpreted as images and
        /// solutions formed by the method described.
        ImageSolver(const askap::scimath::Params& ip);

        /// @brief Initialize this solver
        virtual void init();

        /// @brief Solve for parameters, updating the values kept internally
        /// The solution is constructed from the normal equations
        /// @param q Solution quality information
        virtual bool solveNormalEquations(askap::scimath::Quality& q);
        
/// @brief Clone this object
        virtual askap::scimath::Solver::ShPtr clone() const;
        
        /// @brief Save the weights as a parameter
        virtual void saveWeights();

        /// @brief Save the PSFs as a parameter
        virtual void savePSF();

        /// @return a reference to normal equations object
        /// @note In this class and derived classes the type returned
        /// by this method is narrowed to always provide image-specific 
        /// normal equations objects
        virtual const scimath::ImagingNormalEquations& normalEquations() const;

	/// @brief Setup the preconditioner
	void addPreconditioner(askap::synthesis::IImagePreconditioner::ShPtr pc);

	/// @brief Do the preconditioning
	bool doPreconditioning(casa::Array<float>& psf, casa::Array<float>& dirty);

	/// @brief Divide the PSF and dirty image by the diagonal of the Hessian
	bool doNormalization(const casa::Vector<double>& diag, 
			     const float& tolerance,
			     casa::Array<float>& psf, 
			     casa::Array<float>& dirty);
   
	/// @brief Divide the PSF and dirty image by the diagonal of the Hessian
	bool doNormalization(const casa::Vector<double>& diag, 
			     const float& tolerance,
			     casa::Array<float>& psf, 
			     casa::Array<float>& dirty,
			     casa::Array<float>& mask);
   
      private:
	/// Instance of a preconditioner
	// IImagePreconditioner::ShPtr itsPreconditioner;
        //std::map<std::int, boost::shared_ptr<askap::synthesis::IImagePreconditioner> > itsPreconditioners;
        std::map<int, IImagePreconditioner::ShPtr> itsPreconditioners;
    };

  }
}
#endif
