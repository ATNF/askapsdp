//
// @file : Evolving demonstration program for synthesis capabilities
//
#include <measurementequation/ComponentEquation.h>
#include <measurementequation/ImageFFTEquation.h>
#include <measurementequation/ImageSolver.h>

#include <measurementequation/SynthesisParamsHelper.h>

#include <fitting/CompositeEquation.h>
#include <fitting/ParamsCASATable.h>
#include <fitting/Axes.h>

//#include <dataaccess/TableConstDataSource.h>
#include <dataaccess/DataIteratorStub.h>

#include <casa/aips.h>
#include <casa/BasicSL/Constants.h>
#include <casa/Arrays/Cube.h>

#include <stdexcept>
#include <iostream>

using std::cout;
using std::endl;

using namespace conrad::scimath;
using namespace conrad::synthesis;

int main(int argc, const char** argv)
{
  try
  {
     if (argc!=2) {
        std::cerr<<"Usage "<<argv[0]<<" measurement_set"<<std::endl;
        exit(1);
     }
     
//    TableConstDataSource ds(argv[1]);

    cout << "Synthesis demonstration program" << endl;

// Get the nvss model - fix all the parameters
    ParamsCASATable pt("nvss.par", true);
    Params nvsspar(ComponentEquation::defaultParameters());
    pt.getParameters(nvsspar);
    std::cout << "Read NVSS model" << std::endl;
    vector<string> names(nvsspar.freeNames());
    std::cout << "Number of free parameters in NVSS model = " << names.size() << std::endl;
    for (vector<string>::iterator it=names.begin();it!=names.end();it++) {
      nvsspar.fix(*it);
    }
    IDataSharedIter idi = IDataSharedIter(new DataIteratorStub(1));    
    
    CompositeEquation me(nvsspar);
    ComponentEquation ce(nvsspar, idi);
    ImageFFTEquation ie(nvsspar, idi);
    me.add(ce);
    me.add(ie);

// Predict the visibilities for the nvss model
//    it->chooseBuffer("model");
//    for (it=ds.createConstIterator();it!=it.end();++it) {
      ce.predict();
//    }
    
// Define an image
    SynthesisParamsHelper::add(nvsspar, "image.i.nvss", 
      12.5*casa::C::hour, 45.0*casa::C::degree, 12.0*casa::C::arcsec,
      3*1024, 3*1024, 1.420e9-256.0e6, 1.420e9, 1);

    std::cout << "Added NVSS image to model " << std::endl;
    std::cout << "Number of free parameters now = " << nvsspar.freeNames().size() << std::endl;
    
    NormalEquations ne(nvsspar);
    std::cout << "Constructed normal equations" << std::endl;
    
    ImageSolver is(nvsspar);
    std::cout << "Constructed image solver" << std::endl;
//    for (it=ds.createConstIterator();it!=it.end();++it) {
      me.calcEquations(ne);
      std::cout << "Calculated normal equations" << std::endl;
      is.addNormalEquations(ne);
      std::cout << "Added normal equations to solver" << std::endl;
//    }
    Quality q;
    std::cout << "Solving normal equations" << std::endl;
    is.solveNormalEquations(q);
    std::cout << q << std::endl;
    
    {
      ParamsCASATable result("dSynthesis.par", false);
      result.setParameters(is.parameters());
    }
    
    std::cout << "Finished imaging" << std::endl;
    exit(0);
  }
  catch (std::exception& x)
  {
    std::cout << "Unexpected exception in " << argv[0] << ": " << x.what() << std::endl;
    exit(1);
  }
};
