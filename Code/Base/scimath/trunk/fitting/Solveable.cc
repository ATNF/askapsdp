/// @file
///
/// Holds common control parameters for Solvers
///
/// (c) 2007 ASKAP, All Rights Reserved.
/// @author Tim Cornwell tim.cornwel@csiro.au
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
