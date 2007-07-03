//
// @file : Evolving CONRAD program for synthesis capabilities
//
#include <conrad/ConradError.h>

#include <measurementequation/ComponentEquation.h>
#include <measurementequation/ImageFFTEquation.h>
#include <measurementequation/ImageSolver.h>
#include <measurementequation/ImageMultiScaleSolver.h>
#include <measurementequation/SynthesisParamsHelper.h>

#include <gridding/IVisGridder.h>
#include <gridding/BoxVisGridder.h>
#include <gridding/SphFuncVisGridder.h>
#include <gridding/AntennaIllumVisGridder.h>

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
    vector<string> images=parset.getStringVector("Images.Names");
    for (vector<string>::iterator it=images.begin();it!=images.end();it++)
    {
      std::cout << "Defining image " << *it << std::endl;
      std::vector<int> shape=parset.getInt32Vector("Images."+*it+".shape");
      int nchan=parset.getInt32("Images."+*it+".nchan");
      std::vector<double> freq=parset.getDoubleVector("Images."+*it+".frequency");
      std::vector<std::string> direction=parset.getStringVector("Images."+*it+".direction");
      std::vector<std::string> cellsize=parset.getStringVector("Images."+*it+".cellsize");
      
      SynthesisParamsHelper::add(skymodel, *it, direction, cellsize, shape, 
        freq[0], freq[1], nchan);
    }

    TableDataSource ds(ms);

    /// Now set up the imager
    
    IVisGridder::ShPtr gridder;
    if(parset.getString("Imager.gridder")=="AntennaIllum") {
      double diameter=parset.getDouble("Imager.AntennaIllum.diameter");
      double blockage=parset.getDouble("Imager.AntennaIllum.blockage");
      std::cout << "Using Antenna Illumination for gridding function" << std::endl;
      gridder=IVisGridder::ShPtr(new AntennaIllumVisGridder(diameter, blockage));
    }
    else if(parset.getString("Imager.gridder")=="Box") {
      std::cout << "Using Box function for gridding" << std::endl;
      gridder=IVisGridder::ShPtr(new BoxVisGridder());
    }
    else {
      std::cout << "Using spheriodal function for gridding" << std::endl;
      gridder=IVisGridder::ShPtr(new SphFuncVisGridder());
    }

    NormalEquations ne(skymodel);
    std::cout << "Constructed normal equations" << std::endl;

    IDataSelectorPtr sel=ds.createSelector();
    IDataConverterPtr conv=ds.createConverter();
    conv->setFrequencyFrame(casa::MFrequency::Ref(casa::MFrequency::TOPO),"Hz");
    IDataSharedIter it=ds.createIterator(sel, conv);
    
    it.init();
    it.chooseOriginal();

    int nCycles(parset.getInt32("Imager.cycles", 10));
    
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
      if(parset.getString("Imager.solver")=="Clean") {
        ImageMultiScaleSolver is(skymodel);
        std::cout << "Constructed image multiscale solver" << std::endl;
        is.addNormalEquations(ne);
        std::cout << "Added normal equations to solver" << std::endl;
        is.setNiter(parset.getInt32("Imager.niter", 100));
        is.setGain(parset.getFloat("Imager.gain", 0.7));
        is.setAlgorithm(parset.getString("Imager.algorithm", "MultiScale"));
        std::vector<float> scales(1); scales[0]=0;
        is.setScales(parset.getFloatVector("Imager.scales", scales));
        is.solveNormalEquations(q);
        results.setParameters(is.parameters());
      }
      else {
        ImageSolver is(skymodel);
        std::cout << "Constructed image solver" << std::endl;
        is.addNormalEquations(ne);
        std::cout << "Added normal equations to solver" << std::endl;
        is.solveNormalEquations(q);
        results.setParameters(is.parameters());
      }

      std::cout << "Number of degrees of freedom = " << q.DOF() << std::endl;
    }

    for (vector<string>::iterator it=images.begin();it!=images.end();it++)
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
