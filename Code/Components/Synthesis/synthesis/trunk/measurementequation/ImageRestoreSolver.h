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
#include <Common/ParameterSet.h>


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
    
        /// @brief static method to create solver
        /// @details Each solver should have a static factory method, which is
        /// able to create a particular type of the solver and initialise it with
        /// the parameters taken from the given parset. It is assumed that the method
        /// receives a subset of parameters where the solver name, if it was present in
        /// the parset, is already taken out
        /// @param[in] parset input parset file
        /// @param[in] ip model parameters
        /// @return a shared pointer to the solver instance
        static boost::shared_ptr<ImageRestoreSolver> createSolver(const LOFAR::ParameterSet &parset,
                   const askap::scimath::Params &ip);
        
      protected:
        /// @brief set noise equalisation flag
        /// @param[in] flag true, to switch noise equalisation on
        inline void equaliseNoise(bool flag) { itsEqualiseNoise = flag; }
      
        /// @brief solves for and adds residuals
        /// @details Restore solver convolves the current model with the beam and adds the
        /// residual image. The latter has to be "solved for" with a proper preconditioning and
        /// normalisation using the normal equations stored in the base class. All operations
        /// required to extract residuals from normal equations and fill an array with them
        /// are encapsulated in this method. Faceting needs a subimage only, hence the array
        /// to fill may not have exactly the same shape as the dirty (residual) image corresponding
        /// to the given parameter. This method assumes that the centres of both images are the same
        /// and extracts only data required.
        /// @param[in] name name of the parameter to work with
        /// @param[in] shape shape of the parameter (we wouldn't need it if the shape of the
        ///                   output was always the same as the shape of the paramter. It is not
        ///                   the case for faceting).
        /// @param[in] out output array
        void addResiduals(const std::string &name, const casa::IPosition &shape,
                         casa::Array<double> out) const;
        
        /// @brief obtain an estimate of the restoring beam
        /// @details This method fits a 2D Gaussian into the central area of the PSF
        /// (a support is searched assuming 50% cutoff) if the appropriate option
        /// is set. Otherwise, it just returns the beam parameters passed in the constructor
        /// (i.e. user override).
        /// @param[in] name name of the parameter to work with
        /// @param[in] shape shape of the parameter
        casa::Vector<casa::Quantum<double> > getBeam(const std::string &name, const casa::IPosition &shape) const;
        
      private:
        /// @brief Major, minor axes, and position angle of beam
        casa::Vector<casa::Quantum<double> > itsBeam;
        
        /// @brief true if the mosaicing weight is to be equalised
        /// @details We optionally can multiply the residual to the weight before
        /// adding to the model convolved with the restoring beam. As per Sault et al.
        /// (1996) this gives aesthetically pleasing images. However, as not all flux is
        /// recovered in the model, this weighting scheme potentially introduces some 
        /// direction-dependent flux error (but gives flat noise).
        bool itsEqualiseNoise; 
    };

  }
}
#endif
