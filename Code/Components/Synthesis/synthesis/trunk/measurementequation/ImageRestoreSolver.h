/// @file
///
/// ImageRestoreSolver: This solver restores images by smoothing the model
/// and adding the residuals. Note that the units will be changed from
/// Jy/pixel to Jy/beam.
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
#ifndef SYNIMAGERESTORESOLVER_H_
#define SYNIMAGERESTORESOLVER_H_

#include <measurementequation/ImageSolver.h>
#include <casa/Arrays/Vector.h>
#include <casa/Quanta.h>
#include <lattices/Lattices/ArrayLattice.h>

namespace askap
{
  namespace synthesis
  {
    /// @brief Restore solver for images.
    ///
    /// ImageRestoreSolver: This solver restores images by smoothing the model
    /// and adding the residuals. Note that the units will be changed from
    /// Jy/pixel to Jy/beam.
    /// @ingroup measurementequation
    class ImageRestoreSolver : public ImageSolver
    {
      public:

        /// @brief Constructor from existing params
        /// The parameters named image* will be interpreted as images and
        /// solutions formed by the method described.
        /// @param ip Parameters
        /// @param beam Major, minor, pa of beam as Quanta
        ImageRestoreSolver(const askap::scimath::Params& ip,
          const casa::Vector<casa::Quantum<double> >& beam);

        /// @brief Initialize this solver
        virtual void init();

        /// @brief Solve for parameters, updating the values kept internally
        /// The solution is constructed from the normal equations
        /// @param q Solution quality information
        /// @return one needs to figure it out and write here
        virtual bool solveNormalEquations(askap::scimath::Quality& q);
        
        /// @brief Clone this object
        /// @return a shared pointer to a cloned object
        virtual askap::scimath::Solver::ShPtr clone() const;
    
      protected:
      
        /// @brief solves for and adds residuals
        /// @details Restore solver convolves the current model with the beam and adds the
        /// residual image. The latter has to be "solved for" with a proper preconditioning and
        /// normalisation using the normal equations stored in the base class. All operations
        /// required to extract residuals from normal equations and fill an array with them
        /// are encapsulated in this method. Faceting needs a subimage only, hence the array
        /// to fill may not have exactly the same shape as the dirty (residual) image corresponding
        /// to the given parameter. This method assumes that the centres of both images are the same
        /// and extracts only data required (this feature is not yet implemented).
        /// @param[in] name name of the parameter to work with
        /// @param[in] shape shape of the parameter (we wouldn't need it if the shape of the
        ///                   output was always the same as the shape of the paramter. It is not
        ///                   the case for faceting).
        /// @param[in] out output array
        void addResiduals(const std::string &name, const casa::IPosition &shape,
                         casa::Array<double> &out);        
        
      private:
        /// @brief Major, minor axes, and position angle of beam
        casa::Vector<casa::Quantum<double> > itsBeam;
    };

  }
}
#endif
