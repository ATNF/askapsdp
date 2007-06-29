#include <measurementequation/SynthesisParamsHelper.h>
#include <fitting/Axes.h>

#include <casa/aips.h>
#include <casa/BasicSL/Constants.h>
#include <casa/Quanta.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Cube.h>

using namespace conrad::scimath;

namespace conrad {
  namespace synthesis {
  

    void SynthesisParamsHelper::add(conrad::scimath::Params& ip, const string& name, 
      const vector<string>& direction, 
      const vector<string>& cellsize, 
      const vector<int>& shape,
      const double freqmin, const double freqmax, const int nchan) 
    {
      int nx=shape[0];
      int ny=shape[1];
      
      casa::Quantity q;

      casa::Quantity::read(q, cellsize[0]);
      double xcellsize=q.getValue("rad");
      
      casa::Quantity::read(q, cellsize[1]);
      double ycellsize=q.getValue("rad");
      
      casa::Quantity::read(q, direction[0]);
      double ra=q.getValue("rad");
      
      casa::Quantity::read(q, direction[1]);
      double dec=q.getValue("rad");
      
      /// @todo Do something with the frame info in direction[2]
      Axes axes;
      axes.add("RA", ra-double(nx)*xcellsize/2.0, ra+double(nx)*xcellsize/2.0);
      axes.add("DEC", dec-double(ny)*ycellsize/2.0, dec+double(ny)*ycellsize/2.0);
      
      if(nchan>1) {
        casa::Array<double> pixels(casa::IPosition(4, nx, ny, 1, nchan));
        pixels.set(0.0);
        axes.add("FREQUENCY", freqmin, freqmax);
        ip.add(name, pixels, axes);
      }
      else {
        casa::Cube<double> pixels(nx, ny, 1);
        pixels.set(0.0);
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

