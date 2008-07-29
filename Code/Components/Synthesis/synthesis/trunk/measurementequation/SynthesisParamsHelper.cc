/// @copyright (c) 2007 CSIRO
/// Australia Telescope National Facility (ATNF)
/// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
/// PO Box 76, Epping NSW 1710, Australia
/// atnf-enquiries@csiro.au
///
/// This file is part of the ASKAP software distribution.
///
/// The ASKAP software distribution is free software: you can redistribute it
/// and/or modify it under the terms of the GNU General Public License as
/// published by the Free Software Foundation; either version 2 of the License,
/// or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
///

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
#include <APS/Exceptions.h>
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
    
    void SynthesisParamsHelper::setUpImages(const askap::scimath::Params::ShPtr& params,
				const LOFAR::ACC::APS::ParameterSet &parset)
    {
      try {
	     vector<string> images=parset.getStringVector("Names");
         std::vector<int> shape=parset.getInt32Vector("shape");
         std::vector<std::string> cellsize=parset.getStringVector("cellsize");
	  
         for (vector<string>::iterator it=images.begin();it!=images.end();it++) {
              int nchan=parset.getInt32(*it+".nchan");
              std::vector<double> freq=parset.getDoubleVector(*it+".frequency");
              std::vector<std::string> direction=parset.getStringVector(*it+".direction");
              ASKAPCHECK(!parset.isDefined(*it+".shape"), "Parameters like Cimager.Images."<<*it<<
                         ".shape are deprecated. Use Cimager.Images.shape (same for all images)");
              ASKAPCHECK(!parset.isDefined(*it+".cellsize"), "Parameters like Cimager.Images."<<*it<<
                         ".cellsize are deprecated. Use Cimager.Images.cellsize (same for all images)");
                      
		      add(*params, *it, direction, cellsize, shape, freq[0], freq[1], nchan);
	     }
	  }
	  catch (const LOFAR::ACC::APS::APSException &ex) {
	      throw AskapError(ex.what());
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
      ASKAPCHECK(cellsize.size() == 2, "Cell size should have exactly 2 parameters, you have "<<cellsize.size());
      ASKAPCHECK(direction.size() == 3, "Direction should have exactly 3 parameters, you have "<<direction.size());
      ASKAPCHECK(direction[2] == "J2000", "Only J2000 is implemented at the moment, you have requested "<<direction[2]);
      
      const double xcellsize =-1.0*convertQuantity(cellsize[0],"rad");
      const double ycellsize = convertQuantity(cellsize[1],"rad");
      
      const double ra = convertQuantity(direction[0],"rad");
      const double dec = convertQuantity(direction[1],"rad");
      
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
    
    /// @brief A helper method to parse string of quantities
    /// @details Many parameters in parset file are given as quantities or
    /// vectors of quantities, e.g. [8.0arcsec,8.0arcsec]. This method allows
    /// to parse vector of strings corresponding to such parameter and return
    /// a vector of double values in the required units.
    /// @param[in] strval input vector of strings
    /// @param[in] unit required units (given as a string)
    /// @return vector of doubles with converted values
    std::vector<double> SynthesisParamsHelper::convertQuantity(const std::vector<std::string> &strval,
                       const std::string &unit)
    {
       std::vector<double> result(strval.size());
       std::vector<std::string>::const_iterator inIt = strval.begin();
       for (std::vector<double>::iterator outIt = result.begin(); inIt != strval.end(); 
                                                ++inIt,++outIt) {
            ASKAPDEBUGASSERT(outIt != result.end());                                       
            *outIt = convertQuantity(*inIt,unit);
       }
       return result;
    }
    
    /// @brief A helper method to parse string of quantities
    /// @details Many parameters in parset file are given as quantities or
    /// vectors of quantities, e.g. 8.0arcsec. This method allows
    /// to parse a single string corresponding to such a parameter and return
    /// a double value converted to the requested units.
    /// @param[in] strval input string
    /// @param[in] unit required units (given as a string)
    /// @return converted value
    double SynthesisParamsHelper::convertQuantity(const std::string &strval,
                       const std::string &unit)
    {
       casa::Quantity q;
      
       casa::Quantity::read(q, strval);
       return q.getValue(casa::Unit(unit));
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
      //double lat((axes.start("DEC")+axes.end("DEC"))/2.0);
      
      casa::Quantum<double> incLon((axes.end("RA")-axes.start("RA"))/double(nx), "rad");
      casa::Quantum<double> incLat((axes.end("DEC")-axes.start("DEC"))/double(ny), "rad");
      
      Projection projection(Projection::SIN);
      casa::DirectionCoordinate radec(MDirection::J2000,
				      projection,
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
      
      for (int i=0;i<2;++i) {
	     start(i)=refValue(i)-refInc(i)*(refPix(i));
	     end(i)=refValue(i)+refInc(i)*(double(imagePixels.shape()(i))-refPix(i));
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
    
    /// @brief check whether parameter list defines at least one component
    /// @details Parameter lists can have a mixture of components and
    /// images defined. This method checks whether the given parameter
    /// list defines at least one component.
    /// @param[in] params a shared pointer to the parameter container
    /// @return true, if at least one component is defined
    bool SynthesisParamsHelper::hasComponent(const askap::scimath::Params::ShPtr &params)
    {
       ASKAPDEBUGASSERT(params);
       return params->completions("flux.i").size()!=0;
    }
    
    /// @brief check whether parameter list defines at least one image
    /// @details Parameter lists can have a mixture of components and
    /// images defined. This method checks whether the given parameter
    /// list defines at least one image.
    /// @param[in] params a shared pointer to the parameter container
    /// @return true, if at least one image is defined
    bool SynthesisParamsHelper::hasImage(const askap::scimath::Params::ShPtr &params)
    {
       ASKAPDEBUGASSERT(params);
       return params->completions("image.i").size()!=0;
    }
    
    /// @brief load component-related parameters from a parset file
    /// @details Parameter layout is different in scimath::Params and
    /// parset files for some reason. Typically a source is defined with
    /// parameters like flux.i.name, direction.ra.name, ... within the
    /// scimath::Params, but in the parset file the names of the parameters
    /// are sources.name.flux.i, sources.name.direction.ra, etc). This
    /// method translates the parameter names and copies the values accross.
    /// @param[in] params a shared pointer to the parameter container
    /// @param[in] parset a parset object to read the data from
    /// @param[in] srcName name of the source
    /// @param[in] baseKey a prefix added to parset parameter names (default
    /// is "sources.", wich matches the current layout of the parset file)
    void SynthesisParamsHelper::copyComponent(const askap::scimath::Params::ShPtr &params,
           const LOFAR::ACC::APS::ParameterSet &parset, 
           const std::string &srcName, const std::string &baseKey)
    {
       ASKAPDEBUGASSERT(params);
       // first, create a list of parameters describing the component
       // if the value of the map is true, the parameter is mandatory
       // (in the future we may have a more flexible code here filling this map)
       std::map<std::string, bool>  parameterList;
       parameterList["flux.i"] = true;
       parameterList["direction.ra"] = true;
       parameterList["direction.dec"] = true;
       parameterList["shape.bmaj"] = false;
       parameterList["shape.bmin"] = false;
       parameterList["shape.bpa"] = false;
       
       // now iterate through all parameters
       for (std::map<std::string, bool>::const_iterator ci = parameterList.begin();
            ci!=parameterList.end(); ++ci) {
            const std::string parName = baseKey+srcName+"."+ci->first;
            if (parset.isDefined(parName)) {
                const double val = parset.getDouble(parName);
                params->add(ci->first+"."+srcName, val);
            } else {
                if (ci->second) {
                    ASKAPTHROW(AskapError, "Parameter "<<parName<<
                           " is required to define the source "<<srcName<<
                           ", baseKey="<<baseKey);
                }
            }
       }
    }
  }
}

