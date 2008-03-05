#include <measurementequation/SynthesisParamsHelper.h>
#include <fitting/Axes.h>

#include <askap_synthesis.h>
#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, ".measurementequation");

#include <askap/AskapError.h>

#include <casa/aips.h>
#include <casa/BasicSL/Constants.h>
#include <casa/Quanta.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>

#include <images/Images/PagedImage.h>
#include <images/Images/TempImage.h>
#include <lattices/Lattices/ArrayLattice.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/StokesCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/Projection.h>

#include <APS/ParameterSet.h>
#include <Common/Exception.h>

#include <measures/Measures/Stokes.h>

#include <vector>
#include <algorithm>

using namespace askap::scimath;
using namespace casa;

namespace askap
{
  namespace synthesis
  {
  
    /// @brief populate scimath parameters from a LOFAR Parset object
    /// @details One needs often needs a possibility to populate 
    /// scimath::Params class from a Parset file (e.g. to load 
    /// initial gains from an external file). A number of add methods
    /// collected in this class happen to be image-specific. This is
    /// a generic method, which just copies all numeric fields
    /// @param[in] params a reference to scimath parameter object, where the
    /// parameters from parset file will be added
    /// @param[in] parset a const reference to a parset object
    /// @return a reference to params passed as an input (for chaining)
    scimath::Params& operator<<(scimath::Params &params, const LOFAR::ACC::APS::ParameterSet &parset)
    {
       for (LOFAR::ACC::APS::ParameterSet::const_iterator ci = parset.begin();
            ci != parset.end();++ci) {
            try {
               vector<double> vec = parset.getDoubleVector(ci->first);
               casa::Vector<double> arr(vec.size());
               std::copy(vec.begin(),vec.end(),arr.cbegin());
               params.add(ci->first, arr);
            }
            catch (const LOFAR::Exception &) {
               // ignore non-numeric parameters
            }
       }
       return params;
    }
    
    void SynthesisParamsHelper::setUpImages(askap::scimath::Params::ShPtr& params,
				const LOFAR::ACC::APS::ParameterSet &parset)
    {
	  vector<string> images=parset.getStringVector("Names");
      for (vector<string>::iterator it=images.begin();it!=images.end();it++) {
           std::vector<int> shape=parset.getInt32Vector(*it+".shape");
           int nchan=parset.getInt32(*it+".nchan");
           std::vector<double> freq=parset.getDoubleVector(*it+".frequency");
           std::vector<std::string> direction=parset.getStringVector(*it+".direction");
           std::vector<std::string> cellsize=parset.getStringVector(*it+".cellsize");

		   add(*params, *it, direction, cellsize, shape,
						freq[0], freq[1], nchan);
	   }
	}
    
    
    void SynthesisParamsHelper::add(askap::scimath::Params& ip,
				    const LOFAR::ACC::APS::ParameterSet& parset, const std::string& baseKey)
    {
      vector<string> images=parset.getStringVector(baseKey+"Names");
      for (vector<string>::iterator it=images.begin(); it!=images.end(); it++)
	{
	  ASKAPLOG_INFO_STR(logger, "Defining image "<< *it );
	  std::vector<int> shape=parset.getInt32Vector(baseKey+*it+".shape");
	  int nchan=parset.getInt32("Images."+*it+".nchan");
	  std::vector<double> freq=parset.getDoubleVector(baseKey+*it
							  +".frequency");
	  std::vector<std::string> direction=parset.getStringVector(baseKey+*it
								    +".direction");
	  std::vector<std::string> cellsize=parset.getStringVector(baseKey+*it
								   +".cellsize");
	  
	  SynthesisParamsHelper::add(ip, *it, direction, cellsize, shape,
				     freq[0], freq[1], nchan);
	}
    }
    
    void SynthesisParamsHelper::add(askap::scimath::Params& ip,
				    const string& name, const vector<string>& direction,
				    const vector<string>& cellsize, const vector<int>& shape,
				    const double freqmin, const double freqmax, const int nchan)
    {
      int nx=shape[0];
      int ny=shape[1];
      
      casa::Quantity q;
      
      casa::Quantity::read(q, cellsize[0]);
      double xcellsize=-1.0*q.getValue("rad");
      
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
      
      axes.add("STOKES", 0.0, 0.0);
      
      casa::Array<double> pixels(casa::IPosition(4, nx, ny, 1, nchan));
      pixels.set(0.0);
      axes.add("FREQUENCY", freqmin, freqmax);
      ip.add(name, pixels, axes);
    }
    
    void SynthesisParamsHelper::saveAsCasaImage(const askap::scimath::Params& ip, const string& name,
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
      
      casa::Quantum<double> incLon((axes.end("RA")-axes.start("RA"))/double(nx), "rad");
      casa::Quantum<double> incLat((axes.end("DEC")-axes.start("DEC"))/double(ny), "rad");
      
      casa::DirectionCoordinate radec(MDirection::J2000,
				      Projection(Projection::SIN),
				      refLon, refLat,
				      incLon, incLat,
				      xform, nx/2, nx/2);
      
      casa::CoordinateSystem imageCoords;
      imageCoords.addCoordinate(radec);
      
      casa::Vector<int> iquv(1);
      iquv(0) = Stokes::I;
      
      casa::StokesCoordinate stokes(iquv);
      imageCoords.addCoordinate(stokes);
      
      int nchan=imagePixels.shape()(2);
      double restfreq = 0.0;
      double crpix = (nchan-1)/2;
      double crval = (axes.start("FREQUENCY")+axes.end("FREQUENCY"))/2.0;
      double cdelt = (axes.end("FREQUENCY")-axes.start("FREQUENCY"))/double(nchan);
      casa::SpectralCoordinate freq(casa::MFrequency::TOPO, crval, cdelt, crpix, restfreq);
      imageCoords.addCoordinate(freq);
      
      casa::PagedImage<float> imgImagePixels(TiledShape(imagePixels.shape()),
					     imageCoords, casa::String(imagename));
      imgImagePixels.copyData(latImagePixels);
      
      imgImagePixels.setUnits("Jy/pixel");
      
    }
    
    void SynthesisParamsHelper::getFromCasaImage(askap::scimath::Params& ip, const string& name,
						 const string& imagename)
    {
      
      casa::PagedImage<float> imgImagePixels(imagename);
      casa::Array<float> floatImagePixels(imgImagePixels.shape());
      casa::ArrayLattice<float> latImagePixels(floatImagePixels);
      latImagePixels.copyData(imgImagePixels);
      casa::Array<double> imagePixels(imgImagePixels.shape());
      casa::convertArray<double, float>(imagePixels, floatImagePixels);
      
      casa::CoordinateSystem imageCoords(imgImagePixels.coordinates());
      
      /// Fill in the axes information
      Axes axes;
      /// First do the direction
      int whichDir=imageCoords.findCoordinate(Coordinate::DIRECTION);
      ASKAPCHECK(whichDir>-1, "No direction coordinate present in model");
      casa::DirectionCoordinate radec(imageCoords.directionCoordinate(whichDir));
      
      casa::Vector<casa::String> units(2);
      units.set("rad");
      radec.setWorldAxisUnits(units);
      
      casa::Vector<double> refPix(radec.referencePixel());
      casa::Vector<double> refInc(radec.increment());
      casa::Vector<double> refValue(radec.referenceValue());
      
      casa::Vector<double> start(2);
      casa::Vector<double> end(2);
      
      for (int i=0;i<2;++i)
	{
	  start(i)=refValue(i)+refInc(i)*(refPix(i)-0.0);
	  end(i)=refValue(i)+refInc(i)*(refPix(i)-double(imagePixels.shape()(i)));
	}
      
      axes.add("RA", start(0), end(0));
      axes.add("DEC", start(1), end(1));
      
      axes.add("STOKES", 0.0, 0.0);
      
      int whichSpectral=imageCoords.findCoordinate(Coordinate::SPECTRAL);
      ASKAPCHECK(whichSpectral>-1, "No spectral coordinate present in model");
      casa::SpectralCoordinate freq(imageCoords.spectralCoordinate(whichSpectral));
      int nChan=imagePixels.shape()(whichSpectral);
      double startFreq, endFreq;
      freq.toWorld(startFreq, 1.0);
      freq.toWorld(endFreq, double(nChan));
      axes.add("FREQUENCY", startFreq, endFreq);
      
      ip.add(name, imagePixels.reform(IPosition(4, imagePixels.shape()(0), imagePixels.shape()(1), 1, nChan)),
	     axes);
      
    }
    
    boost::shared_ptr<casa::TempImage<float> >
    SynthesisParamsHelper::tempImage(const askap::scimath::Params& ip,
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
    SynthesisParamsHelper::coordinateSystem(const askap::scimath::Params& ip,
					    const string& name)
    {
      const Axes axes(ip.axes(name));
      
      casa::DirectionCoordinate radec(directionCoordinate(ip, name));
      
      casa::CoordinateSystem imageCoords;
      imageCoords.addCoordinate(radec);
      
      casa::Vector<int> iquv(1);
      iquv(0) = Stokes::I;
      
      casa::StokesCoordinate stokes(iquv);
      imageCoords.addCoordinate(stokes);
      
      int nchan=ip.value(name).shape()(2);
      double restfreq = 0.0;
      double crpix = (nchan-1)/2;
      double crval = (axes.start("FREQUENCY")+axes.end("FREQUENCY"))/2.0;
      double cdelt = (axes.end("FREQUENCY")-axes.start("FREQUENCY"))/double(nchan);
      casa::SpectralCoordinate freq(casa::MFrequency::TOPO, crval, cdelt, crpix, restfreq);
      imageCoords.addCoordinate(freq);
      
      return imageCoords;
    }
    
    casa::DirectionCoordinate
    SynthesisParamsHelper::directionCoordinate(const askap::scimath::Params& ip,
					       const string& name)
    {
      const Axes axes(ip.axes(name));
      
      casa::Matrix<double> xform(2,2);
      xform = 0.0; xform.diagonal() = 1.0;
      int nx=ip.value(name).shape()(0);
      int ny=ip.value(name).shape()(1);
      casa::Quantum<double> refLon((axes.start("RA")+axes.end("RA"))/2.0, "rad");
      casa::Quantum<double> refLat((axes.start("DEC")+axes.end("DEC"))/2.0, "rad");
      
      casa::Quantum<double> incLon((axes.end("RA")-axes.start("RA"))/double(nx), "rad");
      casa::Quantum<double> incLat((axes.end("DEC")-axes.start("DEC"))/double(ny), "rad");
      
      casa::DirectionCoordinate radec(MDirection::J2000,
				      Projection(Projection::SIN),
				      refLon, refLat,
				      incLon, incLat,
				      xform, nx/2, nx/2);
      
      return radec;
    }
    
    void SynthesisParamsHelper::update(askap::scimath::Params& ip, const string& name,
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

