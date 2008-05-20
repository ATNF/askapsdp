/// @file
///
/// ImageRestoreSolver: This solver restores images by smoothing the model
/// and adding the residuals. Note that the units will be changed from
/// Jy/pixel to Jy/beam.
///
/// (c) 2007 ASKAP, All Rights Reserved.
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
          const casa::Vector<casa::Quantum<double> >& beam,
	  const float& weinerparam);

        /// @brief Initialize this solver
        virtual void init();

        /// @brief Solve for parameters, updating the values kept internally
        /// The solution is constructed from the normal equations
        /// @param q Solution quality information
        virtual bool solveNormalEquations(askap::scimath::Quality& q);
        
    /// @brief Clone this object
        virtual askap::scimath::Solver::ShPtr clone() const;
 
      protected:
	/// Apply a Weiner filter to the dirty image
	void applyWeinerFilter(casa::ArrayLattice<float>& psf, casa::ArrayLattice<float>& dirty, float& noisepower);

      private:
        /// @brief Major, minor axes, and position angle of beam
        casa::Vector<casa::Quantum<double> > itsBeam;

	/// Weiner filter Noise Power
	float itsWeinerParam;
    };

  }
}
#endif
