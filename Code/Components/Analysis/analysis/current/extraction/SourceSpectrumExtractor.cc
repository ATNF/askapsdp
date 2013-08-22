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
#include <imageaccess/BeamLogger.h>

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

	SourceSpectrumExtractor::SourceSpectrumExtractor(const LOFAR::ParameterSet& parset):
	    SpectralBoxExtractor(parset)
	{
	    /// @details Initialise the extractor from a LOFAR parset. This
	    /// sets the input cube, the box width, the scaling flag, the
	    /// base name for the output spectra files (these will have _X
	    /// appended, where X is the ID of the object in question), and
	    /// the set of polarisation products to extract.

	    this->itsFlagUseDetection = parset.getBool("useDetectedPixels",false);
	    if(this->itsFlagUseDetection){
		this->itsBoxWidth = -1;
		if(parset.isDefined("spectralBoxWidth")) ASKAPLOG_WARN_STR(logger, "useDetectedPixels option selected, so setting spectralBoxWidth=-1");
	    }

	    this->itsFlagDoScale = parset.getBool("scaleSpectraByBeam",true);
	    this->itsBeamLog = parset.getString("beamLog","");

	    this->initialiseArray();

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
	    this->itsBeamLog = other.itsBeamLog;
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

	    this->itsBeamScaleFactor = std::vector<float>();

	    if(this->itsFlagDoScale){

		if(this->openInput()){
	
		    std::vector< casa::Vector<Quantum<Double> > > beamvec;
		
		    casa::Vector<Quantum<Double> > inputBeam = this->itsInputCubePtr->imageInfo().restoringBeam();

		    ASKAPLOG_DEBUG_STR(logger, "Setting beam scaling factor. BeamLog="<<this->itsBeamLog<<", image beam = " << inputBeam);

		    if(this->itsBeamLog == ""){
			if(inputBeam.size()==0) {
			    ASKAPLOG_WARN_STR(logger, "Input image \""<<this->itsInputCube<<"\" has no beam information. Not scaling spectra by beam");
			    this->itsBeamScaleFactor.push_back(1.);
			}
			else{
			    beamvec.push_back(inputBeam);
			    ASKAPLOG_DEBUG_STR(logger, "Beam for input cube = " << inputBeam);
			} 
		    }
		    else{
			accessors::BeamLogger beamlog(this->itsBeamLog);
			beamlog.read();
			beamvec = beamlog.beamlist();

			if(int(beamvec.size()) != this->itsInputCubePtr->shape()(this->itsSpcAxis)){
			    ASKAPLOG_ERROR_STR(logger, "Beam log " << this->itsBeamLog << " has " << beamvec.size() 
					       << " entries - was expecting " << this->itsInputCubePtr->shape()(this->itsSpcAxis));
			    beamvec=std::vector< Vector<Quantum<Double> > >(1,inputBeam);
			}
		    }

		    if(beamvec.size() > 0) {

			for(size_t i=0;i<beamvec.size();i++){

			    casa::DirectionCoordinate dirCoo = this->itsInputCoords.directionCoordinate(this->itsInputCoords.findCoordinate(casa::Coordinate::DIRECTION));
			    double fwhmMajPix = beamvec[i][0].getValue(dirCoo.worldAxisUnits()[0]) / fabs(dirCoo.increment()[0]);
			    double fwhmMinPix = beamvec[i][1].getValue(dirCoo.worldAxisUnits()[1]) / fabs(dirCoo.increment()[1]);

			    if(this->itsFlagUseDetection){
				double bpaDeg = beamvec[i][2].getValue("deg");
				duchamp::DuchampBeam beam(fwhmMajPix,fwhmMinPix,bpaDeg);
				this->itsBeamScaleFactor.push_back(beam.area());
				if(this->itsBeamLog=="")
				    ASKAPLOG_DEBUG_STR(logger, "Beam scale factor = " << this->itsBeamScaleFactor << " using beam of " << fwhmMajPix <<"x"<<fwhmMinPix);
			    }
			    else{
	    
				double costheta = cos(beamvec[i][2].getValue("rad"));
				double sintheta = sin(beamvec[i][2].getValue("rad"));
		    
				double majSDsq = fwhmMajPix * fwhmMajPix / 8. / M_LN2;
				double minSDsq = fwhmMinPix * fwhmMinPix / 8. / M_LN2;
		    
				int hw = (this->itsBoxWidth - 1)/2;
				double scaleFactor = 0.;
				for(int y=-hw; y<=hw; y++){
				    for(int x=-hw; x<=hw; x++){
					double u=x*costheta + y*sintheta;
					double v=x*sintheta - y*costheta;
					scaleFactor += exp(-0.5 * (u*u/majSDsq + v*v/minSDsq));
				    }
				}
				this->itsBeamScaleFactor.push_back(scaleFactor);

				if(this->itsBeamLog=="")
				    ASKAPLOG_DEBUG_STR(logger, "Beam scale factor = " << this->itsBeamScaleFactor);
		    
			    }
			}

		    }

		    ASKAPLOG_DEBUG_STR(logger, "Defined the beam scale factor vector of size " << this->itsBeamScaleFactor.size());

		    this->closeInput();
		}
		else ASKAPLOG_ERROR_STR(logger, "Could not open image");
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

		this->setBeamScale();

	    for(size_t stokes=0; stokes<this->itsStokesList.size(); stokes++){

		this->itsInputCube = this->itsInputCubeList[stokes%this->itsInputCubeList.size()]; // get either the matching image for the current stokes value, or the first&only in the input list
		this->itsCurrentStokes = this->itsStokesList[stokes];
		this->defineSlicer();
		if(this->openInput()){
		    casa::Stokes stk;
		    ASKAPLOG_INFO_STR(logger, "Extracting spectrum from " << this->itsInputCube << " with shape " << this->itsInputCubePtr->shape() 
				      << " for source ID " << this->itsSource->getID() 
				      << " using slicer " << this->itsSlicer << " and Stokes " << stk.name(this->itsCurrentStokes));

		    const SubImage<Float> *sub = new SubImage<Float>(*this->itsInputCubePtr, this->itsSlicer);
		    ASKAPASSERT(sub->size()>0);
		    const casa::MaskedArray<Float> msub(sub->get(),sub->getMask());
		    casa::Array<Float> subarray(sub->shape());
		    subarray = msub;

		    casa::IPosition outBLC(4,0),outTRC(this->itsArray.shape()-1);
		    outBLC(2) = outTRC(2) = stokes;

		    if(!this->itsFlagUseDetection){
			casa::Array<Float> sumarray = partialSums(subarray, IPosition(2,0,1));
			this->itsArray(outBLC,outTRC) = sumarray.reform(this->itsArray(outBLC,outTRC).shape());

		    }
		    else {
			ASKAPLOG_INFO_STR(logger, "Extracting integrated spectrum using all detected spatial pixels");
			IPosition shape = this->itsInputCubePtr->shape();

			PixelInfo::Object2D spatmap=this->itsSource->getSpatialMap();
			casa::IPosition blc(shape.size(),0),trc(shape.size(),0),inc(shape.size(),1);	
			trc(this->itsSpcAxis)=shape[this->itsSpcAxis]-1;
			if(this->itsStkAxis>-1){
			    casa::Stokes stk;
			    blc(this->itsStkAxis) = trc(this->itsStkAxis) = this->itsInputCoords.stokesPixelNumber(stk.name(this->itsCurrentStokes));
			}

			for(int x=this->itsSource->getXmin(); x<=this->itsSource->getXmax();x++) {
			    for(int y=this->itsSource->getYmin(); y<=this->itsSource->getYmax();y++){
				if(spatmap.isInObject(x,y)){
				    blc(this->itsLngAxis)=trc(this->itsLngAxis)=x-this->itsSource->getXmin(); 
				    blc(this->itsLatAxis)=trc(this->itsLatAxis)=y-this->itsSource->getYmin();
				    casa::Array<Float> spec=subarray(blc,trc,inc).reform(this->itsArray(outBLC,outTRC).shape());
				    this->itsArray(outBLC,outTRC) = this->itsArray(outBLC,outTRC) + spec;
				}
			    }
			}
		    }
      
		    delete sub;

		    this->closeInput();
		}
		else ASKAPLOG_ERROR_STR(logger, "Could not open image");
	    }


	    if(this->itsFlagDoScale){
		
		if(this->itsBeamScaleFactor.size()==1){
		    this->itsArray /= this->itsBeamScaleFactor[0];
		}
		else{
		    casa::IPosition start(this->itsArray.ndim(),0),end=this->itsArray.shape()-1;
		    start(this->itsLngAxis)=start(this->itsLatAxis)=0;
		    for(int z=0;z<this->itsArray.shape()(this->itsSpcAxis);z++){
			start(this->itsSpcAxis) = end(this->itsSpcAxis) = z;
			this->itsArray(start,end) = this->itsArray(start,end) / this->itsBeamScaleFactor[z];
		    }
		}

	    }
		
	}
	
	
    }
}
