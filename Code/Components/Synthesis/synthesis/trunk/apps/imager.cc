//
// @file : Evolving CONRAD program for synthesis capabilities
//
#include <conrad/ConradError.h>

#include <measurementequation/ComponentEquation.h>
#include <measurementequation/ImageFFTEquation.h>
#include <measurementequation/SynthesisParamsHelper.h>

#include <measurementequation/ImageSolverFactory.h>
#include <gridding/VisGridderFactory.h>

#include <fitting/CompositeEquation.h>
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
    
    string parsetname("imager.in");
    if (argc==2)
    {
      parsetname=argv[1];
    }

    ParameterSet parset(parsetname);
    string ms=parset.getString("DataSet");

    Params skymodel;

    /// Load the local sky model if it has been specified
    if(parset.isDefined("Parms.LocalSky")&&parset.getString("Parms.LocalSky")!="")
    {
      string localsky(parset.getString("Parms.LocalSky"));
      ParamsCasaTable pt(localsky, true);
      Params localskypar(ComponentEquation::defaultParameters());
      pt.getParameters(localskypar);
      std::cout << "Read Local Sky model " << localsky << std::endl;
      vector<string> names(localskypar.freeNames());
      std::cout << "Number of free parameters in Local Sky model = " << names.size() << std::endl;
      for (vector<string>::iterator it=names.begin();it!=names.end();it++)
      {
        localskypar.fix(*it);
      }
      skymodel.merge(localskypar);
    }

    /// Create the specified images
    SynthesisParamsHelper::add(skymodel, parset, "Images.");
    
    /// Create the gridder and solver
    ParameterSet subset(parset.makeSubset("Imager."));
    IVisGridder::ShPtr gridder=VisGridderFactory::make(subset);
    Solver::ShPtr solver=ImageSolverFactory::make(skymodel, subset);
    
    /// Create data iterator
    TableDataSource ds(ms);
    IDataSelectorPtr sel=ds.createSelector();
    IDataConverterPtr conv=ds.createConverter();
    conv->setFrequencyFrame(casa::MFrequency::Ref(casa::MFrequency::TOPO),"Hz");
    IDataSharedIter it=ds.createIterator(sel, conv);
    
    it.init();
    it.chooseOriginal();

    NormalEquations ne(skymodel);
    std::cout << "Constructed normal equations" << std::endl;

    int nCycles(parset.getInt32("Imager.solver.cycles", 10));
    
    for (int cycle=0;cycle<nCycles;cycle++) {
      if(nCycles>1) {
        std::cout << "*** Starting major cycle " << cycle << " ***" << std::endl;
      }
      ImageFFTEquation ie(skymodel, it, gridder);
      ie.calcEquations(ne);
      std::cout << "Calculated normal equations" << std::endl;

      string resultfile(parset.getString("Parms.Result"));
      ParamsCasaTable results(resultfile, false);

      Quality q;
      std::cout << "Solving normal equations" << std::endl;
      solver->addNormalEquations(ne);
      std::cout << "Added normal equations to solver" << std::endl;
      solver->solveNormalEquations(q);
      skymodel=solver->parameters();
      results.setParameters(skymodel);

      std::cout << "Number of degrees of freedom = " << q.DOF() << std::endl;
    }

    vector<string> resultimages=skymodel.names();
    for (vector<string>::iterator it=resultimages.begin();it!=resultimages.end();it++)
    {
      casa::Array<double> resultImage(skymodel.value(*it));
      std::cout << *it << std::endl
        << "Maximum = " << max(resultImage) << ", minimum = " << min(resultImage) << std::endl
        << "Axes " << skymodel.axes(*it) << std::endl;
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
