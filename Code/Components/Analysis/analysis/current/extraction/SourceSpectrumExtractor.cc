/// @file
///
/// Class to handle extraction of a summed spectrum corresponding to a source.
///
/// @copyright (c) 2011 CSIRO
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
/// @author Matthew Whiting <Matthew.Whiting@csiro.au>
///
#include <extraction/SourceSpectrumExtractor.h>
#include <askap_analysis.h>
#include <extraction/SourceDataExtractor.h>
#include <extraction/SpectralBoxExtractor.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <sourcefitting/RadioSource.h>

#include <imageaccess/CasaImageAccess.h>
#include <utils/PolConverter.h>

#include <duchamp/PixelMap/Object2D.hh>

#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/MaskedArray.h>
#include <casa/Arrays/Slicer.h>
#include <casa/Arrays/Vector.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/ImageOpener.h>
#include <images/Images/FITSImage.h>
#include <images/Images/MIRIADImage.h>
#include <images/Images/SubImage.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <measures/Measures/Stokes.h>

#include <Common/ParameterSet.h>

using namespace askap::analysis::sourcefitting;

ASKAP_LOGGER(logger, ".sourcespectrumextractor");

namespace askap {

  namespace analysis {

    SourceSpectrumExtractor::SourceSpectrumExtractor(const LOFAR::ParameterSet& parset)
    {
      /// @details Initialise the extractor from a LOFAR parset. This
      /// sets the input cube, the box width, the scaling flag, the
      /// base name for the output spectra files (these will have _X
      /// appended, where X is the ID of the object in question), and
      /// the set of polarisation products to extract.

      this->itsInputCube = parset.getString("spectralCube","");
      this->itsBoxWidth = parset.getInt16("spectralBoxWidth",defaultSpectralExtractionBoxWidth);
      this->itsFlagUseDetection = parset.getBool("useDetectedPixels",false);
      this->itsFlagDoScale = parset.getBool("scaleSpectraByBeam",true);
      this->itsOutputFilenameBase = parset.getString("spectralOutputBase","");

      // Take the following from SynthesisParamsHelper.cc in Synthesis
      // there could be many ways to define stokes, e.g. ["XX YY"] or ["XX","YY"] or "XX,YY"
      // to allow some flexibility we have to concatenate all elements first and then 
      // allow the parser from PolConverter to take care of extracting the products.                                            
      const std::vector<std::string> stokesVec = parset.getStringVector("polarisation", std::vector<std::string>(1,"IQUV"));
      std::string stokesStr;
      for (size_t i=0; i<stokesVec.size(); ++i) {
	stokesStr += stokesVec[i];
      }
      this->itsStokesList = scimath::PolConverter::fromString(stokesStr);
      
      this->itsSource = 0;
      this->itsInputCubePtr = 0;
    }

    SourceSpectrumExtractor::SourceSpectrumExtractor(const SourceSpectrumExtractor& other)
    {
      this->operator=(other);
    }

    SourceSpectrumExtractor& SourceSpectrumExtractor::operator=(const SourceSpectrumExtractor& other)
    {
      if(this == &other) return *this;
      ((SpectralBoxExtractor &) *this) = other;
      this->itsFlagDoScale = other.itsFlagDoScale;
      this->itsFlagUseDetection = other.itsFlagUseDetection;
      this->itsBeamScaleFactor = other.itsBeamScaleFactor;
      this->itsStokesList = other.itsStokesList;
      return *this;
    }

 
    void SourceSpectrumExtractor::setBeamScale()
    {
      /// @details This sets the scale factor used to correct the peak
      /// flux of an unresolved source to a total flux. The beam
      /// information is read from the input image, and the beam
      /// weighting is integrated over the same size box as will be
      /// used to extract the spectrum.
      ///
      /// If the input image has no beam information, or if the flag
      /// itsFlagDoScale=false, then the scale factor is set to 1.

      if(!this->itsFlagDoScale) this->itsBeamScaleFactor = 1.;
      else{

 	this->openInput();
 	Vector<Quantum<Double> > inputBeam = this->itsInputCubePtr->imageInfo().restoringBeam();
	ASKAPLOG_DEBUG_STR(logger, "Beam for input cube = " << inputBeam);
	if(inputBeam.size()==0) {
	  ASKAPLOG_WARN_STR(logger, "Input image \""<<this->itsInputCube<<"\" has no beam information. Not scaling spectra by beam");
	  this->itsBeamScaleFactor = 1.;
	}
	else{

	  casa::CoordinateSystem coo = this->itsInputCubePtr->coordinates();
	  casa::DirectionCoordinate dirCoo = coo.directionCoordinate(coo.findCoordinate(casa::Coordinate::DIRECTION));
	  double fwhmMajPix = inputBeam[0].getValue(dirCoo.worldAxisUnits()[0]) / fabs(dirCoo.increment()[0]);
	  double fwhmMinPix = inputBeam[1].getValue(dirCoo.worldAxisUnits()[1]) / fabs(dirCoo.increment()[1]);

	  if(this->itsFlagUseDetection){
	    double bpaDeg = inputBeam[2].getValue("deg");
	    duchamp::DuchampBeam beam(fwhmMajPix,fwhmMinPix,bpaDeg);
	    this->itsBeamScaleFactor = beam.area();
	    ASKAPLOG_DEBUG_STR(logger, "Beam scale factor = " << this->itsBeamScaleFactor << " using beam of " << fwhmMajPix <<"x"<<fwhmMinPix);
	  }
	  else{
	    
	    double costheta = cos(inputBeam[2].getValue("rad"));
	    double sintheta = sin(inputBeam[2].getValue("rad"));
	    
	    double majSDsq = fwhmMajPix * fwhmMajPix / 8. / M_LN2;
	    double minSDsq = fwhmMinPix * fwhmMinPix / 8. / M_LN2;
	    
	    int hw = (this->itsBoxWidth - 1)/2;
	    this->itsBeamScaleFactor = 0.;
	    for(int y=-hw; y<=hw; y++){
	      for(int x=-hw; x<=hw; x++){
		double u=x*costheta + y*sintheta;
		double v=x*sintheta - y*costheta;
		this->itsBeamScaleFactor += exp(-0.5 * (u*u/majSDsq + v*v/minSDsq));
	      }
	    }

	    ASKAPLOG_DEBUG_STR(logger, "Beam scale factor = " << this->itsBeamScaleFactor);

	  }

	}
      }

    }

    void SourceSpectrumExtractor::extract()
    {
      /// @details The main function that extracts the spectrum from
      /// the desired input. The input cube is opened for reading by
      /// the SourceDataExtractor::openInput() function. A box of
      /// required width is centred on the peak pixel of the
      /// RadioSource, extending over the full spectral range of the
      /// input cube. The box will be truncated at the spatial edges
      /// if necessary. The output spectrum is determined one channel
      /// at a time, summing all pixels within the box and scaling by
      /// the beam if so required. The output spectrum is stored in
      /// itsArray, ready for later access or export.

      this->openInput();
      this->setBeamScale();
      ASKAPLOG_INFO_STR(logger, "Extracting spectrum from " << this->itsInputCube << " for source ID " << this->itsSource->getID() << " using slicer " << this->itsSlicer);

      const SubImage<Float> *sub = new SubImage<Float>(*this->itsInputCubePtr, this->itsSlicer);
      const casa::MaskedArray<Float> msub(sub->get(),sub->getMask());
      casa::Array<Float> subarray(sub->shape());
      subarray = msub;

      //      if(this->itsBoxWidth > 0)
      if(!this->itsFlagUseDetection)
	this->itsArray = partialSums(subarray, IPosition(2,0,1)) / this->itsBeamScaleFactor;
      else {
	ASKAPLOG_INFO_STR(logger, "Extracting integrated spectrum using all detected spatial pixels");
	IPosition shape = this->itsInputCubePtr->shape();
	CoordinateSystem coords = this->itsInputCubePtr->coordinates();
	int lngAxis=coords.directionAxesNumbers()[0];
	int latAxis=coords.directionAxesNumbers()[1];
	int specAxis=coords.spectralAxisNumber();
	PixelInfo::Object2D spatmap=this->itsSource->getSpatialMap();
	casa::IPosition blc(shape.size(),0),trc(shape.size(),0),inc(shape.size(),1);	
	trc(specAxis)=shape[specAxis]-1;
	IPosition newshape(shape);
	newshape(lngAxis) = newshape(latAxis) = 1;
	ASKAPLOG_DEBUG_STR(logger, "Initialising array to zero with shape " << newshape);
	this->itsArray = casa::Array<Float>(newshape,0.);
	for(int x=this->itsSource->getXmin(); x<=this->itsSource->getXmax();x++) {
	  for(int y=this->itsSource->getYmin(); y<=this->itsSource->getYmax();y++){
	    if(spatmap.isInObject(x,y)){
	      blc(lngAxis)=trc(lngAxis)=x-this->itsSource->getXmin(); 
	      blc(latAxis)=trc(latAxis)=y-this->itsSource->getYmin();
	      casa::Array<Float> spec=subarray(blc,trc,inc);
	      ASKAPCHECK(spec.shape() == this->itsArray.shape(),"Extracted pixel spectrum not of correct shape: "<<spec.shape()<< " vs " <<this->itsArray.shape());
	      this->itsArray += spec;
	    }
	  }
	}
	this->itsArray /= this->itsBeamScaleFactor;
      }
      
      delete sub;

    }


  }

}
