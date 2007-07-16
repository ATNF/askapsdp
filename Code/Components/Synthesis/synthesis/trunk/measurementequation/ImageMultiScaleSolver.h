/// @file
///
/// ImageMultiScaleSolver: This solver calculates the dirty image (or equivalent)
/// for all parameters called image*
///
/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef SYNIMAGEMULTISCALESOLVER_H_
#define SYNIMAGEMULTISCALESOLVER_H_

#include <fitting/Solver.h>
#include <fitting/NormalEquations.h>
#include <fitting/DesignMatrix.h>
#include <fitting/Params.h>

#include <boost/shared_ptr.hpp>
#include <lattices/Lattices/LatticeCleaner.h>

#include <map>

namespace conrad
{
  namespace synthesis
  {
    /// @brief Simple linear solver for images.
    ///
    /// This solver takes the normal equations and simply divides
    /// the data vector by the diagonal of the normal matrix. This
    /// is analogous to making the dirty image or a linear mosaic
    /// of dirty images.
    class ImageMultiScaleSolver : public conrad::scimath::Solver
    {
      public:

        /// @brief Constructor from parameters.
        /// The parameters named image* will be interpreted as images and
        /// solutions formed by the method described.
        /// The default scales are 0, 10, 30 pixels
        /// @param ip Parameters i.e. the images
        ImageMultiScaleSolver(const conrad::scimath::Params& ip);

        /// @brief Constructor from parameters and scales.
        /// The parameters named image* will be interpreted as images and
        /// solutions formed by the method described.
        /// @param ip Parameters i.e. the images
        /// @param scales Scales to be solved in pixels
        ImageMultiScaleSolver(const conrad::scimath::Params& ip,
          const casa::Vector<float>& scales);
          
        /// @brief Initialize this solver
        virtual void init();

        /// @brief Solve for parameters, updating the values kept internally
        /// The solution is constructed from the normal equations
        /// @param q Solution quality information
        virtual bool solveNormalEquations(conrad::scimath::Quality& q);
        
/// @brief Clone this object
        virtual conrad::scimath::Solver::ShPtr clone() const;
        
        /// Set the scales
        void setScales(const casa::Vector<float>& scales);
        
      protected:
        /// Scales in pixels
        casa::Vector<float> itsScales;
        /// Map of Cleaners
        std::map<string, boost::shared_ptr<casa::LatticeCleaner<float> > > itsCleaners;

    };

  }
}
#endif
