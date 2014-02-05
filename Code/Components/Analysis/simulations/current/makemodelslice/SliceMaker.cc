/// @file
///
/// XXX Notes on program XXX
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
#include <makemodelslice/SliceMaker.h>

#include <askap_simulations.h>

#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <simulationutilities/SimulationUtilities.h>
#include <analysisparallel/SubimageDef.h>

#include <coordinates/Coordinates/CoordinateSystem.h>
#include <casa/Quanta/Unit.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
#include <images/Images/PagedImage.h>
#include <boost/scoped_ptr.hpp>
#include <Common/ParameterSet.h>

#include <duchamp/Utils/Section.hh>

ASKAP_LOGGER(logger, ".sliceMaker");

namespace askap {

    namespace simulations {

	SliceMaker::SliceMaker(const LOFAR::ParameterSet& parset)
	{
	    this->itsModelName = parset.getString("modelname");
	    this->itsSliceName = parset.getString("slicename");
	    this->itsSubimageDef = analysisutilities::SubimageDef(parset);
	    this->itsNumChunks = this->itsSubimageDef.nsubx() * this->itsSubimageDef.nsuby() * this->itsSubimageDef.nsubz();
//	    this->itsSliceShape = casa::IPosition(parset.getIntVector("sliceshape"));

	    this->itsNpix = parset.getIntVector("npixslice");
	    this->itsNchan = parset.getInt("nchanslice");

	    this->itsSubimageDef.define(this->itsSliceShape.size());
	    this->itsSubimageDef.setImageDim(this->itsSliceShape.asStdVector());
	    this->itsChanRange = parset.getIntVector("chanRange");
	    ASKAPASSERT(this->itsChanRange.size() == 2);
	    ASKAPCHECK(this->itsNchan == (abs(this->itsChanRange[1]-this->itsChanRange[0])+1),
		       "Channel range (["<<this->itsChanRange[0]<<","<<this->itsChanRange[1]
		       <<"]) does not match requested number of channels ("<<this->itsNchan<<")");
	    
	}


	void SliceMaker::initialise()
	{

	    if(this->itsNumChunks == 1)
		this->itsChunkList.push_back(this->itsModelName);
	    else{
		for(unsigned int i=0; i<this->itsNumChunks; i++){
		    duchamp::Section sec=this->itsSubimageDef.section(i);
		    std::string loc = locationString(sec);
		    std::stringstream chunkname;
		    chunkname << this->itsModelName << "_w"  << i+1 << loc;
		    this->itsChunkList.push_back(chunkname.str());
		}
	    }

	    const casa::PagedImage<float> refImage(this->itsChunkList[0]);
	    this->itsRefShape = refImage.shape();
	    this->itsRefCoordinates = refImage.coordinates();
	    this->itsRefUnits = refImage.units();

	    this->itsSpcAxis = this->itsRefCoordinates.spectralAxisNumber();
	    this->itsLngAxis = this->itsRefCoordinates.directionAxesNumbers()[0];
	    this->itsLatAxis = this->itsRefCoordinates.directionAxesNumbers()[1];

	    this->itsSliceShape = casa::IPosition(this->itsRefShape);
	    this->itsSliceShape[this->itsLngAxis] = this->itsNpix[0];
	    this->itsSliceShape[this->itsLatAxis] = this->itsNpix[1];
	    this->itsSliceShape[this->itsSpcAxis] = this->itsNchan;

//	    ASKAPASSERT(this->itsSliceShape[this->itsSpcAxis] == (fabs(this->itsChanRange[1]-this->itsChanRange[0])+1));

	}

	void SliceMaker::createSlice()
	{

	    casa::CoordinateSystem newCoords = this->itsRefCoordinates;
	    casa::Vector<double> refPix = newCoords.referencePixel();
	    refPix[this->itsLngAxis] = this->itsSliceShape[this->itsLngAxis]/2.;
	    refPix[this->itsLatAxis] = this->itsSliceShape[this->itsLatAxis]/2.;
	    refPix[this->itsSpcAxis] -= this->itsChanRange[0];
	    ASKAPASSERT(newCoords.setReferencePixel(refPix));
    
	    const double size = static_cast<double>(this->itsSliceShape.product()) * sizeof(float);
	    ASKAPLOG_INFO_STR(logger, "Creating image cube " << itsSliceName
			      << " of shape " << this->itsSliceShape
			      << " and size approximately " << std::setprecision(2)
			      << (size / 1024.0 / 1024.0) << "MB.");
	    itsSlice.reset(new casa::PagedImage<float>(casa::TiledShape(this->itsSliceShape), newCoords, itsSliceName));

	    itsSlice->setUnits(this->itsRefUnits);

	}

	const void SliceMaker::writeChunks()
	{

	    casa::IPosition stride(this->itsSliceShape.size(),1);
	    for (unsigned int i=0; i<this->itsNumChunks; i++){

		ASKAPLOG_DEBUG_STR(logger, "Reading image " << this->itsChunkList[i]);
		const casa::PagedImage<float> img(this->itsChunkList[i]);
		ASKAPLOG_DEBUG_STR(logger, "Image has shape " << img.shape());

		casa::IPosition blc(4,0);
		casa::IPosition trc = img.shape()-1;
		blc[this->itsSpcAxis] = std::min(this->itsChanRange[0],this->itsChanRange[1]);
		trc[this->itsSpcAxis] = std::max(this->itsChanRange[0],this->itsChanRange[1]);
		casa::Slicer slicer(blc,trc,casa::Slicer::endIsLast);
		ASKAPLOG_DEBUG_STR(logger, "Will use slicer " << slicer << " to extract");

		casa::Array<float> arr=img.getSlice(slicer);
	
		casa::IPosition where = this->itsSubimageDef.blc(i);
		this->itsSlice->putSlice(arr,where,stride);

	    }

	}

    }

}
