/// @file
///
/// A linear solver for parameters from the normal equations
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
#include <fitting/LinearSolver.h>
#include <fitting/GSLSVDReplacement.h>

#include <askap/AskapError.h>
#include <profile/AskapProfiler.h>

#include <casa/aips.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_linalg.h>

#include <iostream>

#include <string>
#include <map>

#include <cmath>
using std::abs;
using std::map;
using std::string;

namespace askap
{
  namespace scimath
  {
    const double LinearSolver::KeepAllSingularValues;
    
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
    LinearSolver::LinearSolver(double maxCondNumber) : 
           itsMaxCondNumber(maxCondNumber) 
    {
      ASKAPASSERT(itsMaxCondNumber!=0);
    };

    
    void LinearSolver::init()
    {
      resetNormalEquations();
    }

    /// @brief solve for parameters
    /// The solution is constructed from the normal equations and given
    /// parameters are updated. If there are no free parameters in the
    /// given Params class, all unknowns in the normal
    /// equatons will be solved for.
    /// @param[in] params parameters to be updated 
    /// @param[in] quality Quality of solution
    /// @note This is fully general solver for the normal equations for any shape
    /// parameters.        
    bool LinearSolver::solveNormalEquations(Params &params, Quality& quality)
    {
      ASKAPTRACE("LinearSolver::solveNormalEquations");
      
// Solving A^T Q^-1 V = (A^T Q^-1 A) P
      int nParameters=0;

// Find all the free parameters
      vector<string> names(params.freeNames());
      if (names.size() == 0) {
          // list of parameters is empty, will solve for all 
          // unknowns in the equation 
          names = normalEquations().unknowns();
      }
      ASKAPCHECK(names.size()>0, "No free parameters in Linear Solver");

      map<string, int> indices;
      for (vector<string>::const_iterator it=names.begin();it!=names.end();it++)
      {
        indices[*it] = nParameters;
        const casa::uInt newParameters = normalEquations().dataVector(*it).nelements();
        nParameters += newParameters;
        ASKAPDEBUGASSERT((params.isFree(*it) ? params.value(*it).nelements() : newParameters) == newParameters);        
      }
      ASKAPCHECK(nParameters>0, "No free parameters in Linear Solver");

// Convert the normal equations to gsl format
      gsl_matrix * A = gsl_matrix_alloc (nParameters, nParameters);
      gsl_vector * B = gsl_vector_alloc (nParameters);
      gsl_vector * X = gsl_vector_alloc (nParameters);

      for (map<string, int>::const_iterator indit2=indices.begin();indit2!=indices.end();indit2++)
      {
        for (map<string, int>::const_iterator indit1=indices.begin();indit1!=indices.end();indit1++)
        {
// Axes are dof, dof for each parameter
// Take a deep breath for const-safe indexing into the double layered map
          const casa::Matrix<double>& nm = normalEquations().normalMatrix(indit1->first, indit2->first);
          
          for (size_t row=0; row<nm.nrow(); ++row)
          {
            for (size_t col=0; col<nm.ncolumn(); ++col)
            {
              gsl_matrix_set(A, row+(indit1->second), col+(indit2->second), nm(row,col));
//              std::cout << "A " << row << " " << col << " " << nm(row,col) << std::endl; 
            }
          }
        }
      }
      for (map<string, int>::const_iterator indit1=indices.begin();indit1!=indices.end();indit1++)
      {
        const casa::Vector<double> &dv = normalEquations().dataVector(indit1->first);
        for (size_t row=0; row<dv.nelements(); ++row)
        {
          gsl_vector_set(B, row+(indit1->second), dv(row));
//          std::cout << "B " << row << " " << dv(row) << std::endl; 
        }
      }
      
      /*
      // temporary code to export matrices, which cause problems with the GSL
      // to write up a clear case
      {
        std::ofstream os("dbg.dat");
        os<<nParameters<<std::endl;
        for (int row=0;row<nParameters;++row) {
             for (int col=0;col<nParameters;++col) {
                  if (col) {
                      os<<" ";
                  } 
                  os<<gsl_matrix_get(A,row,col);
             }
             os<<std::endl;
        }
      }
      // end of the temporary code
      */
      
      if(algorithm()=="SVD")
      {  
        gsl_matrix * V = gsl_matrix_alloc (nParameters, nParameters);
        ASKAPDEBUGASSERT(V!=NULL);
        gsl_vector * S = gsl_vector_alloc (nParameters);
        ASKAPDEBUGASSERT(S!=NULL);
        gsl_vector * work = gsl_vector_alloc (nParameters);
        ASKAPDEBUGASSERT(work!=NULL);
        
        gsl_linalg_SV_decomp (A, V, S, work);
        
        //SVDecomp (A, V, S);
        
        // code to put a limit on the condition number of the system
        const double singularValueLimit = nParameters>1 ? 
                    gsl_vector_get(S,0)/itsMaxCondNumber : -1.; 
        for (int i=1; i<nParameters; ++i) {
             if (gsl_vector_get(S,i)<singularValueLimit) {
                 gsl_vector_set(S,i,0.);
             }
        }
        
        /*
        // temporary code for debugging
        {
          std::ofstream os("dbg2.dat");
          for (int i=0; i<nParameters; ++i) {
               os<<i<<" "<<gsl_vector_get(S,i)<<std::endl;
          } 
          
          std::cout<<"new singular value spectrum is ready"<<std::endl;
          char tst;
          std::cin>>tst;
          
        }
        // end of temporary code
        */
        
        gsl_vector * X = gsl_vector_alloc(nParameters);
        ASKAPDEBUGASSERT(X!=NULL);
        
        gsl_linalg_SV_solve (A, V, S, B, X);
        
// Now find the statistics for the decomposition
        int rank=0;
        double smin=1e50;
        double smax=0.0;
        for (int i=0;i<nParameters;i++)
        {
          double sValue=std::abs(gsl_vector_get(S, i));
          if(sValue>0.0)
          {
            rank++;
            if(sValue>smax) smax=sValue;
            if(sValue<smin) smin=sValue;
          }
        }
        quality.setDOF(nParameters);
        quality.setRank(rank);
        quality.setCond(smax/smin);
        if(rank==nParameters)
        {
          quality.setInfo("SVD decomposition rank complete");
        }
        else
        {
          quality.setInfo("SVD decomposition rank deficient");
        }
// Update the parameters for the calculated changes. Exploit reference
// semantics of casa::Array.
        map<string, int>::const_iterator indit;
        for (indit=indices.begin();indit!=indices.end();++indit)
        {
          casa::IPosition vecShape(1, params.value(indit->first).nelements());
          casa::Vector<double> value(params.value(indit->first).reform(vecShape));
          for (size_t i=0; i<value.nelements(); ++i)
          {
//          	std::cout << value(i) << " " << gsl_vector_get(X, indit->second+i) << std::endl;
            value(i)+=gsl_vector_get(X, indit->second+i);
          }
        }
        gsl_vector_free(S);
        gsl_vector_free(work);
        gsl_matrix_free(V);
      }
      else
      {
        quality.setInfo("Cholesky decomposition");
        gsl_linalg_cholesky_decomp(A);
        gsl_linalg_cholesky_solve(A, B, X);
// Update the parameters for the calculated changes
        map<string, int>::const_iterator indit;
        for (indit=indices.begin();indit!=indices.end();++indit)
        {
          casa::IPosition vecShape(1, params.value(indit->first).nelements());
          casa::Vector<double> value(params.value(indit->first).reform(vecShape));
          for (size_t i=0; i<value.nelements(); ++i)
          {
            value(i)+=gsl_vector_get(X, indit->second+i);
          }
        }
      }

// Free up gsl storage
      gsl_vector_free(B);
      gsl_matrix_free(A);
      gsl_vector_free(X);
        
      return true;
    };

    Solver::ShPtr LinearSolver::clone() const
    {
      return Solver::ShPtr(new LinearSolver(*this));
    }

  }
}
