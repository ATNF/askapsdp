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
/// @author XXX XXX <XXX.XXX@csiro.au>
///
#include <extraction/MomentMapExtractor.h>
#include <askap_analysis.h>
#include <extraction/SourceDataExtractor.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <sourcefitting/RadioSource.h>

#include <imageaccess/CasaImageAccess.h>

#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Slicer.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/ImageOpener.h>
#include <images/Images/FITSImage.h>
#include <images/Images/MIRIADImage.h>
#include <images/Images/SubImage.h>
#include <coordinates/Coordinates/CoordinateUtil.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/StokesCoordinate.h>
#include <measures/Measures/Stokes.h>

#include <Common/ParameterSet.h>

using namespace askap::analysis::sourcefitting;

ASKAP_LOGGER(logger, ".cubecutoutextractor");



namespace askap {

    namespace analysis { 

	MomentMapExtractor::MomentMapExtractor(const LOFAR::ParameterSet& parset):
	    SourceDataExtractor(parset)
	{
	    this->itsSpatialMethod = parset.getString("SpatialMethod","box");
	    if(this->itsSpatialMethod != "fullfield" && this->itsSpatialMethod != "box"){
		ASKAPLOG_WARN_STR(logger, "The value of SpatialMethod='"<<this->itsSpatialMethod<<"' is not recognised - setting SpatialMethod='box'");
		this->itsSpatialMethod="box";
	    }

	    this->itsPadSize = parset.getUint("padsize",5);

	    this->itsOutputFilenameBase = parset.getString("momentOutputBase","");

	}
	
	MomentMapExtractor::MomentMapExtractor(const MomentMapExtractor& other)
	{
	    this->operator=(other);
	}

	MomentMapExtractor& MomentMapExtractor::operator= (const MomentMapExtractor& other)
	{
	    if(this == &other) return *this;
	    ((SourceDataExtractor &) *this) = other;
	    this->itsSpatialMethod = other.itsSpatialMethod;
	    this->itsPadSize = other.itsPadSize;
	    return *this;
	}
	
	void MomentMapExtractor::defineSlicer()
	{

	    this->openInput();
	    IPosition shape = this->itsInputCubePtr->shape();
	    casa::IPosition blc(shape.size(),0);
	    casa::IPosition trc=shape-1;

	    if(this->itsSpatialMethod == "box") {
		long zero=0;
		blc(this->itsLngAxis) = std::max(zero, this->itsSource->getXmin()-this->itsPadSize);
		blc(this->itsLatAxis) = std::max(zero, this->itsSource->getYmin()-this->itsPadSize);
		trc(this->itsLngAxis) = std::min(shape(this->itsLngAxis)-1, this->itsSource->getXmax()+this->itsPadSize);
		trc(this->itsLatAxis) = std::min(shape(this->itsLatAxis)-1, this->itsSource->getYmax()+this->itsPadSize);
		/// @todo Not yet dealing with Stokes axis properly.
	    }
	    else if(this->itsSpatialMethod == "fullfield") {
		// Don't need to do anything here, as we use the Slicer based on the full image shape.
	    }
	    else ASKAPTHROW(AskapError,"Incorrect value for method ('"<<this->itsSpatialMethod<<"') in cube cutout");

	    this->itsSlicer = casa::Slicer(blc,trc,casa::Slicer::endIsLast);
	    this->initialiseArray();

	}

	void MomentMapExtractor::initialiseArray()
	{
	    this->openInput();
	    int lngsize = this->itsSlicer.length()(this->itsLngAxis);
	    int latsize = this->itsSlicer.length()(this->itsLatAxis);
	    casa::IPosition shape(4,lngsize,latsize,1,1);
	    ASKAPLOG_DEBUG_STR(logger, "Moment map extraction: Initialising array to zero with shape " << shape);
	    this->itsArray = casa::Array<Float>(shape,0.0);
	    this->closeInput();
	}


	void MomentMapExtractor::extract()
	{
	    this->defineSlicer();
	    this->openInput();
	    
	    ASKAPLOG_INFO_STR(logger, "Extracting noise spectrum from " << this->itsInputCube << " surrounding source ID " << this->itsSource->getID());
	    
	    const SubImage<Float> *sub = new SubImage<Float>(*this->itsInputCubePtr, this->itsSlicer);
	    ASKAPASSERT(sub->size()>0);
	    const casa::MaskedArray<Float> msub(sub->get(),sub->getMask());
	    casa::Array<Float> subarray(sub->shape());
	    subarray = msub;

	    casa::SpectralCoordinate spcoo(this->itsInputCoords.spectralCoordinate(this->itsInputCoords.findCoordinate(casa::Coordinate::SPECTRAL)));
	    double specIncr = spcoo.increment()[0];

	    casa::IPosition loc(4,0);
	    for(int y=this->itsSlicer.start()(this->itsLatAxis);y<=this->itsSlicer.end()(this->itsLatAxis);y++){
		loc(this->itsLatAxis)=y;
		for(int x=this->itsSlicer.start()(this->itsLngAxis);x<=this->itsSlicer.end()(this->itsLngAxis);x++){
		    loc(this->itsLngAxis)=x;
		    // for each pixel in the moment map...
		    
		    int zmin,zmax;
		    if(this->itsSpatialMethod=="box"){
			zmin = this->itsSource->getZmin();
			zmax = this->itsSource->getZmax();
		    }
		    else{
			zmin = this->itsSlicer.start()(this->itsSpcAxis);
			zmax = this->itsSlicer.end()(this->itsSpcAxis);
		    }

		    for(int z=zmin;z<=zmax;z++){
			loc(this->itsSpcAxis)=z;
			this->itsArray(loc) += subarray(loc);
		    }
		    this->itsArray(loc) *= specIncr;

		}
	    }

	    delete sub;
	    
	    this->closeInput();
	}

	void MomentMapExtractor::writeImage()
	{
	    ASKAPLOG_INFO_STR(logger, "Writing moment map to " << this->itsOutputFilename);
	    accessors::CasaImageAccess ia;

	    this->itsInputCube = this->itsInputCubeList[0];
	    this->openInput();
	    IPosition inshape = this->itsInputCubePtr->shape();
	    casa::CoordinateSystem newcoo=casa::CoordinateUtil::defaultCoords4D();
	    casa::DirectionCoordinate dircoo(this->itsInputCoords.directionCoordinate(this->itsInputCoords.findCoordinate(casa::Coordinate::DIRECTION)));
	    casa::SpectralCoordinate spcoo(this->itsInputCoords.spectralCoordinate(this->itsInputCoords.findCoordinate(casa::Coordinate::SPECTRAL)));
	    casa::Vector<Int> stkvec(this->itsStokesList.size());
	    for(size_t i=0;i<stkvec.size();i++) stkvec[i]=this->itsStokesList[i];
	    casa::StokesCoordinate stkcoo(stkvec);
	    newcoo.replaceCoordinate(dircoo,newcoo.findCoordinate(casa::Coordinate::DIRECTION));
	    newcoo.replaceCoordinate(spcoo,newcoo.findCoordinate(casa::Coordinate::SPECTRAL));
	    newcoo.replaceCoordinate(stkcoo,newcoo.findCoordinate(casa::Coordinate::STOKES));

	    // shift the reference pixel for the spatial coords, so that the RA/DEC (or whatever) are correct. Leave the spectral/stokes axes untouched.
	    int lngAxis=newcoo.directionAxesNumbers()[0];
	    int latAxis=newcoo.directionAxesNumbers()[1];
	    int stkAxis=newcoo.polarizationAxisNumber();
	    casa::IPosition outshape(4,1);
	    outshape(lngAxis)=this->itsSlicer.length()(this->itsLngAxis);
	    outshape(latAxis)=this->itsSlicer.length()(this->itsLatAxis);
	    outshape(stkAxis)=stkvec.size();
	    casa::Vector<Float> shift(outshape.size(),0), incrFrac(outshape.size(),1);
	    shift(lngAxis)=this->itsXloc;
	    shift(latAxis)=this->itsYloc;
	    casa::Vector<Int> newshape=outshape.asVector();
	    newcoo.subImage(shift,incrFrac,newshape);

	    Array<Float> newarray(this->itsArray.reform(outshape));

	    ia.create(this->itsOutputFilename,newarray.shape(),newcoo);

	    /// @todo save the new units - if units were per beam, remove this factor
      
	    // write the array
	    ia.write(this->itsOutputFilename,newarray);
	    ia.setUnits(this->itsOutputFilename, this->itsInputCubePtr->units().getName());

	    this->closeInput();

	}

    }

}
