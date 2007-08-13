/// @file
/// @brief A method to set up images from a parset file
/// @details Parameters are currently passed around using parset files.
/// The methods declared in this file set up images 
/// from the ParameterSet object. This is probably a temporary solution.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///

#include <measurementequation/ParsetInterface.h>
#include <measurementequation/SynthesisParamsHelper.h>
#include <measurementequation/ImageSolver.h>
#include <measurementequation/ImageMultiScaleSolver.h>

#include <iostream>

void conrad::synthesis::operator<<(conrad::scimath::Params& params,
                 const LOFAR::ACC::APS::ParameterSet &parset)
{
  vector<string> images=parset.getStringVector("Names");
  for (vector<string>::iterator it=images.begin();it!=images.end();it++)
  {
    std::vector<int> shape=parset.getInt32Vector(*it+".shape");
    int nchan=parset.getInt32(*it+".nchan");
    std::vector<double> freq=parset.getDoubleVector(*it+".frequency");
    std::vector<std::string> direction=parset.getStringVector(*it+".direction");
    std::vector<std::string> cellsize=parset.getStringVector(*it+".cellsize");
    
    SynthesisParamsHelper::add(params, *it, direction, cellsize, shape, 
      freq[0], freq[1], nchan);
  }
}

void conrad::synthesis::operator<<(conrad::scimath::Solver::ShPtr& solver, const LOFAR::ACC::APS::ParameterSet &parset) {
  conrad::scimath::Params params;
  if(parset.getString("solver")=="Clean") {
    std::vector<float> defaultScales(3);
    defaultScales[0]=0.0;
    defaultScales[1]=10.0;
    defaultScales[2]=30.0;
    std::vector<float> scales=parset.getFloatVector("solver.Clean.scales", defaultScales);
    solver = conrad::scimath::Solver::ShPtr(new ImageMultiScaleSolver(params, casa::Vector<float>(scales)));
    std::cout << "Constructed image multiscale solver" << std::endl;
    solver->setGain(parset.getFloat("solver.Clean.gain", 0.7));
    solver->setAlgorithm(parset.getString("solver.Clean.algorithm", "MultiScale"));
    solver->setVerbose(parset.getBool("solver.Clean.verbose", true));
    solver->setNiter(parset.getInt32("solver.Clean.niter", 100));
    casa::Quantity threshold;
    casa::Quantity::read(threshold, parset.getString("solver.Clean.threshold", "0Jy"));
    solver->setThreshold(threshold);
  }
  else {
    solver = conrad::scimath::Solver::ShPtr(new ImageSolver(params));
    casa::Quantity threshold;
    casa::Quantity::read(threshold, parset.getString("solver.Dirty.threshold", "0Jy"));
    solver->setThreshold(threshold);
    std::cout << "Constructed dirty image solver" << std::endl;
  }
}

