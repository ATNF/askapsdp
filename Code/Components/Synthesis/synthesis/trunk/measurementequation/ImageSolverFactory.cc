#include <askap_synthesis.h>
#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, ".measurementequation");

#include <askap/AskapError.h>

#include <measurementequation/ImageSolverFactory.h>

#include <measurementequation/ImageSolver.h>
#include <measurementequation/ImageMultiScaleSolver.h>
#include <measurementequation/ImageMSMFSolver.h>
#include <measurementequation/IImagePreconditioner.h>
#include <measurementequation/WeinerPreconditioner.h>

using namespace askap::scimath;

using namespace LOFAR::ACC::APS;

namespace askap
{
  namespace synthesis
  {

    ImageSolverFactory::ImageSolverFactory()
    {
    }

    ImageSolverFactory::~ImageSolverFactory()
    {
    }
    
    Solver::ShPtr ImageSolverFactory::make(askap::scimath::Params &ip, const LOFAR::ACC::APS::ParameterSet &parset) {
      ImageSolver::ShPtr solver;
      if(parset.getString("solver")=="Clean") {
        std::vector<float> defaultScales(3);
        defaultScales[0]=0.0;
        defaultScales[1]=10.0;
        defaultScales[2]=30.0;
        
	string algorithm=parset.getString("solver.Clean.algorithm","MultiScale");
	std::vector<float> scales=parset.getFloatVector("solver.Clean.scales", defaultScales);
	
	if(algorithm=="MSMFS"){
          int nterms=parset.getInt32("solver.Clean.nterms",2);
          solver = ImageSolver::ShPtr(new ImageMSMFSolver(ip, casa::Vector<float>(scales),int(nterms)));
          ASKAPLOG_INFO_STR(logger, "Constructed image multiscale multi-frequency solver" );
          solver->setAlgorithm(algorithm);
	}
	else{
          solver = ImageSolver::ShPtr(new ImageMultiScaleSolver(ip, casa::Vector<float>(scales)));
          ASKAPLOG_INFO_STR(logger, "Constructed image multiscale solver" );
          //solver->setAlgorithm(algorithm);
          solver->setAlgorithm(parset.getString("solver.Clean.algorithm", "MultiScale"));
	}
        
	solver->setTol(parset.getFloat("solver.Clean.tolerance", 0.1));
        solver->setGain(parset.getFloat("solver.Clean.gain", 0.7));
        solver->setVerbose(parset.getBool("solver.Clean.verbose", true));
        solver->setNiter(parset.getInt32("solver.Clean.niter", 100));
        casa::Quantity threshold;
        casa::Quantity::read(threshold, parset.getString("solver.Clean.threshold", "0Jy"));
        solver->setThreshold(threshold);
      }
      else {
        solver = ImageSolver::ShPtr(new ImageSolver(ip));
        casa::Quantity threshold;
        casa::Quantity::read(threshold, parset.getString("solver.Dirty.threshold", "0Jy"));
        solver->setTol(parset.getFloat("solver.Dirty.tolerance", 0.1));
        solver->setThreshold(threshold);
        ASKAPLOG_INFO_STR(logger, "Constructed dirty image solver" );
      }
      
      // Set Up the Preconditioners - a whole list of 'em
      const vector<string> preconditioners=parset.getStringVector("preconditioner.Names",std::vector<std::string>());
      if(preconditioners.size())
      {
        for (vector<string>::const_iterator pc = preconditioners.begin(); pc != preconditioners.end(); ++pc) 
        {
          if( (*pc)=="Weiner" )
	  {
	    float noisepower = parset.getFloat("preconditioner.Weiner.noisepower",0.0);
            solver->addPreconditioner(IImagePreconditioner::ShPtr(new WeinerPreconditioner(noisepower)));
	  }
	  /*
	     if( (*pc)=="UVTaper" ) // take input in units of "klambda". Make this a "measures" thing...
	     {
	     float uvwidth = parset.getFloat("preconditioner.UVTaper.uvwidth",);
	     solver->addPreconditioner(IPreconditioner::ShPtr(new UVTaperPreconditioner(uvwidth)));
	     }
	     if( (*pc)=="ApproxPsf" ) //later, add the option of specifying a beam, or fitting for it.
	     {
	     solver->addPreconditioner(IPreconditioner::ShPtr(new ApproxPsfPreconditioner()));
	     }
	   */
	}
      }
      else
      {
        solver->addPreconditioner(IImagePreconditioner::ShPtr(new WeinerPreconditioner()));
      }
      
      return solver;
    }
  }
}
