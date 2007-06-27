/// @file Solveable.h
///
/// Solveable: Manage control for an iterative solver
///
/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef SCIMATHSOLVEABLE_H_
#define SCIMATHSOLVEABLE_H_

#include <string>

namespace conrad
{
  namespace scimath
  {

/// @brief Base class for solveables
///
/// Derive solveable classes from this base to have standard parameters available.
/// None of these have standard meanings. Similarly the names of the algorithms
/// are not specified here.

    class Solveable
    {
      public:
/// Standard constructor
        Solveable();
          
        virtual ~Solveable();

/// Get gain
        double gain();
/// Set gain
/// @param gain Gain to be set
        void setGain(const double gain=1.0);

/// Get number of iterations
        int niter();
/// Set number of iterations
/// @param niter Number of iterations
        void setNiter(const int niter=1);

/// Get tolerance for solution
        double tol();
        /// Set tolerance for solution
        /// @param tol Tolerance for solution
        void setTol(const double tol=1e-6);

/// Get algorithm e.g. "SVD" or "MSClean"
        std::string algorithm();
        /// Set algorithm
        /// @param algorithm Name of algrithm
        void setAlgorithm(const std::string& algorithm=std::string(""));

/// Get subalgorithm
        std::string subAlgorithm();
        /// Set subalgorithm
        /// @param subalgorithm Name of subalgrithm
        void setSubAlgorithm(const std::string& subalgorithm=std::string(""));

      private:
/// Algorithm name
        std::string itsAlgorithm;
        /// Sub algorithm name
        std::string itsSubAlgorithm;
        /// Gain
        double itsGain;
        /// Number of iterations
        int itsNiter;
        /// Tolerance for solution
        double itsTol;
    };

  }
}
#endif
