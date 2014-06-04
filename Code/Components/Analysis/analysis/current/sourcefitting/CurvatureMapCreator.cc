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
#include <outputs/ImageWriter.h>
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

	void CurvatureMapCreator::calculate(duchamp::Cube &cube)
	{
	    this->itsCube = &cube;
	    ImageOpener::registerOpenImageFunction(ImageOpener::FITS, FITSImage::openFITSImage);
	    ImageOpener::registerOpenImageFunction(ImageOpener::MIRIAD, MIRIADImage::openMIRIADImage);
	    const LatticeBase* lattPtr = ImageOpener::openImage(this->itsCube->pars().getImageFile());
	    if (lattPtr == 0)
		ASKAPTHROW(AskapError, "Requested image \"" << this->itsCube->pars().getImageFile() << "\" does not exist or could not be opened.");
	    const ImageInterface<Float>* imagePtr = dynamic_cast<const ImageInterface<Float>*>(lattPtr);
	    casa::Slicer slicer = analysisutilities::subsectionToSlicer(this->itsCube->pars().section());
	    analysisutilities::fixSlicer(slicer, this->itsCube->header().getWCS());
	    const SubImage<Float> *sub = new SubImage<Float>(*imagePtr, slicer);
	    casa::IPosition inputShape = sub->shape();

	    //casa::IPosition inputShape(2,this->itsCube->getDimX(),this->itsCube->getDimY());
	    casa::Array<float> inputArray(inputShape,this->itsCube->getArray(),casa::SHARE);

	    casa::IPosition kernelShape(2,3,3);
	    casa::Array<float> kernel(kernelShape,1.);
	    kernel(casa::IPosition(2,1,1)) = -8.;

	    ASKAPLOG_DEBUG_STR(logger, "Defined a kernel for the curvature map calculations: " << kernel);

	    casa::Convolver<float> convolver(kernel,inputShape);
	    ASKAPLOG_DEBUG_STR(logger, "Defined a convolver");

	    this->itsArray = casa::Array<float>(inputShape);
	    ASKAPLOG_DEBUG_STR(logger, "About to convolve");
	    convolver.linearConv(this->itsArray, inputArray);
	    ASKAPLOG_DEBUG_STR(logger, "Convolving done.");

	    this->findSigma();
	}


	void CurvatureMapCreator::findSigma()
	{
	    
	    this->itsSigmaCurv = madfm(this->itsArray,False,False,False) / Statistics::correctionFactor;
	    ASKAPLOG_DEBUG_STR(logger, "Found sigma_curv = " << this->itsSigmaCurv);

	}


	void CurvatureMapCreator::write()
	{
	    if(this->itsFilename!=""){
		ImageWriter writer(this->itsCube, this->itsFilename);
		ASKAPLOG_DEBUG_STR(logger, "Creating the output image " << this->itsFilename);
		writer.create();
		ASKAPLOG_DEBUG_STR(logger, "Writing curvature map of shape " << this->itsArray.shape() << " to " << this->itsFilename);
		writer.write(this->itsArray);
		ASKAPLOG_DEBUG_STR(logger, "Curvature image written");
	    }
	    
	}


    }

}
