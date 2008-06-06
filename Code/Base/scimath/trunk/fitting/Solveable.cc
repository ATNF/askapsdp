/// @file
///
/// Holds common control parameters for Solvers
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
#include <fitting/Solveable.h>

using std::string;

namespace askap
{
  namespace scimath
  {

/// Standard constructor
    Solveable::Solveable() : itsAlgorithm(""), itsSubAlgorithm(""), 
      itsGain(0.1), itsNiter(100), itsTol(1e-6), itsVerbose(false),
      itsThreshold(casa::Quantity(0.0, "Jy"))
    {
    };

    Solveable::~Solveable(){};

/// Get and set gain
    double Solveable::gain() {return itsGain;}
    void Solveable::setGain(const double gain) {itsGain=gain;};

/// Get and set number of iterations
    int Solveable::niter() {return itsNiter;}
    void Solveable::setNiter(const int niter) {itsNiter=niter;};

/// Get and set tolerance for solution
    double Solveable::tol() {return itsTol;};
    void Solveable::setTol(const double tol) {itsTol=tol;};

/// Get and set algorithm
    const std::string& Solveable::algorithm() {return itsAlgorithm;};
    void Solveable::setAlgorithm(const std::string& algorithm) {itsAlgorithm=algorithm;};

/// Get and set subalgorithm
    const std::string& Solveable::subAlgorithm() {return itsSubAlgorithm;};
    void Solveable::setSubAlgorithm(const std::string& subalgorithm) {itsSubAlgorithm=subalgorithm;};
    
    void Solveable::setVerbose(bool verbose) {itsVerbose=verbose;};
    bool Solveable::verbose() {return itsVerbose;};

    void Solveable::setThreshold(const casa::Quantity& threshold) 
    {
      itsThreshold=threshold;
    }

    const casa::Quantity& Solveable::threshold() 
    {
      return itsThreshold;
    }

    

  }
}
