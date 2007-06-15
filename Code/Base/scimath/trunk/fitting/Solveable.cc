#include <fitting/Solveable.h>

using std::string;

namespace conrad
{
  namespace scimath
  {

/// Standard constructor
    Solveable::Solveable(const double gain, const int niter, const double tol, 
      const std::string& algorithm,
      const std::string& subalgorithm) : itsGain(gain), itsNiter(niter), itsTol(tol),
      itsAlgorithm(algorithm), itsSubAlgorithm(subalgorithm)
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
    std::string Solveable::algorithm() {return itsAlgorithm;};
    void Solveable::setAlgorithm(const std::string& algorithm) {itsAlgorithm=algorithm;};

/// Get and set subalgorithm
    std::string Solveable::subAlgorithm() {return itsSubAlgorithm;};
    void Solveable::setSubAlgorithm(const std::string& subalgorithm) {itsSubAlgorithm=subalgorithm;};

  }
}
