/// @file Solveable.h
///
/// Solveable: Manage control for an iterative solver
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef SCIMATHSOLVEABLE_H_
#define SCIMATHSOLVEABLE_H_

#include <string>

namespace conrad
{
  namespace scimath
  {

    class Solveable
/// Derive isolveable classes from this base to have standard parameters available.
    {
      public:
/// Standard constructor
        Solveable(const double gain=0.1, const int niter=100, const double tol=1e-6, const std::string algorithm=std::string(""),
          const std::string subalgorithm=std::string(""));
        virtual ~Solveable();

/// Get and set gain
        double gain();
        void setGain(const double gain=1.0);

/// Get and set number of iterations
        int niter();
        void setNiter(const int niter=1);

/// Get and set tolerance for solution
        double tol();
        void setTol(const double tol=1e-6);

/// Get and set algorithm e.g. "SVD" or "MSClean"
        std::string algorithm();
        void setAlgorithm(const std::string algorithm=std::string(""));

/// Get and set subalgorithm
        std::string subAlgorithm();
        void setSubAlgorithm(const std::string subalgorithm=std::string(""));

      private:
        std::string itsAlgorithm;
        std::string itsSubAlgorithm;
        double itsGain;
        int itsNiter;
        double itsTol;
    };

  }
}
#endif
