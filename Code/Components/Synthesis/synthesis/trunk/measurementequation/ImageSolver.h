/// @file
///
/// ImageSolver: This solver calculates the dirty image (or equivalent)
/// for all parameters called image*
///
/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef SYNIMAGESOLVER_H_
#define SYNIMAGESOLVER_H_

#include <fitting/Solver.h>
#include <fitting/NormalEquations.h>
#include <fitting/DesignMatrix.h>
#include <fitting/Params.h>

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
    class ImageSolver : public conrad::scimath::Solver
    {
      public:

        /// @brief Constructor from parameters.
        /// The parameters named image* will be interpreted as images and
        /// solutions formed by the method described.
        ImageSolver(const conrad::scimath::Params& ip);

        /// @brief Initialize this solver
        virtual void init();

        /// @brief Solve for parameters, updating the values kept internally
        /// The solution is constructed from the normal equations
        /// @param q Solution quality information
        virtual bool solveNormalEquations(conrad::scimath::Quality& q);
        
/// @brief Clone this object
        virtual conrad::scimath::Solver::ShPtr clone();

    };

  }
}
#endif
