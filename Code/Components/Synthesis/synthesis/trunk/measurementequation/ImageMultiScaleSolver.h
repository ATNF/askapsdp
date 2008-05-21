/// @file
///
/// ImageMultiScaleSolver: This solver calculates the dirty image (or equivalent)
/// for all parameters called image*
///
/// (c) 2007 ASKAP, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef SYNIMAGEMULTISCALESOLVER_H_
#define SYNIMAGEMULTISCALESOLVER_H_

#include <measurementequation/ImageSolver.h>

#include <boost/shared_ptr.hpp>
#include <lattices/Lattices/LatticeCleaner.h>

#include <map>

namespace askap
{
  namespace synthesis
  {
    /// @brief Multiscale solver for images.
    ///
    /// @details This solver performs multi-scale clean using the 
    /// casa::LatticeCleaner classes
    ///
    /// @ingroup measurementequation
    class ImageMultiScaleSolver : public ImageSolver
    {
      public:

        /// @brief Constructor from parameters.
        /// The parameters named image* will be interpreted as images and
        /// solutions formed by the method described.
        /// The default scales are 0, 10, 30 pixels
        /// @param ip Parameters i.e. the images
        ImageMultiScaleSolver(const askap::scimath::Params& ip);

        /// @brief Constructor from parameters and scales.
        /// The parameters named image* will be interpreted as images and
        /// solutions formed by the method described.
        /// @param ip Parameters i.e. the images
        /// @param scales Scales to be solved in pixels
        ImageMultiScaleSolver(const askap::scimath::Params& ip,
          const casa::Vector<float>& scales);
        
        /// @brief Initialize this solver
        virtual void init();

        /// @brief Solve for parameters, updating the values kept internally
        /// The solution is constructed from the normal equations
        /// @param q Solution quality information
        virtual bool solveNormalEquations(askap::scimath::Quality& q);
        
/// @brief Clone this object
        virtual askap::scimath::Solver::ShPtr clone() const;
        
        /// Set the scales
        void setScales(const casa::Vector<float>& scales);
               
      protected:
	/// Precondition the PSF and the dirty image
	void preconditionNE(casa::ArrayLattice<float>& psf, casa::ArrayLattice<float>& dirty);
	
        /// Scales in pixels
        casa::Vector<float> itsScales;
        /// Map of Cleaners
        std::map<string, boost::shared_ptr<casa::LatticeCleaner<float> > > itsCleaners;

    };

  }
}
#endif
