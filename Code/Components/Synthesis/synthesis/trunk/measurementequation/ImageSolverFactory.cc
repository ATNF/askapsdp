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
    
    Solver::ShPtr ImageSolverFactory::make(Params& ip, const ParameterSet& parset) {
      Solver::ShPtr solver;
      if(parset.getString("solver")=="Clean") {
        solver = Solver::ShPtr(new ImageMultiScaleSolver(ip));
        std::cout << "Constructed image multiscale solver" << std::endl;
        solver->setGain(parset.getFloat("solver.gain", 0.7));
        solver->setAlgorithm(parset.getString("solver.algorithm", "MultiScale"));
  //      std::vector<float> scales(1); scales[0]=0;
  //      dynamic_cast<ImageMultiScaleSolver*>(*solver)->setScales(parset.getFloatVector("solver.scales", scales));
  
      }
      else {
        solver = Solver::ShPtr(new ImageSolver(ip));
        std::cout << "Constructed dirty image solver" << std::endl;
      }
      solver->setVerbose(parset.getBool("solver.verbose", true));
      solver->setNiter(parset.getInt32("solver.niter", 100));
      return solver;
    }
  }
}
