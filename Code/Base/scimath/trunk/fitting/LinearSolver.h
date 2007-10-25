/// @file
///
/// LinearSolver: This solver uses SVD to solve the normal
/// equations.
///
/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef SCIMATHLINEARSOLVER_H_
#define SCIMATHLINEARSOLVER_H_

#include <fitting/Solver.h>
#include <fitting/NormalEquations.h>
#include <fitting/DesignMatrix.h>
#include <fitting/Params.h>

namespace conrad
{
  namespace scimath
  {
    /// Solve the normal equations for updates to the parameters
    class LinearSolver : public Solver
    {
      public:
       /// @brief no limit on the condition number
       static const double KeepAllSingularValues = -1.;
      
       /// @brief Constructor
       /// @details Optionally, it is possible to limit the condition number of
       /// normal equation matrix to a given number.
       /// @param ip Parameters for this solver
       /// @param maxCondNumber maximum allowed condition number of the range
       /// of the normal equation matrix for the SVD algorithm. Effectively this
       /// puts the limit on the singular values, which are considered to be
       /// non-zero (all greater than the largest singular value divided by this
       /// condition number threshold). Default is 1e3. Put a negative number
       /// if you don't want to drop any singular values (may be a not very wise
       /// thing to do!). A very large threshold has the same effect. Zero
       /// threshold is not allowed and will cause an exception.
       explicit LinearSolver(const Params& ip, double maxCondNumber = 1e3);
        
/// Destructor
        virtual ~LinearSolver();

/// Initialize this solver
        virtual void init();

/// Solve for parameters, updating the values kept internally
/// The solution is constructed from the normal equations
/// @param q Quality information
        virtual bool solveNormalEquations(Quality& q);
        
/// @brief Clone this object
        virtual Solver::ShPtr clone() const;

       private:
         /// @brief maximum condition number allowed
         /// @details Effectively, this is a threshold for singular values 
         /// taken into account in the svd method
         double itsMaxCondNumber;
    };

  }
}
#endif
