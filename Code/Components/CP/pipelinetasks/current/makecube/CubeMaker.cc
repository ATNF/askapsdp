/// @file CubeMaker.cc
///
/// 
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
#include <makecube/CubeMaker.h>
#include <makecube/CubeMakerHelperFunctions.h>
#include <askap_pipelinetasks.h>

#include <askap/AskapError.h>
#include <askap/AskapLogging.h>

#include <Common/ParameterSet.h>
#include <casa/Arrays/IPosition.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <images/Images/PagedImage.h>
#include <casa/Quanta/Unit.h>

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

ASKAP_LOGGER(logger, ".CubeMaker");

namespace askap {
    namespace cp {
	namespace pipelinetasks {

	    CubeMaker::CubeMaker(const LOFAR::ParameterSet &parset)
	    {
		this->itsInputNamePattern = parset.getString("inputNamePattern","");
		this->itsCubeName = parset.getString("outputCube","");
		this->itsRestFrequency = parset.getDouble("restFrequency",-1.);
		this->itsBeamReference = parset.getString("beamReference","mid");
		this->itsBeamFile = parset.getString("beamFile","");
		this->itsCube = 0;
	    }

	    CubeMaker::~CubeMaker()
	    {
		if(this->itsCube!=0) delete this->itsCube;
	    }

	    void CubeMaker::initialise()
	    {
		this->itsInputNames = expandPattern(this->itsInputNamePattern);
		if (this->itsInputNames.size() < 2) ASKAPTHROW(AskapError,"Insufficient input files");
		this->itsNumChan = this->itsInputNames.size();
		
		if(this->itsBeamReference == "mid") this->itsBeamImageNum = this->itsNumChan / 2;
		else if(this->itsBeamReference == "first") this->itsBeamImageNum = 0;
		else if(this->itsBeamReference == "last") this->itsBeamImageNum = this->itsNumChan-1;
		else { 
		    int num = atoi(this->itsBeamReference.c_str());
		    if(num>=0 && num<this->itsNumChan) this->itsBeamImageNum = size_t(num);
		    else{
			ASKAPLOG_WARN_STR(logger, "beamReference value ("<<this->itsBeamReference<<") not valid. Using middle value of " << this->itsNumChan/2);
			this->itsBeamImageNum = this->itsNumChan / 2;
		    }
		}

		this->getReferenceData();
		this->getSecondCoordinates();
	    }

	    void CubeMaker::getReferenceData()
	    {
		// Get reference data. This will be used to construct the cube, and later
		// verified to be consistant in the rest of the images
		const casa::PagedImage<float> refImage(this->itsInputNames[0]);
		this->itsRefShape = refImage.shape();
		this->itsRefCoordinates = refImage.coordinates();
		this->itsRefUnits = refImage.units();
	    }

	    void CubeMaker::getSecondCoordinates()
	    {
		const casa::PagedImage<float> secondImage(this->itsInputNames[1]);
		this->itsSecondCoordinates = secondImage.coordinates();
	    }

	    void CubeMaker::createCube()
	    {
		// Create new image cube
		casa::CoordinateSystem newCsys = makeCoordinates(this->itsRefCoordinates,
								 this->itsSecondCoordinates, this->itsRefShape);

		if(this->itsRestFrequency>0.) this->setRestFreq(newCsys);
		    
		const casa::IPosition cubeShape(4, this->itsRefShape(0), this->itsRefShape(1), this->itsRefShape(2), this->itsNumChan);
		const double size = static_cast<double>(cubeShape.product()) * sizeof(float);
		ASKAPLOG_INFO_STR(logger, "Creating image cube " << this->itsCubeName 
				  << "  of size approximately " << std::setprecision(2)
				  << (size / 1024.0 / 1024.0 / 1024.0) << "GB. This may take a few minutes.");

		this->itsCube = new casa::PagedImage<float>(casa::TiledShape(cubeShape), newCsys, this->itsCubeName);
	    }

	    void CubeMaker::setRestFreq(casa::CoordinateSystem &csys)
	    {
		assertValidCoordinates(csys);
		const int whichSpectral = csys.findCoordinate(casa::Coordinate::SPECTRAL);
		casa::SpectralCoordinate speccoord = csys.spectralCoordinate(whichSpectral);
		if(!speccoord.setRestFrequency(this->itsRestFrequency))
		    ASKAPLOG_ERROR_STR(logger, "Could not set the rest frequency to " << this->itsRestFrequency);
		else{
		    if(!csys.replaceCoordinate(speccoord,whichSpectral))
			ASKAPLOG_ERROR_STR(logger, "Could not set the rest frequency - error replacing the spectral coordinates");
		}
	    }

	    void CubeMaker::setImageInfo()
	    {
		if(this->itsCube){
		    this->itsCube->setUnits(this->itsRefUnits);
		    casa::PagedImage<float> midImage(this->itsInputNames[this->itsBeamImageNum]);
		    this->itsCube->setImageInfo(midImage.imageInfo());
		}
	    }

	    void CubeMaker::writeSlices()
	    {
		for (size_t i = 0; i < this->itsInputNames.size(); ++i) {
		    if(!this->writeSlice(i))
			ASKAPTHROW(AskapError,"Could not write slice #"<<i);
		}
	    }

	    bool CubeMaker::writeSlice(size_t i)
	    {
		if(this->itsCube){
		    
		    ASKAPLOG_INFO_STR(logger, "Adding slice from image " << this->itsInputNames[i]);
		    casa::PagedImage<float> img(this->itsInputNames[i]);

		    // Ensure shape is the same
		    if (img.shape() != this->itsRefShape) {
			ASKAPLOG_ERROR_STR(logger, "Error: Input images must all have the same shape");
			return false;
		    }

		    // Ensure coordinate system is the same
		    if (!compatibleCoordinates(img.coordinates(), this->itsRefCoordinates)) {
			ASKAPLOG_ERROR_STR(logger,
					   "Error: Input images must all have compatible coordinate systems");
			return false;
		    }

		    // Ensure units are the same
		    if (img.units() != this->itsRefUnits) {
			ASKAPLOG_ERROR_STR(logger, "Error: Input images must all have the same units");
			return false;
		    }

		    casa::Array<float> arr = img.get();
		    casa::IPosition where(4, 0, 0, 0, i);
		    this->itsCube->putSlice(arr, where);
		    return true;

		}
		else {
		    ASKAPLOG_ERROR_STR(logger, "Cube not open");
		    return false;
		}

	    }

	    void CubeMaker::recordBeams()
	    {
		if(this->itsBeamFile!=""){
		    std::ofstream fbeam(this->itsBeamFile.c_str());
		    for(size_t i=0;i<this->itsInputNames.size();i++){
			casa::PagedImage<float> img(this->itsInputNames[i]);
			casa::Vector<Quantum<Double> > beam=img.imageInfo().restoringBeam();
			fbeam << this->itsInputNames[i] << " " 
			      << beam[0].getValue("arcsec") << " " 
			      << beam[1].getValue("arcsec") << " " 
			      << beam[2].getValue("deg") <<"\n";
		    }
		    
		}
	    }

	}
    }
}
    
