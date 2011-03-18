/// @file Solveable.h
///
/// Solveable: Manage control for an iterative solver
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
#ifndef SCIMATHSOLVEABLE_H_
#define SCIMATHSOLVEABLE_H_

#include <string>
#include <casa/aips.h>
#include <casa/BasicSL/Constants.h>
#include <casa/Quanta.h>

namespace askap
{
  namespace scimath
  {

/// @brief Base class for solveables
///
/// Derive solveable classes from this base to have standard parameters available.
/// None of these have standard meanings. Similarly the names of the algorithms
/// are not specified here.
/// @ingroup fitting
    class Solveable
    {
      public:
/// Standard constructor
        explicit Solveable();
          
        virtual ~Solveable();

/// Get gain
        double gain() const;
/// Set gain
/// @param gain Gain to be set
        void setGain(const double gain=1.0);

/// Get number of iterations
        int niter() const;
/// Set number of iterations
/// @param niter Number of iterations
        void setNiter(const int niter=1);

/// Get tolerance for solution
        double tol() const;
        /// Set tolerance for solution
        /// @param tol Tolerance for solution
        void setTol(const double tol=1e-6);

/// Get algorithm e.g. "SVD" or "MSClean"
        const std::string& algorithm() const;
        /// Set algorithm
        /// @param algorithm Name of algrithm
        void setAlgorithm(const std::string& algorithm=std::string(""));

/// Get subalgorithm
        const std::string& subAlgorithm() const;
        /// Set subalgorithm
        /// @param subalgorithm Name of subalgrithm
        void setSubAlgorithm(const std::string& subalgorithm=std::string(""));
        
/// Set verbose flag
/// @param verbose True for lots of output
        void setVerbose(bool verbose=true);
/// Return verbose flag
        bool verbose() const;

/// Set threshold: if the absolute value of the maximum residual is less
/// than this number, iteration will stop.
/// @param threshold Quantity
        void setThreshold(const casa::Quantity& threshold=casa::Quantity(0.0, "Jy"));

/// Return threshold
        const casa::Quantity& threshold() const;

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
        /// Verbose output?
        bool itsVerbose;
        /// Threshold
        casa::Quantity itsThreshold;
    };

  }
}
#endif
