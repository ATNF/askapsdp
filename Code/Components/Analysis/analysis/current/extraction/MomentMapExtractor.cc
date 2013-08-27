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

#include <string>
#include <iostream>
#include <sstream>

#include <sourcefitting/RadioSource.h>

#include <imageaccess/CasaImageAccess.h>

#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayLogical.h>
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
	    this->itsSpatialMethod = parset.getString("spatialMethod","box");
	    if(this->itsSpatialMethod != "fullfield" && this->itsSpatialMethod != "box"){
		ASKAPLOG_WARN_STR(logger, "The value of spatialMethod='"<<this->itsSpatialMethod<<"' is not recognised - setting spatialMethod='box'");
		this->itsSpatialMethod="box";
	    }
	    this->itsFlagUseDetection = parset.getBool("useDetectedPixels",true);

	    this->itsPadSize = parset.getUint("padsize",5);

	    this->itsOutputFilenameBase = parset.getString("momentOutputBase","");

	    for(int i=0;i<3;i++) this->itsMomentRequest[i]=false;
	    std::vector<int> request = parset.getIntVector("moments",std::vector<int>(1,0));
	    bool haveDud=false;
	    for(size_t i=0;i<request.size();i++){
		if(request[i]<0 || request[i]>2) haveDud=true;
		else this->itsMomentRequest[i]=true;
	    }
	    std::vector<int> momentsUsed;
	    for(int i=0;i<3;i++) if(this->itsMomentRequest[i]) momentsUsed.push_back(i);
	    if(haveDud) ASKAPLOG_WARN_STR(logger, "You requested invalid moments. Only doing " << casa::Vector<Int>(momentsUsed));
	    else ASKAPLOG_INFO_STR(logger, "Will compute the following moments " << casa::Vector<Int>(momentsUsed));

	    this->itsMom0map=casa::Array<Float>();
	    this->itsMom1map=casa::Array<Float>();
	    this->itsMom2map=casa::Array<Float>();

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
	    this->itsMomentRequest = other.itsMomentRequest;
	    this->itsMom0map = other.itsMom0map;
	    this->itsMom1map = other.itsMom1map;
	    this->itsMom2map = other.itsMom2map;
	    return *this;
	}
	
	void MomentMapExtractor::defineSlicer()
	{

	    if(this->openInput()){
		IPosition shape = this->itsInputCubePtr->shape();
		casa::IPosition blc(shape.size(),0);
		casa::IPosition trc=shape-1;
		
		long zero=0;
		blc(this->itsSpcAxis) = std::max(zero, this->itsSource->getZmin()-3);
		trc(this->itsSpcAxis) = std::min(shape(this->itsSpcAxis)-1, this->itsSource->getZmax()+3);

		if(this->itsSpatialMethod == "box") {
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

	casa::IPosition MomentMapExtractor::arrayShape()
	{
	    int lngsize = this->itsSlicer.length()(this->itsLngAxis);
	    int latsize = this->itsSlicer.length()(this->itsLatAxis);
	    casa::IPosition shape(4,lngsize,latsize,1,1);
	    return shape;
	}

	void MomentMapExtractor::initialiseArray()
	{
	    if(this->openInput()){
		casa::IPosition shape=this->arrayShape();
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

		if(this->itsMomentRequest[0]) this->getMom0(subarray);
		if(this->itsMomentRequest[1]) this->getMom1(subarray);
		if(this->itsMomentRequest[2]) this->getMom2(subarray);

		delete sub;
	    
		this->closeInput();
	    }
	    else ASKAPLOG_ERROR_STR(logger, "Could not open image");
	}

	void MomentMapExtractor::writeImage()
	{
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

		int lngAxis=newcoo.directionAxesNumbers()[0];
		int latAxis=newcoo.directionAxesNumbers()[1];
		int stkAxis=newcoo.polarizationAxisNumber();
		casa::IPosition outshape(4,1);
		outshape(lngAxis)=this->itsSlicer.length()(this->itsLngAxis);
		outshape(latAxis)=this->itsSlicer.length()(this->itsLatAxis);
		outshape(stkAxis)=stkvec.size();
		if(this->itsSpatialMethod == "box"){
		    // shift the reference pixel for the spatial coords, so that the RA/DEC (or whatever) are correct. Leave the spectral/stokes axes untouched.
		    // only want to do this if we are trimming.
		    casa::Vector<Float> shift(outshape.size(),0), incrFac(outshape.size(),1);
		    shift(lngAxis)=this->itsSource->getXmin()-this->itsPadSize;
		    shift(latAxis)=this->itsSource->getYmin()-this->itsPadSize;
		    casa::Vector<Int> newshape=outshape.asVector();
		    newcoo.subImageInSitu(shift,incrFac,newshape);
		}

		for(int i=0;i<3;i++){
		    if(this->itsMomentRequest[i]){
			
			casa::LogicalArray theMask;
			std::string newunits;
			switch(i){
			case 0:
			    this->itsArray = this->itsMom0map; 
			    ASKAPLOG_DEBUG_STR(logger , this->itsMom0map.shape() << " " << this->itsMom0mask.shape() << " " << outshape );
			    theMask = this->itsMom0mask.reform(outshape);
			    if(spcoo.restFrequency() > 0.) 
				newunits=this->itsInputCubePtr->units().getName() + " " + spcoo.velocityUnit();
			    else 
				newunits=this->itsInputCubePtr->units().getName() + " " + spcoo.worldAxisUnits()[0];
			    break;
			case 1: 
			    this->itsArray = this->itsMom1map; 
			    ASKAPLOG_DEBUG_STR(logger , this->itsMom1map.shape() << " " << this->itsMom1mask.shape() << " " << outshape );
			    theMask = this->itsMom1mask.reform(outshape);;
			    if(spcoo.restFrequency() > 0.) 
				newunits=spcoo.velocityUnit();
			    else 
				newunits=spcoo.worldAxisUnits()[0];
			    break;
			case 2:
			    this->itsArray = this->itsMom2map; 
			    ASKAPLOG_DEBUG_STR(logger , this->itsMom2map.shape() << " " << this->itsMom2mask.shape() << " " << outshape );
			    theMask = this->itsMom2mask.reform(outshape);;
			    if(spcoo.restFrequency() > 0.) 
				newunits=spcoo.velocityUnit();
			    else 
				newunits=spcoo.worldAxisUnits()[0];
			    break;
			}

			Array<Float> newarray(this->itsArray.reform(outshape));
			
			std::string filename=this->outfile(i);
			ASKAPLOG_INFO_STR(logger, "Writing moment-"<<i<<" map to '"<<filename <<"'");
			ia.create(filename,newarray.shape(),newcoo);

			// write the array
			ia.write(filename,newarray);
			
			ia.setUnits(filename, newunits);

			 this->writeBeam(filename);

			casa::PagedImage<float> img(filename);
			ASKAPLOG_DEBUG_STR(logger, img.shape() << " " << theMask.shape());
			img.makeMask(filename);
			img.pixelMask().put(theMask);

		    }
		}
			
		this->closeInput();
	    }
	    else ASKAPLOG_ERROR_STR(logger, "Could not open image");
	}

	std::string MomentMapExtractor::outfile(int moment)
	{
	    std::stringstream ss;
	    ss << moment;
	    std::string filename=this->itsOutputFilename;
	    size_t loc;
	    while(loc=filename.find("%m"),
		  loc!=std::string::npos){
		filename.replace(loc,2,ss.str());
	    }
	    return filename;
	}

	double MomentMapExtractor::getSpectralIncrement()
	{
	    double specIncr;
	    casa::SpectralCoordinate spcoo(this->itsInputCoords.spectralCoordinate(this->itsInputCoords.findCoordinate(casa::Coordinate::SPECTRAL)));
	    if(spcoo.restFrequency() > 0.){
		// can convert to velocity
		double vel1,vel2;
		spcoo.pixelToVelocity(vel1,0);
		spcoo.pixelToVelocity(vel2,1);
		specIncr = fabs(vel1-vel2);
		// ASKAPLOG_DEBUG_STR(logger, "Velocity increment = " << specIncr << " " << spcoo.velocityUnit());
	    }
	    else{
		// can't do velocity conversion, so just use the WCS spectral units
		specIncr = fabs(spcoo.increment()[0]);
		// ASKAPLOG_DEBUG_STR(logger, "Spectral increment = " << specIncr << " " << spcoo.worldAxisUnits()[0]);
	    }
	    return specIncr;
	}


	void MomentMapExtractor::getMom0(const casa::Array<Float> &subarray)
	{
		
	    ASKAPLOG_INFO_STR(logger, "Extracting moment-0 map");
	    this->itsMom0map = casa::Array<Float>(this->arrayShape(),0.0);
	    uint zeroInt=0;
	    casa::LogicalArray basemask = (partialNTrue(this->itsInputCubePtr->pixelMask().getSlice(this->itsSlicer),casa::IPosition(1,this->itsSpcAxis))>zeroInt).reform(this->arrayShape());
	    this->itsMom0mask = casa::LogicalArray(this->arrayShape(),false);

	    casa::IPosition outloc(4,0),inloc(4,0);
	    casa::IPosition start=this->itsSlicer.start();

	    if(this->itsFlagUseDetection){
		std::vector<PixelInfo::Voxel> voxlist=this->itsSource->getPixelSet();
		std::vector<PixelInfo::Voxel>::iterator vox;
		for(vox=voxlist.begin();vox!=voxlist.end();vox++){
		    outloc(this->itsLngAxis) = inloc(this->itsLngAxis) = vox->getX() - start(this->itsLngAxis);
		    outloc(this->itsLatAxis) = inloc(this->itsLatAxis) = vox->getY() - start(this->itsLatAxis);
		    inloc(this->itsSpcAxis) = vox->getZ() - start(this->itsSpcAxis);
		    this->itsMom0map(outloc) = this->itsMom0map(outloc) + subarray(inloc);
		    this->itsMom0mask(outloc) = true;
		}
	    }
	    else{
		// just sum each spectrum over the slicer's range.
		casa::IPosition outBLC(4,0),outTRC(this->itsMom0map.shape()-1);
		casa::Array<Float> sumarray = partialSums(subarray,casa::IPosition(1,this->itsSpcAxis));
		this->itsMom0map(outBLC,outTRC) = sumarray.reform(this->itsMom0map(outBLC,outTRC).shape());
		this->itsMom0mask(outBLC,outTRC) = true;
	    }
	    this->itsMom0mask = this->itsMom0mask && basemask;
	    this->itsMom0map *= float(this->getSpectralIncrement());
		
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
	    // 		    this->itsMom0map(outloc) = this->itsMom0map(outloc) + subarray(inloc);
	    // 		    // ASKAPLOG_DEBUG_STR(logger, x << " " << y << " " << z << " yes " << outloc << " " << inloc << " "<< this->itsMom0map(outloc));
	    // 		}
	    // 	    }
	    // 	    this->itsMom0map(outloc) *= this->getSpectralIncrement();

	    // 	}
	    // }
	    

	}

	double MomentMapExtractor::getSpecVal(int z)
	{
	    casa::SpectralCoordinate spcoo(this->itsInputCoords.spectralCoordinate(this->itsInputCoords.findCoordinate(casa::Coordinate::SPECTRAL)));
	    double specval;
	    if(spcoo.restFrequency()>0.){
		casa::Quantum<Double> vel;
		ASKAPASSERT(spcoo.pixelToVelocity(vel,double(z)));
		specval=vel.getValue();
	    }
	    else ASKAPASSERT(spcoo.toWorld(specval,double(z)));
	    return specval;
	}

	void MomentMapExtractor::getMom1(const casa::Array<Float> &subarray)
	{
	    ASKAPLOG_INFO_STR(logger, "Extracting moment-1 map");
	    this->itsMom1map = casa::Array<Float>(this->arrayShape(),0.0);
	    uint zeroInt=0;
	    casa::LogicalArray basemask = (partialNTrue(this->itsInputCubePtr->pixelMask().getSlice(this->itsSlicer),casa::IPosition(1,this->itsSpcAxis))>zeroInt).reform(this->arrayShape());
	    this->itsMom1mask = casa::LogicalArray(this->arrayShape(),false);

	    casa::IPosition start=this->itsSlicer.start();
	    
	    if(this->itsMom0map.size()==0) this->getMom0(subarray);
	    casa::Array<Float> sumNuS(this->itsMom1map.shape(),0.0);
	     casa::Array<Float> sumS = this->itsMom0map / this->getSpectralIncrement();
	    if(this->itsFlagUseDetection){
		casa::IPosition outloc(4,0),inloc(4,0);
		std::vector<PixelInfo::Voxel> voxlist=this->itsSource->getPixelSet();
		std::vector<PixelInfo::Voxel>::iterator vox;
		for(vox=voxlist.begin();vox!=voxlist.end();vox++){
		    outloc(this->itsLngAxis) = inloc(this->itsLngAxis) = vox->getX() - start(this->itsLngAxis);
		    outloc(this->itsLatAxis) = inloc(this->itsLatAxis) = vox->getY() - start(this->itsLatAxis);
		    inloc(this->itsSpcAxis) = vox->getZ() - start(this->itsSpcAxis);
		    sumNuS(outloc) = sumNuS(outloc) + subarray(inloc) * this->getSpecVal(vox->getZ());
		    this->itsMom1mask(outloc) = true;
		}
	    }
	    else{
		// just sum each spectrum over the slicer's range.
		casa::IPosition outBLC(this->itsMom1map.ndim(),0),outTRC(this->itsMom1map.shape()-1);
		casa::Array<Float> nuArray(subarray.shape(),0.);
		for (int z=0;z<subarray.shape()(this->itsSpcAxis);z++){
		    casa::IPosition blc(subarray.ndim(),0), trc=subarray.shape()-1;
		    blc(this->itsSpcAxis) = trc(this->itsSpcAxis) = z;
		    nuArray(blc,trc) = this->getSpecVal(z + start(this->itsSpcAxis));
		}
		casa::Array<Float> nuSubarray = nuArray * subarray;
		casa::Array<Float> sumarray = partialSums(nuSubarray,casa::IPosition(1,this->itsSpcAxis));
		sumNuS(outBLC,outTRC) = sumarray.reform(sumNuS(outBLC,outTRC).shape());
		this->itsMom1mask(outBLC,outTRC) = true;
	    }
	    
	    float zero=0.;
	    this->itsMom1mask = this->itsMom1mask && basemask;
	    this->itsMom1mask = this->itsMom1mask && (this->itsMom0map > zero);

	    this->itsMom1map = (sumNuS / this->itsMom0map) * this->getSpectralIncrement();

	}

	void MomentMapExtractor::getMom2(const casa::Array<Float> &subarray)
	{
	    ASKAPLOG_INFO_STR(logger, "Extracting moment-2 map");
	    this->itsMom2map = casa::Array<Float>(this->arrayShape(),0.0);
	    uint zeroInt=0;
	    casa::LogicalArray basemask = (partialNTrue(this->itsInputCubePtr->pixelMask().getSlice(this->itsSlicer),casa::IPosition(1,this->itsSpcAxis))>zeroInt).reform(this->arrayShape());
	    this->itsMom2mask = casa::LogicalArray(this->arrayShape(),false);
	    casa::IPosition start=this->itsSlicer.start();

	    if(this->itsMom1map.size()==0) this->getMom1(subarray);
	    casa::Array<Float> sumNu2S(this->itsMom2map.shape(),0.0);
	    casa::Array<Float> sumS = this->itsMom0map / this->getSpectralIncrement();
	    if(this->itsFlagUseDetection){
		casa::IPosition outloc(4,0),inloc(4,0);
		std::vector<PixelInfo::Voxel> voxlist=this->itsSource->getPixelSet();
		std::vector<PixelInfo::Voxel>::iterator vox;
		for(vox=voxlist.begin();vox!=voxlist.end();vox++){
		    outloc(this->itsLngAxis) = inloc(this->itsLngAxis) = vox->getX() - start(this->itsLngAxis);
		    outloc(this->itsLatAxis) = inloc(this->itsLatAxis) = vox->getY() - start(this->itsLatAxis);
		    inloc(this->itsSpcAxis) = vox->getZ() - start(this->itsSpcAxis);
		    sumNu2S(outloc) = sumNu2S(outloc) + subarray(inloc) * (this->getSpecVal(vox->getZ())-this->itsMom1map(outloc))*(this->getSpecVal(vox->getZ())-this->itsMom1map(outloc));
		    this->itsMom2mask(outloc) = true;
		}
	    }
	    else {
		// just sum each spectrum over the slicer's range.
		casa::IPosition outBLC(this->itsMom2map.ndim(),0),outTRC(this->itsMom2map.shape()-1);
		casa::IPosition shapeIn(subarray.shape());
		casa::IPosition shapeMap(shapeIn); shapeMap(this->itsSpcAxis)=1;
		casa::Array<Float> nu2Array(shapeIn,0.);
		casa::Array<Float> meanNu(this->itsMom1map.reform(shapeMap));
		ASKAPLOG_DEBUG_STR(logger, meanNu.shape());
		for (int z=0;z<subarray.shape()(this->itsSpcAxis);z++){
		    casa::IPosition blc(subarray.ndim(),0), trc=subarray.shape()-1;
		    blc(this->itsSpcAxis) = trc(this->itsSpcAxis) = z;
		    nu2Array(blc,trc) = this->getSpecVal(z + start(this->itsSpcAxis));
		    nu2Array(blc,trc) = (nu2Array(blc,trc) - meanNu);
		}
		// casa::Array<Float> nu2Subarray = (nu2Array-this->itsMom1map)*(nu2Array-this->itsMom1map) * subarray;
		casa::Array<Float> nu2Subarray = nu2Array * nu2Array * subarray;
		casa::Array<Float> sumarray = partialSums(nu2Subarray,casa::IPosition(1,this->itsSpcAxis));
		sumNu2S(outBLC,outTRC) = sumarray.reform(sumNu2S(outBLC,outTRC).shape());
		this->itsMom2mask(outBLC,outTRC) = true;
	    }

	    this->itsMom2map =  (sumNu2S / this->itsMom0map) * this->getSpectralIncrement();

	    float zero=0.;
	    this->itsMom2mask = this->itsMom2mask && basemask;
	    this->itsMom2mask = this->itsMom2mask && (this->itsMom0map > zero);
	    this->itsMom2mask = this->itsMom2mask && (this->itsMom2map > zero);

	    this->itsMom2map = sqrt(this->itsMom2map);


	}


    }

}
