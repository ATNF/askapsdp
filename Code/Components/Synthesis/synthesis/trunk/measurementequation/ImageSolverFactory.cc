#include <measurementequation/ImageSolverFactory.h>

#include <measurementequation/ImageSolver.h>
#include <measurementequation/ImageMultiScaleSolver.h>

using namespace conrad::scimath;

using namespace LOFAR::ACC::APS;

namespace conrad
{
  namespace synthesis
  {

    ImageSolverFactory::ImageSolverFactory()
    {
    }

    ImageSolverFactory::~ImageSolverFactory()
    {
    }
    
    Solver::ShPtr ImageSolverFactory::make(conrad::scimath::Params &ip, const LOFAR::ACC::APS::ParameterSet &parset) {
      Solver::ShPtr solver;
      if(parset.getString("solver")=="Clean") {
        std::vector<float> defaultScales(3);
        defaultScales[0]=0.0;
        defaultScales[1]=10.0;
        defaultScales[2]=30.0;
        std::vector<float> scales=parset.getFloatVector("solver.Clean.scales", defaultScales);
        solver = Solver::ShPtr(new ImageMultiScaleSolver(ip, casa::Vector<float>(scales)));
        std::cout << "Constructed image multiscale solver" << std::endl;
        solver->setTol(parset.getFloat("solver.Clean.tolerance", 0.1));
        solver->setGain(parset.getFloat("solver.Clean.gain", 0.7));
        solver->setAlgorithm(parset.getString("solver.Clean.algorithm", "MultiScale"));
        solver->setVerbose(parset.getBool("solver.Clean.verbose", true));
        solver->setNiter(parset.getInt32("solver.Clean.niter", 100));
        casa::Quantity threshold;
        casa::Quantity::read(threshold, parset.getString("solver.Clean.threshold", "0Jy"));
        solver->setThreshold(threshold);
      }
      else {
        solver = Solver::ShPtr(new ImageSolver(ip));
        casa::Quantity threshold;
        casa::Quantity::read(threshold, parset.getString("solver.Dirty.threshold", "0Jy"));
        solver->setTol(parset.getFloat("solver.Dirty.tolerance", 0.1));
        solver->setThreshold(threshold);
        std::cout << "Constructed dirty image solver" << std::endl;
      }
      return solver;
    }
  }
}
