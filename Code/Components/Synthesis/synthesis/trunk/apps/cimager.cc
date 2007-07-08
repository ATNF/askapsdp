//
// @file : Evolving CONRAD program for synthesis capabilities
//
#include <conrad/ConradError.h>

#include <measurementequation/ImageFFTEquation.h>
#include <measurementequation/SynthesisParamsHelper.h>

#include <measurementequation/ImageSolverFactory.h>
#include <gridding/VisGridderFactory.h>

#include <fitting/ParamsCasaTable.h>
#include <fitting/Axes.h>

#include <dataaccess/DataAccessError.h>
#include <dataaccess/TableDataSource.h>

#include <casa/aips.h>
#include <casa/BasicSL/Constants.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/OS/Timer.h>

#include <APS/ParameterSet.h>

#include <stdexcept>
#include <iostream>

using std::cout;
using std::endl;

using namespace conrad;
using namespace conrad::scimath;
using namespace conrad::synthesis;
using namespace LOFAR::ACC::APS;

int main(int argc, const char** argv)
{
  try
  {

    cout << "CONRAD synthesis imaging program" << endl;

    casa::Timer timer;
    timer.mark();
    
    string parsetname("cimager.in");
    if (argc==2)
    {
      parsetname=argv[1];
    }

    ParameterSet parset(parsetname);
    vector<string> ms=parset.getStringVector("DataSet");

    /// Create the specified images from the definition in the
    /// parameter set
    Params skymodel;
    SynthesisParamsHelper::add(skymodel, parset, "Images.");
    
    /// Create the gridder and solver using factories acting on 
    /// parametersets
    ParameterSet subset(parset.makeSubset("Cimager."));
    IVisGridder::ShPtr gridder=VisGridderFactory::make(subset);
    Solver::ShPtr solver=ImageSolverFactory::make(skymodel, subset);

    NormalEquations ne(skymodel);
    std::cout << "Constructed normal equations" << std::endl;

    // Now do the required number of major cycles
    int nCycles(parset.getInt32("Cimager.solver.cycles", 10));
    for (int cycle=0;cycle<nCycles;cycle++) {
      
      if(nCycles>1) {
        std::cout << "*** Starting major cycle " << cycle << " ***" << std::endl;
      }
    
      /// Now iterate through all data sets
      /// @todo Convert this to a factory
      for (vector<string>::iterator thisms=ms.begin();thisms!=ms.end();++thisms) {
        std::cout << "Processing data set " << *thisms << std::endl;
        TableDataSource ds(*thisms);
        IDataSelectorPtr sel=ds.createSelector();
        IDataConverterPtr conv=ds.createConverter();
        conv->setFrequencyFrame(casa::MFrequency::Ref(casa::MFrequency::TOPO),"Hz");
        IDataSharedIter it=ds.createIterator(sel, conv);
        it.init();
        it.chooseOriginal();
        ImageFFTEquation ie(skymodel, it, gridder);
        std::cout << "Constructed measurement equation" << std::endl;

        ie.calcEquations(ne);
        std::cout << "Calculated normal equations" << std::endl;
        solver->addNormalEquations(ne);
        std::cout << "Added normal equations to solver" << std::endl;
      }

      // Perform the solution
      Quality q;
      std::cout << "Solving normal equations" << std::endl;
      solver->solveNormalEquations(q);
      std::cout << "Solved normal equations" << std::endl;
      skymodel=solver->parameters();

      vector<string> resultimages=skymodel.names();
      for (vector<string>::iterator it=resultimages.begin();it!=resultimages.end();it++)
      {
        casa::Array<double> resultImage(skymodel.value(*it));
        std::cout << *it << std::endl
          << "Maximum = " << max(resultImage) << ", minimum = " << min(resultImage) << std::endl;
      }
    }

    // Now write the results to a table
    string resultfile(parset.getString("Parms.Result"));
    ParamsCasaTable results(resultfile, false);
    results.setParameters(skymodel);
 
    // And write the images to CASA image files           
    vector<string> resultimages=skymodel.names();
    for (vector<string>::iterator it=resultimages.begin();it!=resultimages.end();it++)
    {
      SynthesisParamsHelper::saveAsCasaImage(skymodel, *it, *it);
      
    }
    std::cout << "Finished imaging" << std::endl;
    std::cout << "user:   " << timer.user () << std::endl; 
    std::cout << "system: " << timer.system () << std::endl;
    std::cout << "real:   " << timer.real () << std::endl; 

    exit(0);
  }
  catch (conrad::ConradError& x)
  {
    std::cout << "Conrad error in " << argv[0] << ": " << x.what() << std::endl;
    exit(1);
  }
  catch (std::exception& x)
  {
    std::cout << "Unexpected exception in " << argv[0] << ": " << x.what() << std::endl;
    exit(1);
  }
};
