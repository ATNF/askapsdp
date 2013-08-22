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

#include <duchamp/PixelMap/Voxel.hh>

#include <Common/ParameterSet.h>

using namespace askap::analysis::sourcefitting;

ASKAP_LOGGER(logger, ".momentmapextractor");



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

	    if(this->openInput()){
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
		ASKAPLOG_DEBUG_STR(logger, "Defined slicer for moment map extraction as : " << this->itsSlicer);
		this->closeInput();
		this->initialiseArray();
	    }
	    else ASKAPLOG_ERROR_STR(logger, "Could not open image");
	}

	void MomentMapExtractor::initialiseArray()
	{
	    if(this->openInput()){
		int lngsize = this->itsSlicer.length()(this->itsLngAxis);
		int latsize = this->itsSlicer.length()(this->itsLatAxis);
		casa::IPosition shape(4,lngsize,latsize,1,1);
		ASKAPLOG_DEBUG_STR(logger, "Moment map extraction: Initialising array to zero with shape " << shape);
		this->itsArray = casa::Array<Float>(shape,0.0);
		this->closeInput();
	    }
	    else ASKAPLOG_ERROR_STR(logger, "Could not open image");
	}

	void MomentMapExtractor::extract()
	{
	    this->defineSlicer();
	    if(this->openInput()){
	    
		ASKAPLOG_INFO_STR(logger, "Extracting moment map from " << this->itsInputCube << " surrounding source ID " << this->itsSource->getID());
		
		const SubImage<Float> *sub = new SubImage<Float>(*this->itsInputCubePtr, this->itsSlicer);
		ASKAPASSERT(sub->size()>0);
		const casa::MaskedArray<Float> msub(sub->get(),sub->getMask());
		casa::Array<Float> subarray(sub->shape());
		subarray = msub;
		
		casa::SpectralCoordinate spcoo(this->itsInputCoords.spectralCoordinate(this->itsInputCoords.findCoordinate(casa::Coordinate::SPECTRAL)));
		// double specIncr = fabs(spcoo.increment()[0]);
		// ASKAPLOG_DEBUG_STR(logger, "Spectral increment = " << specIncr << " " << spcoo.worldAxisUnits()[0]);
		double vel1,vel2;
		spcoo.pixelToVelocity(vel1,0);
		spcoo.pixelToVelocity(vel2,1);
		double specIncr = fabs(vel1-vel2);
		ASKAPLOG_DEBUG_STR(logger, "Velocity increment = " << specIncr << " " << spcoo.velocityUnit());
		
		casa::IPosition outloc(4,0),inloc(4,0);
		casa::IPosition start=this->itsSlicer.start();
		std::vector<PixelInfo::Voxel> voxlist=this->itsSource->getPixelSet();
		std::vector<PixelInfo::Voxel>::iterator vox;
		for(vox=voxlist.begin();vox!=voxlist.end();vox++){
		    outloc(this->itsLngAxis) = inloc(this->itsLngAxis) = vox->getX() - start(this->itsLngAxis);
		    outloc(this->itsLatAxis) = inloc(this->itsLatAxis) = vox->getY() - start(this->itsLatAxis);
		    inloc(this->itsSpcAxis) = vox->getZ() - start(this->itsSpcAxis);
		    this->itsArray(outloc) = this->itsArray(outloc) + subarray(inloc);
		}
		this->itsArray(outloc) *= specIncr;
		
		// for(int y=this->itsSlicer.start()(this->itsLatAxis);y<=this->itsSlicer.end()(this->itsLatAxis);y++){
		// 	outloc(this->itsLatAxis)=inloc(this->itsLatAxis)=y-this->itsSlicer.start()(this->itsLatAxis);
		// 	for(int x=this->itsSlicer.start()(this->itsLngAxis);x<=this->itsSlicer.end()(this->itsLngAxis);x++){
		// 	    outloc(this->itsLngAxis)=inloc(this->itsLngAxis)=x-this->itsSlicer.start()(this->itsLngAxis);
		// 	    // for each pixel in the moment map...
		
		// 	    int zmin,zmax;
		// 	    if(this->itsSpatialMethod=="box"){
		// 		zmin = this->itsSource->getZmin();
		// 		zmax = this->itsSource->getZmax();
		// 	    }
		// 	    else{
		// 		zmin = this->itsSlicer.start()(this->itsSpcAxis);
		// 		zmax = this->itsSlicer.end()(this->itsSpcAxis);
		// 	    }

		// 	    for(int z=zmin;z<=zmax;z++){
		// 		if(this->itsSource->isInObject(x,y,z)){
		// 		    inloc(this->itsSpcAxis)=z-this->itsSlicer.start()(this->itsSpcAxis);
		// 		    this->itsArray(outloc) = this->itsArray(outloc) + subarray(inloc);
		// 		    // ASKAPLOG_DEBUG_STR(logger, x << " " << y << " " << z << " yes " << outloc << " " << inloc << " "<< this->itsArray(outloc));
		// 		}
		// 	    }
		// 	    this->itsArray(outloc) *= specIncr;

		// 	}
		// }

		delete sub;
	    
		this->closeInput();
	    }
	    else ASKAPLOG_ERROR_STR(logger, "Could not open image");
	}

	void MomentMapExtractor::writeImage()
	{
	    ASKAPLOG_INFO_STR(logger, "Writing moment map to " << this->itsOutputFilename);
	    accessors::CasaImageAccess ia;

	    this->itsInputCube = this->itsInputCubeList[0];
	    if(this->openInput()){
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
		casa::Vector<Float> shift(outshape.size(),0), incrFac(outshape.size(),1);
		shift(lngAxis)=this->itsSource->getXmin()-this->itsPadSize;//this->itsXloc-outshape(lngAxis)/2;
		shift(latAxis)=this->itsSource->getYmin()-this->itsPadSize;//this->itsYloc-outshape(latAxis)/2;
		casa::Vector<Int> newshape=outshape.asVector();

		// ASKAPLOG_DEBUG_STR(logger, "New coordinate ref vals = " << newcoo.referenceValue());
		// ASKAPLOG_DEBUG_STR(logger, "New coordinate ref pixs = " << newcoo.referencePixel());
		newcoo.subImageInSitu(shift,incrFac,newshape);
		// ASKAPLOG_DEBUG_STR(logger, "New coordinate ref vals = " << newcoo.referenceValue());
		// ASKAPLOG_DEBUG_STR(logger, "New coordinate ref pixs = " << newcoo.referencePixel());

		Array<Float> newarray(this->itsArray.reform(outshape));

		ia.create(this->itsOutputFilename,newarray.shape(),newcoo);

		/// @todo save the new units - if units were per beam, remove this factor
      
		// write the array
		ia.write(this->itsOutputFilename,newarray);

		std::string newunits=this->itsInputCubePtr->units().getName() + " " + spcoo.velocityUnit();
		ia.setUnits(this->itsOutputFilename, newunits);

		this->closeInput();
	    }
	    else ASKAPLOG_ERROR_STR(logger, "Could not open image");
	}

    }

}
