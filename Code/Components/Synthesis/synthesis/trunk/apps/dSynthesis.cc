//
// @file : Evolving demonstration program for synthesis capabilities
//
#include <conrad/ConradError.h>

#include <measurementequation/ComponentEquation.h>
#include <measurementequation/ImageFFTEquation.h>
#include <measurementequation/ImageSolver.h>
#include <measurementequation/SynthesisParamsHelper.h>

#include <gridding/IVisGridder.h>
#include <gridding/SphFuncVisGridder.h>
#include <gridding/AntennaIllumVisGridder.h>

#include <fitting/CompositeEquation.h>
#include <fitting/ParamsCASATable.h>
#include <fitting/Axes.h>

//#include <dataaccess/TableConstDataSource.h>
#include <dataaccess/DataIteratorStub.h>

#include <casa/aips.h>
#include <casa/BasicSL/Constants.h>
#include <casa/Arrays/Cube.h>

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
    
    string parsetname(string(argv[0])+".parset");
    if (argc==2)
    {
      parsetname=argv[1];
    }

    ParameterSet parset(parsetname);
    string ms=parset.getString("DataSet");

    Params skymodel;

    /// Load the local sky model if it has been specified
    if(parset.isDefined("Parms.LocalSky"))
    {
      string localsky(parset.getString("Parms.LocalSky"));
      ParamsCASATable pt(localsky, true);
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

//    TableConstDataSource ds(argv[1]);

    cout << "Synthesis demonstration program" << endl;

    IDataSharedIter idi = IDataSharedIter(new DataIteratorStub(1));

    /// Now set up the imager
    
    IVisGridder::ShPtr gridder;
    if(parset.getString("Imager.gridder")=="AntennaIllum") {
      double diameter=parset.getDouble("Imager.AntennaIllum.diameter");
      double blockage=parset.getDouble("Imager.AntennaIllum.blockage");
      std::cout << "Using Antenna Illumination for gridding function" << std::endl;
      gridder=IVisGridder::ShPtr(new AntennaIllumVisGridder(diameter, blockage));
    }
    else {
      std::cout << "Using spheriodal function for gridding" << std::endl;
      gridder=IVisGridder::ShPtr(new SphFuncVisGridder());
    }
    
    ImageFFTEquation ie(skymodel, idi, gridder);

    NormalEquations ne(skymodel);
    std::cout << "Constructed normal equations" << std::endl;

    ImageSolver is(skymodel);
    std::cout << "Constructed image solver" << std::endl;
//    for (it=ds.createConstIterator();it!=it.end();++it) {
    ie.calcEquations(ne);
    std::cout << "Calculated normal equations" << std::endl;
    is.addNormalEquations(ne);
    std::cout << "Added normal equations to solver" << std::endl;
//    }
    Quality q;
    std::cout << "Solving normal equations" << std::endl;
    is.solveNormalEquations(q);
    std::cout << q << std::endl;

    {
      string resultfile(parset.getString("Parms.Result"));
      ParamsCASATable results(resultfile, false);
      results.setParameters(is.parameters());
    }

    std::cout << "Finished imaging" << std::endl;
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
