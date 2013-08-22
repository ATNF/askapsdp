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
#include <extraction/CubeletExtractor.h>
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

ASKAP_LOGGER(logger, ".cubeletextractor");



namespace askap {

    namespace analysis { 

	CubeletExtractor::CubeletExtractor(const LOFAR::ParameterSet& parset):
	    SourceDataExtractor(parset)
	{
	    std::vector<unsigned int> padsizes = parset.getUintVector("padsize",std::vector<unsigned int>(2,5));
	    if(padsizes.size()>2) ASKAPLOG_WARN_STR(logger, "Only using the first two elements of the padsize vector");
	    this->itsSpatialPad = padsizes[0];
	    if(padsizes.size()>1) 
		this->itsSpectralPad = padsizes[1];
	    else
		this->itsSpectralPad = padsizes[0];	    

	    this->itsOutputFilenameBase = parset.getString("cubeletOutputBase","");


	}
	
	CubeletExtractor::CubeletExtractor(const CubeletExtractor& other)
	{
	    this->operator=(other);
	}

	CubeletExtractor& CubeletExtractor::operator= (const CubeletExtractor& other)
	{
	    if(this == &other) return *this;
	    ((SourceDataExtractor &) *this) = other;
	    this->itsSpatialPad = other.itsSpatialPad;
	    this->itsSpectralPad = other.itsSpectralPad;
	    return *this;
	}
	
	void CubeletExtractor::defineSlicer()
	{

	    if(this->openInput()){
		IPosition shape = this->itsInputCubePtr->shape();
		casa::IPosition blc(shape.size(),0);
		casa::IPosition trc=shape-1;

		long zero=0;
		blc(this->itsLngAxis) = std::max(zero, this->itsSource->getXmin()-this->itsSpatialPad);
		blc(this->itsLatAxis) = std::max(zero, this->itsSource->getYmin()-this->itsSpatialPad);
		blc(this->itsSpcAxis) = std::max(zero, this->itsSource->getZmin()-this->itsSpectralPad);

		trc(this->itsLngAxis) = std::min(shape(this->itsLngAxis)-1, this->itsSource->getXmax()+this->itsSpatialPad);
		trc(this->itsLatAxis) = std::min(shape(this->itsLatAxis)-1, this->itsSource->getYmax()+this->itsSpatialPad);
		trc(this->itsSpcAxis) = std::min(shape(this->itsSpcAxis)-1, this->itsSource->getZmax()+this->itsSpectralPad);
		/// @todo Not yet dealing with Stokes axis properly.

		this->itsSlicer = casa::Slicer(blc,trc,casa::Slicer::endIsLast);
		this->closeInput();
		this->initialiseArray();
	    }
	    else ASKAPLOG_ERROR_STR(logger, "Could not open image");
	}

	void CubeletExtractor::initialiseArray()
	{
	    if(this->openInput()){
		int lngsize = this->itsSlicer.length()(this->itsLngAxis);
		int latsize = this->itsSlicer.length()(this->itsLatAxis);
		int spcsize = this->itsSlicer.length()(this->itsSpcAxis);
		casa::IPosition shape(this->itsInputCubePtr->shape().size(),1);
		shape(this->itsLngAxis)=lngsize;
		shape(this->itsLatAxis)=latsize;
		shape(this->itsSpcAxis)=spcsize;
		ASKAPLOG_DEBUG_STR(logger, "Cubelet extraction: Initialising array to zero with shape " << shape);
		this->itsArray = casa::Array<Float>(shape,0.0);
		this->closeInput();
	    }
	    else ASKAPLOG_ERROR_STR(logger, "Could not open image");
	}
	    
	void CubeletExtractor::extract()
	{
	    this->defineSlicer();
	    if(this->openInput()){
	    
		ASKAPLOG_INFO_STR(logger, "Extracting noise spectrum from " << this->itsInputCube << " surrounding source ID " << this->itsSource->getID());
		
		const SubImage<Float> *sub = new SubImage<Float>(*this->itsInputCubePtr, this->itsSlicer);
		ASKAPASSERT(sub->size()>0);
		const casa::MaskedArray<Float> msub(sub->get(),sub->getMask());
		ASKAPASSERT(this->itsArray.size() == msub.size());
		this->itsArray = msub;
		
		delete sub;
		
		this->closeInput();
	    }
	    else ASKAPLOG_ERROR_STR(logger, "Could not open image");
	}

	void CubeletExtractor::writeImage()
	{
	    ASKAPLOG_INFO_STR(logger, "Writing cube cutout to " << this->itsOutputFilename);
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
		int spcAxis=newcoo.spectralAxisNumber();
		int stkAxis=newcoo.polarizationAxisNumber();
		casa::IPosition outshape(4,1);
		outshape(lngAxis)=this->itsSlicer.length()(this->itsLngAxis);
		outshape(latAxis)=this->itsSlicer.length()(this->itsLatAxis);
		outshape(spcAxis)=this->itsSlicer.length()(this->itsSpcAxis);
		outshape(stkAxis)=stkvec.size();
		casa::Vector<Float> shift(outshape.size(),0), incrFac(outshape.size(),1);
		shift(lngAxis)=this->itsSource->getXmin()-this->itsSpatialPad;
		shift(latAxis)=this->itsSource->getYmin()-this->itsSpatialPad;
		shift(spcAxis)=this->itsSource->getZmin()-this->itsSpectralPad;
		casa::Vector<Int> newshape=outshape.asVector();
		
		// ASKAPLOG_DEBUG_STR(logger, "New coordinate ref vals = " << newcoo.referenceValue());
		// ASKAPLOG_DEBUG_STR(logger, "New coordinate ref pixs = " << newcoo.referencePixel());
		newcoo.subImageInSitu(shift,incrFac,newshape);
		// ASKAPLOG_DEBUG_STR(logger, "New coordinate ref vals = " << newcoo.referenceValue());
		// ASKAPLOG_DEBUG_STR(logger, "New coordinate ref pixs = " << newcoo.referencePixel());
		
		Array<Float> newarray(this->itsArray.reform(outshape));
		
		ia.create(this->itsOutputFilename,newarray.shape(),newcoo);
		
		// write the array
		ia.write(this->itsOutputFilename,newarray);
		
		this->closeInput();
		
	    }		
	    else ASKAPLOG_ERROR_STR(logger, "Could not open image");
	}

    }

}
