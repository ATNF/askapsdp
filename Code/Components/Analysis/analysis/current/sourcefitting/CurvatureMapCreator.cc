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
#include <sourcefitting/CurvatureMapCreator.h>
#include <askap_analysis.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <askapparallel/AskapParallel.h>
#include <Common/ParameterSet.h>
#include <string>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>
#include <outputs/DistributedImageWriter.h>
#include <casainterface/CasaInterface.h>
#include <analysisparallel/SubimageDef.h>
#include <scimath/Mathematics/Convolver.h>
#include <duchamp/Cubes/cubes.hh>

#include <images/Images/PagedImage.h>
#include <images/Images/SubImage.h>
#include <images/Images/ImageOpener.h>
#include <images/Images/FITSImage.h>
#include <images/Images/MIRIADImage.h>
#include <casainterface/CasaInterface.h>

///@brief Where the log messages go.
ASKAP_LOGGER(logger, ".curvaturemap");

namespace askap {

    namespace analysis {


	CurvatureMapCreator::CurvatureMapCreator(askap::askapparallel::AskapParallel &comms, const LOFAR::ParameterSet &parset):
	    itsComms(&comms),itsParset(parset)
	{
	    this->itsFilename = parset.getString("curvatureImage", "");
	    ASKAPLOG_DEBUG_STR(logger, "Define a CurvatureMapCreator to write to image " << this->itsFilename);
	}
	
	CurvatureMapCreator::CurvatureMapCreator(const CurvatureMapCreator& other)
	{
	    this->operator=(other);
	}

	CurvatureMapCreator& CurvatureMapCreator::operator= (const CurvatureMapCreator& other)
	{
	    if(this == &other) return *this;
	    this->itsComms = other.itsComms;
	    this->itsParset = other.itsParset;
	    this->itsFilename = other.itsFilename;
	    this->itsArray = other.itsArray;
	    this->itsSigmaCurv = other.itsSigmaCurv;
	    return *this;
	}

	void CurvatureMapCreator::initialise(duchamp::Cube &cube, analysisutilities::SubimageDef &subdef)
	{
	    /// @details Initialise the class with information from
	    /// the duchamp::Cube. This is done to avoid replicating
	    /// parameters and preserving the parameter
	    /// hierarchy. Once the input image is known, the output
	    /// image names can be set with fixName() (if they have
	    /// not been defined via the parset).

	    this->itsCube = &cube;
	    this->itsSubimageDef = &subdef;

	    ImageOpener::registerOpenImageFunction(ImageOpener::FITS, FITSImage::openFITSImage);
	    ImageOpener::registerOpenImageFunction(ImageOpener::MIRIAD, MIRIADImage::openMIRIADImage);
	    const LatticeBase* lattPtr = ImageOpener::openImage(cube.pars().getImageFile());
	    if (lattPtr == 0)
		ASKAPTHROW(AskapError, "Requested image \"" << cube.pars().getImageFile() << "\" does not exist or could not be opened.");
	    const ImageInterface<Float>* imagePtr = dynamic_cast<const ImageInterface<Float>*>(lattPtr);
	    casa::Slicer slicer = analysisutilities::subsectionToSlicer(cube.pars().section());
	    analysisutilities::fixSlicer(slicer, cube.header().getWCS());
	    const SubImage<Float> *sub = new SubImage<Float>(*imagePtr, slicer);
	    this->itsShape = sub->shape();

	    duchamp::Section sec = this->itsSubimageDef->section(this->itsComms->rank()-1);
	    sec.parse(this->itsShape.asStdVector());
	    duchamp::Section secMaster = this->itsSubimageDef->section(-1);
	    secMaster.parse(this->itsShape.asStdVector());
	    this->itsLocation = casa::IPosition(sec.getStartList());// - casa::IPosition(secMaster.getStartList());

	    ASKAPLOG_DEBUG_STR(logger, "Initialised CurvatureMapCreator with shape="<<this->itsShape<<" and location="<<this->itsLocation);

	}


	void CurvatureMapCreator::calculate()
	{

	    // ImageOpener::registerOpenImageFunction(ImageOpener::FITS, FITSImage::openFITSImage);
	    // ImageOpener::registerOpenImageFunction(ImageOpener::MIRIAD, MIRIADImage::openMIRIADImage);
	    // const LatticeBase* lattPtr = ImageOpener::openImage(this->itsCube->pars().getImageFile());
	    // if (lattPtr == 0)
	    // 	ASKAPTHROW(AskapError, "Requested image \"" << this->itsCube->pars().getImageFile() << "\" does not exist or could not be opened.");
	    // const ImageInterface<Float>* imagePtr = dynamic_cast<const ImageInterface<Float>*>(lattPtr);
	    // casa::Slicer slicer = analysisutilities::subsectionToSlicer(this->itsCube->pars().section());
	    // analysisutilities::fixSlicer(slicer, this->itsCube->header().getWCS());
	    // const SubImage<Float> *sub = new SubImage<Float>(*imagePtr, slicer);
	    // casa::IPosition inputShape = sub->shape();

	    //casa::IPosition inputShape(2,this->itsCube->getDimX(),this->itsCube->getDimY());
	    casa::Array<float> inputArray(this->itsShape,this->itsCube->getArray(),casa::SHARE);

	    casa::IPosition kernelShape(2,3,3);
	    casa::Array<float> kernel(kernelShape,1.);
	    kernel(casa::IPosition(2,1,1)) = -8.;

	    ASKAPLOG_DEBUG_STR(logger, "Defined a kernel for the curvature map calculations: " << kernel);

	    casa::Convolver<float> convolver(kernel,this->itsShape);
	    ASKAPLOG_DEBUG_STR(logger, "Defined a convolver");

	    this->itsArray = casa::Array<float>(this->itsShape);
	    ASKAPLOG_DEBUG_STR(logger, "About to convolve");
	    convolver.linearConv(this->itsArray, inputArray);
	    ASKAPLOG_DEBUG_STR(logger, "Convolving done.");

	    this->findSigma();

	    this->maskBorders();

	}


	void CurvatureMapCreator::findSigma()
	{
	    
	    this->itsSigmaCurv = madfm(this->itsArray,False,False,False) / Statistics::correctionFactor;
	    ASKAPLOG_DEBUG_STR(logger, "Found sigma_curv = " << this->itsSigmaCurv);

	}

	void CurvatureMapCreator::maskBorders()
	{
	    int nsubx=this->itsSubimageDef->nsubx(), nsuby=this->itsSubimageDef->nsuby();
	    int overlapx=this->itsSubimageDef->overlapx()/2, overlapy=this->itsSubimageDef->overlapy()/2;
	    int rank = this->itsComms->rank()-1;
	    int xminOffset = (rank % nsubx == 0) ? 0 : overlapx;
	    int xmaxOffset = (rank % nsubx == (nsubx-1)) ? 0 : overlapx;
	    int yminOffset = (rank / nsubx == 0) ? 0 : overlapy;
	    int ymaxOffset = (rank / nsubx == (nsuby-1)) ? 0 : overlapy;
	    ASKAPLOG_DEBUG_STR(logger, "xminOffset="<<xminOffset << ", xmaxOffset="<<xmaxOffset <<", yminOffset="<<yminOffset <<", ymaxOffset="<<ymaxOffset);
	    ASKAPLOG_DEBUG_STR(logger, "Starting with location="<<this->itsLocation<<" and shape="<<this->itsShape);
	    casa::IPosition blc(this->itsLocation),trc(this->itsShape-1);
	    blc[0]=xminOffset;
	    blc[1]=yminOffset;
	    trc[0]-=xmaxOffset;
	    trc[1]-=ymaxOffset;
	    casa::Slicer arrSlicer(blc,trc,Slicer::endIsLast);
	    ASKAPLOG_DEBUG_STR(logger, "Defined a masking Slicer " << arrSlicer);
	    casa::Array<float> newArr = this->itsArray(arrSlicer);
	    ASKAPLOG_DEBUG_STR(logger, "Have extracted a subarray of shape " << newArr.shape());
	    this->itsArray.assign(newArr);
	    this->itsLocation += blc;
	    this->itsShape = trc-blc+1;
	    ASKAPLOG_DEBUG_STR(logger, "Now have location="<<this->itsLocation << " and shape="<<this->itsShape);
	}


	void CurvatureMapCreator::write()
	{
	    if(this->itsFilename!=""){
		ASKAPLOG_DEBUG_STR(logger, "In CurvatureMapCreator::write()");

		DistributedImageWriter writer(*this->itsComms, this->itsCube, this->itsFilename);
		ASKAPLOG_DEBUG_STR(logger, "Creating the output image " << this->itsFilename);
		writer.create();
		ASKAPLOG_DEBUG_STR(logger, "Writing curvature map of shape " << this->itsArray.shape() << " to " << this->itsFilename);
		writer.write(this->itsArray,this->itsLocation,true);
		ASKAPLOG_DEBUG_STR(logger, "Curvature image written");
	    }
	    
	}


    }

}
