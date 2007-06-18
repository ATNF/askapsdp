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

#include <dataaccess/TableConstDataSource.h>

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
     
    TableConstDataSource ds(argv[1]);

    cout << "Synthesis demonstration program" << endl;

// Get the nvss model
    ParamsCASATable pt("nvss.par", true);
    Params nvsspar(ComponentEquation::defaultParameters());
    pt.getParameters(nvsspar);
    std::cout << "Read parameters" << std::endl;

// Predict the visibilities for the nvss model
//    it->chooseBuffer("model");
//    for (IConstDataSharedIter it=ds.createConstIterator();it!=it.end();++it) {
//      ComponentEquation ce(nvsspar, it);
//      ce.predict();
//    }
    
// Define an image
    Params imagepar;
    SynthesisParamsHelper::add(imagepar, "image.i.nvss", 
      12.5*casa::C::hour, 45.0*casa::C::degree, 12.0*casa::C::arcsec,
      1024, 1024, 1.420e9-256.0e9, 1.420e9, 1);

    std::cout << "Defined image" << std::endl;
    
    NormalEquations ne(imagepar);
    std::cout << "Constructed normal equations" << std::endl;
    ImageSolver is(imagepar);
    std::cout << "Defined image solver" << std::endl;
//    for (IConstDataSharedIter it=ds.createConstIterator();it!=it.end();++it) {
//      ImageFFTEquation ie(imagepar, it);
//      ce.calcNormalEquations(ne);
//      is.addNormalEquation(ne);
//    }
    Quality q;
    std::cout << "Solving normal equations" << std::endl;
    is.solveNormalEquations(q);
    std::cout << q << std::endl;
    
    ParamsCASATable result("dSynthesis.par");
    result.setParameters(is.parameters());
    
    std::cout << "Finished imaging" << std::endl;
    exit(0);
  }
  catch (std::exception& x)
  {
    std::cout << "Unexpected exception in " << argv[0] << ": " << x.what() << std::endl;
    exit(1);
  }
};
