/// @file
///
/// LinearSolver: This solver uses SVD to solve the normal
/// equations.
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
#ifndef SCIMATHLINEARSOLVER_H_
#define SCIMATHLINEARSOLVER_H_

#include <fitting/Solver.h>
#include <fitting/INormalEquations.h>
#include <fitting/DesignMatrix.h>
#include <fitting/Params.h>

namespace askap
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
       /// @param maxCondNumber maximum allowed condition number of the range
       /// of the normal equation matrix for the SVD algorithm. Effectively this
       /// puts the limit on the singular values, which are considered to be
       /// non-zero (all greater than the largest singular value divided by this
       /// condition number threshold). Default is 1e3. Put a negative number
       /// if you don't want to drop any singular values (may be a not very wise
       /// thing to do!). A very large threshold has the same effect. Zero
       /// threshold is not allowed and will cause an exception.
       explicit LinearSolver(double maxCondNumber = 1e3);
        
        /// Initialize this solver
        virtual void init();

        /// @brief solve for parameters
        /// The solution is constructed from the normal equations and given
        /// parameters are updated. If there are no free parameters in the
        /// given Params class, all unknowns in the normal
        /// equatons will be solved for.
        /// @param[in] params parameters to be updated 
        /// @param[in] quality Quality of solution
        virtual bool solveNormalEquations(Params &params, Quality& quality);
        
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
