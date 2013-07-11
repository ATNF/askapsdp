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
		/// @details Read the input parameters from the
		/// ParameterSet. Accepted parameters:
		/// 'inputNamePattern', 'outputCube', 'restFrequency',
		/// 'beamReference', 'beamFile'. Also initialises the
		/// cube pointer to zero.
		this->itsInputNamePattern = parset.getString("inputNamePattern","");
		this->itsCubeName = parset.getString("outputCube","");
		std::string restFreqString = parset.getString("restFrequency","-1.");
		this->itsBeamReference = parset.getString("beamReference","mid");
		this->itsBeamFile = parset.getString("beamFile","");
		this->itsCube = 0;

		if(restFreqString == "HI") this->itsRestFrequency = REST_FREQ_HI;
		else this->itsRestFrequency = atof(restFreqString.c_str());

	    }

	    CubeMaker::~CubeMaker()
	    {
		if(this->itsCube!=0) delete this->itsCube;
	    }

	    void CubeMaker::initialise()
	    {
		/// @details Takes the input name pattern and expands
		/// to a vector list of input filenames, using the
		/// expandPattern function. Parses the beamReference
		/// parameter to get the image number from which to
		/// read the beam information that will be stored in
		/// the output cube. Calls getReferenceData().

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
	    }

	    void CubeMaker::getReferenceData()
	    {
		/// @details The reference data details the shape of
		/// the input images, their units and
		/// coordinates. These are used for construction of
		/// the cube and verification of all input images. The
		/// reference data is read from the first image in the
		/// vector list. The coordinate system of the second
		/// image in that list is also extracted - the
		/// spectral increment will be determined from these
		/// two coordinate systems.

		const casa::PagedImage<float> refImage(this->itsInputNames[0]);
		this->itsRefShape = refImage.shape();
		this->itsRefCoordinates = refImage.coordinates();
		this->itsRefUnits = refImage.units();

		const casa::PagedImage<float> secondImage(this->itsInputNames[1]);
		this->itsSecondCoordinates = secondImage.coordinates();
	    }

	    void CubeMaker::createCube()
	    {
		/// @details The coordinate system for the cube is
		/// constructed using the makeCoordinates function. If
		/// required, the rest frequency is added. The cube is
		/// then created using the reference shape and the
		/// number of channels in the input file list.

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
		/// @details The rest frequency, as provided in the
		/// input parameter set, is added to the coordinate
		/// system, replacing any previous value that is
		/// already there.
		/// @param csys The coordinate system to which the
		/// rest frequency is to be added.

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
		/// @details If the output cube has been created, the
		/// reference units and the requested reference beam
		/// shape are added to the cube.

		if(this->itsCube){
		    this->itsCube->setUnits(this->itsRefUnits);
		    casa::PagedImage<float> midImage(this->itsInputNames[this->itsBeamImageNum]);
		    this->itsCube->setImageInfo(midImage.imageInfo());
		}
	    }

	    void CubeMaker::writeSlices()
	    {
		/// @details Each input channel image is added in order to the output cube.

		for (size_t i = 0; i < this->itsInputNames.size(); ++i) {
		    if(!this->writeSlice(i))
			ASKAPTHROW(AskapError,"Could not write slice #"<<i);
		}
	    }

	    bool CubeMaker::writeSlice(size_t i)
	    {
		/// @details An individual channel image is added to
		/// the cube in the appropriate location. Checks are
		/// performed to verify that the channel image has the
		/// same shape and units as the reference (ie. the
		/// first in the vector list), and has compatible
		/// coordinates (as defined by the
		/// compatibleCoordinates function).
		/// @param The number of the image in the vector list
		/// of input images.
		/// @return Returns true if things work. If any checks
		/// fail, the index is out of bounds, or the cube is
		/// not yet open, then false is returned (and an ERROR
		/// log message written).

		if(this->itsCube){
		    
		    if(i > this->itsInputNames.size()){
			ASKAPLOG_ERROR_STR(logger, "writeSlice - index " << i << " out of bounds");
			return false;
		    }

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
		/// @details The beam shape for each input image is
		/// written to an ascii file (given by the beamFile
		/// input parameter). Each line corresponds to one
		/// file, and has columns: number | image name | major
		/// axis [arcsec] | minor axis [arcsec] | position
		/// angle [deg]. Columns are separated by a single space. 

		casa::PagedImage<float> firstimg(this->itsInputNames[0]);
		casa::Vector<Quantum<Double> > firstbeam=firstimg.imageInfo().restoringBeam();
		
		if(this->itsBeamFile!=""){
		    if(firstbeam.size()==0)
			ASKAPLOG_WARN_STR(logger, "The first input image " << this->itsInputNames[0] << " has no beam, so not making the beamFile " << this->itsBeamFile);
		    else{
			std::ofstream fbeam(this->itsBeamFile.c_str());
			for(size_t i=0;i<this->itsInputNames.size();i++){
			    casa::PagedImage<float> img(this->itsInputNames[i]);
			    casa::Vector<Quantum<Double> > beam=img.imageInfo().restoringBeam();
			    fbeam << i << " " << this->itsInputNames[i] << " " 
				  << beam[0].getValue("arcsec") << " " 
				  << beam[1].getValue("arcsec") << " " 
				  << beam[2].getValue("deg") <<"\n";
			}
		    }
		    
		}
	    }

	}
    }
}
    
