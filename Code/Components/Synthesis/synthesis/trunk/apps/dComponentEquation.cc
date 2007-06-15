//
// @file : Evolving demonstration program for synthesis capabilities
//

#include <measurementequation/ComponentEquation.h>

#include <fitting/ParamsCASATable.h>
#include <fitting/LinearSolver.h>

#include <dataaccess/DataIteratorStub.h>

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

    cout << "ComponentEquation demonstration program" << endl;

    IDataSharedIter idi(new DataIteratorStub(1));

    ParamsCASATable pt("nvss.par", true);

    ComponentEquation ce(idi);
    Params nvsspar=ce.defaultParameters();
    pt.getParameters(nvsspar);
    std::cout << "Read parameters" << std::endl;
    std::cout << nvsspar << std::endl;
    ce.setParameters(nvsspar);
    ce.predict();
    std::cout << "Finished prediction" << std::endl;
    exit(0);
  }
  catch (std::exception& x)
  {
    std::cout << "Unexpected exception in " << argv[0] << ": " << x.what() << std::endl;
    exit(1);
  }
};
