/// @file
///
/// Solver: Abstract (but not pure!) base class for solvers of
/// parametrized equations.
///
/// The base class holds the parameters and the normal equations.
/// Derived classes perform the solution of the normal equations.
///
/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef SCIMATHSOLVER_H_
#define SCIMATHSOLVER_H_

#include <fitting/Params.h>
#include <fitting/INormalEquations.h>
#include <fitting/Solveable.h>
#include <fitting/Quality.h>

namespace conrad
{
  namespace scimath
  {

    /// Base class for solvers
    class Solver : public Solveable
    {
      public:
/// Constructor from parameters
/// @param ip Parameters
        explicit Solver(const Params& ip);
        
/// Copy constructor
        /// @param other Other
        Solver(const Solver& other);
        
        /// Assignment
        /// @param other Other
        Solver& operator=(const Solver& other);

        virtual ~Solver();

/// Initialize this solver
        virtual void init();

/// Set the parameters
/// @param ip Parameters
        void setParameters(const Params& ip);

/// Return current values of params (const)
        const Params& parameters() const;

/// Add the normal equations
/// @param normeq Normal Equations
        virtual void addNormalEquations(const INormalEquations& normeq);

/// Copy the normal equations from another solver
/// @param other Another solver
        virtual void copyNormalEquations(const Solver& other);

/// Solve for parameters, updating the values kept internally
/// The solution is constructed from the normal equations
/// @param q Quality of solution
        virtual bool solveNormalEquations(Quality& q);

/// Shared pointer definition
        typedef boost::shared_ptr<Solver> ShPtr;

/// Clone this into a shared pointer
        virtual Solver::ShPtr clone() const;
        
        /// @return a reference to normal equations object
        virtual const INormalEquations& normalEquations() const;

        /// @brief reset normal equations
        void resetNormalEquations() const;

      protected:
        /// Parameters
        Params::ShPtr itsParams;
      private:  
        /// Normal equations
        INormalEquations::ShPtr itsNormalEquations;
    };

  }
}
#endif                                            /*SOLVER_H_*/
