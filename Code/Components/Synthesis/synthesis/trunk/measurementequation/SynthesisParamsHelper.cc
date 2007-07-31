#include <measurementequation/SynthesisParamsHelper.h>
#include <fitting/Axes.h>

#include <casa/aips.h>
#include <casa/BasicSL/Constants.h>
#include <casa/Quanta.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/Cube.h>

#include <images/Images/PagedImage.h>
#include <images/Images/TempImage.h>
#include <lattices/Lattices/ArrayLattice.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/StokesCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/Projection.h>


#include <APS/ParameterSet.h>


#include <measures/Measures/Stokes.h>

using namespace conrad::scimath;
using namespace casa;

namespace conrad {
  namespace synthesis {
 
     void SynthesisParamsHelper::add(conrad::scimath::Params& ip, const LOFAR::ACC::APS::ParameterSet& parset,
      const std::string& baseKey)
     {
        vector<string> images=parset.getStringVector(baseKey+"Names");
        for (vector<string>::iterator it=images.begin();it!=images.end();it++)
        {
          std::cout << "Defining image " << *it << std::endl;
          std::vector<int> shape=parset.getInt32Vector(baseKey+*it+".shape");
          int nchan=parset.getInt32("Images."+*it+".nchan");
          std::vector<double> freq=parset.getDoubleVector(baseKey+*it+".frequency");
          std::vector<std::string> direction=parset.getStringVector(baseKey+*it+".direction");
          std::vector<std::string> cellsize=parset.getStringVector(baseKey+*it+".cellsize");
          
          SynthesisParamsHelper::add(ip, *it, direction, cellsize, shape, 
            freq[0], freq[1], nchan);
        }
     }
 

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
    
    void SynthesisParamsHelper::saveAsCasaImage(const conrad::scimath::Params& ip, const string& name,
      const string& imagename) 
    {
      const casa::Array<double> imagePixels(ip.value(name));
      const Axes axes(ip.axes(name));
      casa::Array<float> floatImagePixels(imagePixels.shape());
      casa::convertArray<float, double>(floatImagePixels, imagePixels);
      
      casa::ArrayLattice<float> latImagePixels(floatImagePixels);
      casa::Matrix<double> xform(2,2);                                    
      xform = 0.0; xform.diagonal() = 1.0;
      int nx=imagePixels.shape()(0);                          
      int ny=imagePixels.shape()(1);                          
      casa::Quantum<double> refLon((axes.start("RA")+axes.end("RA"))/2.0, "rad");
      casa::Quantum<double> refLat((axes.start("DEC")+axes.end("DEC"))/2.0, "rad");

      casa::Quantum<double> incLon((axes.start("RA")-axes.end("RA"))/double(nx), "rad");
      casa::Quantum<double> incLat((axes.start("DEC")-axes.end("DEC"))/double(ny), "rad");
      
      casa::DirectionCoordinate radec(MDirection::J2000,
                            Projection(Projection::SIN),
                            refLon, refLat,
                            incLon, incLat,
                            xform, nx/2, nx/2);

      casa::CoordinateSystem imageCoords;
      imageCoords.addCoordinate(radec);
      
//      casa::Vector<int> iquv(1);
//      iquv(0) = Stokes::I; 
//
//      casa::StokesCoordinate stokes(iquv);
//      imageCoords.addCoordinate(stokes);
      
      int nchan=imagePixels.shape()(2);
      double restfreq = 0.0;
      double crpix = (nchan-1)/2;
      double crval = (axes.start("FREQUENCY")+axes.end("FREQUENCY"))/2.0;
      double cdelt = (axes.start("FREQUENCY")-axes.end("FREQUENCY"))/double(nchan);
      casa::SpectralCoordinate freq(casa::MFrequency::TOPO, crval, cdelt, crpix, restfreq);
      imageCoords.addCoordinate(freq);
       
      casa::PagedImage<float> imgImagePixels(TiledShape(imagePixels.shape()), 
        imageCoords, casa::String(imagename));
      imgImagePixels.copyData(latImagePixels); 


      imgImagePixels.setUnits("Jy/pixel");      
      
    }
          
    boost::shared_ptr<casa::TempImage<float> > 
      SynthesisParamsHelper::tempImage(const conrad::scimath::Params& ip, 
      const string& name) 
    {
        const casa::Array<double> imagePixels(ip.value(name));

      casa::CoordinateSystem imageCoords(coordinateSystem(ip, name));

      boost::shared_ptr<casa::TempImage<float> > 
        im(new casa::TempImage<float> (TiledShape(imagePixels.shape()), 
          imageCoords));

      im->setUnits("Jy/pixel");
      
      casa::Array<float> floatImagePixels(imagePixels.shape());
      casa::convertArray<float, double>(floatImagePixels, imagePixels);
      casa::ArrayLattice<float> latImagePixels(floatImagePixels);
      im->copyData(latImagePixels);
      return im; 
    }
          
    casa::CoordinateSystem 
      SynthesisParamsHelper::coordinateSystem(const conrad::scimath::Params& ip, 
      const string& name) 
    {
      const Axes axes(ip.axes(name));

      casa::DirectionCoordinate radec(directionCoordinate(ip, name));

      casa::CoordinateSystem imageCoords;
      imageCoords.addCoordinate(radec);
      
//      casa::Vector<int> iquv(1);
//      iquv(0) = Stokes::I; 
//
//      casa::StokesCoordinate stokes(iquv);
//      imageCoords.addCoordinate(stokes);
      
      int nchan=ip.value(name).shape()(2);
      double restfreq = 0.0;
      double crpix = (nchan-1)/2;
      double crval = (axes.start("FREQUENCY")+axes.end("FREQUENCY"))/2.0;
      double cdelt = (axes.start("FREQUENCY")-axes.end("FREQUENCY"))/double(nchan);
      casa::SpectralCoordinate freq(casa::MFrequency::TOPO, crval, cdelt, crpix, restfreq);
      imageCoords.addCoordinate(freq);
      
      return imageCoords; 
    }
          
    casa::DirectionCoordinate
      SynthesisParamsHelper::directionCoordinate(const conrad::scimath::Params& ip, 
      const string& name) 
    {
      const Axes axes(ip.axes(name));

      casa::Matrix<double> xform(2,2);                                    
      xform = 0.0; xform.diagonal() = 1.0;
      int nx=ip.value(name).shape()(0);                          
      int ny=ip.value(name).shape()(1);                          
      casa::Quantum<double> refLon((axes.start("RA")+axes.end("RA"))/2.0, "rad");
      casa::Quantum<double> refLat((axes.start("DEC")+axes.end("DEC"))/2.0, "rad");

      casa::Quantum<double> incLon((axes.start("RA")-axes.end("RA"))/double(nx), "rad");
      casa::Quantum<double> incLat;
      if(refLat.getValue()<0.0) {
        incLat=casa::Quantum<double>((axes.end("DEC")-axes.start("DEC"))/double(ny), "rad");
      }
      else {
        incLat=casa::Quantum<double>((axes.start("DEC")-axes.end("DEC"))/double(ny), "rad");
      }
      
      casa::DirectionCoordinate radec(MDirection::J2000,
                            Projection(Projection::SIN),
                            refLon, refLat,
                            incLon, incLat,
                            xform, nx/2, nx/2);

      return radec; 
    }
          
      
    void SynthesisParamsHelper::update(conrad::scimath::Params& ip, const string& name, 
      const casa::ImageInterface<float>& im)
    {
      /// This next copy should be a reference unless it is too big
      casa::Array<float> floatImagePixels(im.shape());
      casa::ArrayLattice<float> latImagePixels(floatImagePixels);
      latImagePixels.copyData(im);

      casa::Array<double> imagePixels(im.shape());
      casa::convertArray<double, float>(imagePixels, floatImagePixels);
      ip.update(name, imagePixels);
    }
  }
}

