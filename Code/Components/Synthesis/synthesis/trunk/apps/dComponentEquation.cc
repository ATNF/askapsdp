//
// @file : Evolving demonstration program for synthesis capabilities
//

#include <measurementequation/ComponentEquation.h>

#include <fitting/ParamsCASATable.h>
#include <fitting/LinearSolver.h>

#include <dataaccess/TableConstDataSource.h>

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

    ParamsCASATable pt("nvss.par", true);
    Params nvsspar;
    {
      ComponentEquation ce;
      nvsspar=ce.defaultParameters();
    }
    pt.getParameters(nvsspar);
    std::cout << "Read parameters" << std::endl;
    std::cout << nvsspar << std::endl;

    for (IDataSharedIter it=ds.createIterator();it!=it.end();++it) {
      ComponentEquation ce(it);
      ce.setParameters(nvsspar);
      ce.predict();
    }
    std::cout << "Finished prediction" << std::endl;
    exit(0);
  }
  catch (std::exception& x)
  {
    std::cout << "Unexpected exception in " << argv[0] << ": " << x.what() << std::endl;
    exit(1);
  }
};
