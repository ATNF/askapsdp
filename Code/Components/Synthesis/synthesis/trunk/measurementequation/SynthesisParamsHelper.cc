#include <measurementequation/SynthesisParamsHelper.h>
#include <fitting/Axes.h>

#include <casa/aips.h>
#include <casa/BasicSL/Constants.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Cube.h>

using namespace conrad::scimath;

namespace conrad {
  namespace synthesis {
    
    void SynthesisParamsHelper::add(conrad::scimath::Params& ip,
      const string& name, 
      const double ra, const double dec, const double cellsize,
      const int nx, const int ny, 
      const double freqmin, const double freqmax, const int nchan)
    {
      Axes axes;
      axes.add("RA", ra-double(nx)*cellsize/2.0, ra+double(nx)*cellsize/2.0);
      axes.add("DEC", dec-double(ny)*cellsize/2.0, dec+double(ny)*cellsize/2.0);
      
      if(nchan>1) {
        casa::Array<double> pixels(casa::IPosition(4, nx, ny, 1, nchan));
        axes.add("FREQUENCY", freqmin, freqmax);
        ip.add(name, pixels, axes);
      }
      else {
        casa::Cube<double> pixels(nx, ny, 1);
        axes.add("FREQUENCY", freqmin, freqmax);
        ip.add(name, pixels, axes);
      }
    }
      
    void SynthesisParamsHelper::add(conrad::scimath::Params& ip, const string& name, 
      const string& image)
    {
    }
  }
}

