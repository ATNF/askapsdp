#include <gridding/VisGridderFactory.h>
#include <gridding/BoxVisGridder.h>
#include <gridding/SphFuncVisGridder.h>
#include <gridding/AntennaIllumVisGridder.h>
#include <gridding/WProjectVisGridder.h>

using namespace LOFAR::ACC::APS;
namespace conrad
{
  namespace synthesis
  {

    VisGridderFactory::VisGridderFactory()
    {
    }

    VisGridderFactory::~VisGridderFactory()
    {
    }
    
    IVisGridder::ShPtr VisGridderFactory::make(const ParameterSet& parset) {
      IVisGridder::ShPtr gridder;
      /// @todo Better handling of string case
      if(parset.getString("gridder")=="WProject") {
        double wmax=parset.getDouble("gridder.WProject.wmax");
        int nwplanes=parset.getInt32("gridder.WProject.nwplanes");
        std::cout << "Using W projection gridding " << std::endl;
        gridder=IVisGridder::ShPtr(new WProjectVisGridder(wmax, nwplanes));
      }
      else if(parset.getString("gridder")=="AntennaIllum") {
        double diameter=parset.getDouble("gridder.AntennaIllum.diameter");
        double blockage=parset.getDouble("gridder.AntennaIllum.blockage");
        std::cout << "Using Antenna Illumination for gridding function" << std::endl;
        gridder=IVisGridder::ShPtr(new AntennaIllumVisGridder(diameter, blockage));
      }
      else if(parset.getString("gridder")=="Box") {
        std::cout << "Using Box function for gridding" << std::endl;
        gridder=IVisGridder::ShPtr(new BoxVisGridder());
      }
      else {
        std::cout << "Using spheriodal function for gridding" << std::endl;
        gridder=IVisGridder::ShPtr(new SphFuncVisGridder());
      }
      return gridder;
    }
  }
}
