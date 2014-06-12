/// @file ImageWriter.cc
///
/// @copyright (c) 2014 CSIRO
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

#include <askap_analysis.h>
#include <outputs/ImageWriter.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <string>
#include <duchamp/Cubes/cubes.hh>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/IPosition.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <images/Images/PagedImage.h>
#include <images/Images/SubImage.h>
#include <images/Images/ImageInfo.h>
#include <images/Images/ImageOpener.h>
#include <images/Images/FITSImage.h>
#include <images/Images/MIRIADImage.h>

#include <casainterface/CasaInterface.h>
#include <casa/aipstype.h>

///@brief Where the log messages go.
ASKAP_LOGGER(logger, ".imagewriter");

namespace askap {
    
    namespace analysis {

	ImageWriter::ImageWriter(const ImageWriter &other)
	{
	    this->operator=(other);
	}

	ImageWriter& ImageWriter::operator=(const ImageWriter &other)
	{
	    if(this==&other) return *this;
	    this->itsImageName = other.itsImageName;
	    this->itsBunit = other.itsBunit;
	    this->itsShape = other.itsShape;
	    this->itsTileshape = other.itsTileshape;
	    this->itsCoordSys = other.itsCoordSys;
	    this->itsImageInfo = other.itsImageInfo;
	    return *this;
	}

	ImageWriter::ImageWriter(duchamp::Cube *cube, std::string imageName)
	{
	    this->copyMetadata(cube);
	    this->itsImageName = imageName;
	}

	void ImageWriter::copyMetadata(duchamp::Cube *cube)
	{
	    ImageOpener::registerOpenImageFunction(ImageOpener::FITS, FITSImage::openFITSImage);
	    ImageOpener::registerOpenImageFunction(ImageOpener::MIRIAD, MIRIADImage::openMIRIADImage);
	    const LatticeBase* lattPtr = ImageOpener::openImage(cube->pars().getImageFile());
	    if (lattPtr == 0)
		ASKAPTHROW(AskapError, "Requested image \"" << cube->pars().getImageFile() << "\" does not exist or could not be opened.");
	    const ImageInterface<Float>* imagePtr = dynamic_cast<const ImageInterface<Float>*>(lattPtr);

	    this->itsCoordSys = imagePtr->coordinates();
	    this->itsShape = imagePtr->shape();
	    this->itsBunit = imagePtr->units();
	    this->itsImageInfo = imagePtr->imageInfo();

	    // set the default tileshape based on the overall image shape.
	    // this can be changed later if preferred (for smaller subsection writing).
	    this->setTileshapeFromShape(this->itsShape);

	}
	    
	void ImageWriter::setTileshapeFromShape(casa::IPosition &shape)
	{

	    int specAxis=this->itsCoordSys.spectralAxisNumber();
//	    int stkAxis=this->itsCoordSys.polarizationAxisNumber();
	    int lngAxis=this->itsCoordSys.directionAxesNumbers()[0];
	    int latAxis=this->itsCoordSys.directionAxesNumbers()[1];
	    this->itsTileshape = casa::IPosition(shape.size(),1);
	    this->itsTileshape(lngAxis) = std::min(128L,shape(lngAxis));
	    this->itsTileshape(latAxis) = std::min(128L,shape(latAxis));
	    if(this->itsCoordSys.hasSpectralAxis()) this->itsTileshape(specAxis) = std::min(16L,shape(specAxis));

	}


	void ImageWriter::create()
	{
	    if(this->itsImageName != ""){
		ASKAPLOG_DEBUG_STR(logger, "Creating image named " << this->itsImageName << " with shape " << this->itsShape << " and tileshape " << this->itsTileshape);
		casa::PagedImage<float> img(casa::TiledShape(this->itsShape,this->itsTileshape), this->itsCoordSys, this->itsImageName);
		img.setUnits(this->itsBunit);
		img.setImageInfo(this->itsImageInfo);
	    }
	}


	void ImageWriter::write(float *data, const casa::IPosition &shape, bool accumulate)
	{
	    ASKAPASSERT(shape.size() == this->itsShape.size());
	    casa::Array<Float> arr(shape,data,casa::SHARE);
	    casa::IPosition location(this->itsShape.size(),0);
	    this->write(arr,location);
	}

	void ImageWriter::write(float *data, const casa::IPosition &shape, const casa::IPosition &loc, bool accumulate)
	{
	    ASKAPASSERT(shape.size() == this->itsShape.size());
	    ASKAPASSERT(loc.size() == this->itsShape.size());
	    casa::Array<Float> arr(shape,data,casa::SHARE);
	    this->write(arr,loc);
	}

        void ImageWriter::write(const casa::Array<Float> &data, bool accumulate)
	{
	    ASKAPASSERT(data.ndim() == this->itsShape.size());
	    casa::IPosition location(this->itsShape.size(),0);
	    this->write(data,location);
	}

        void ImageWriter::write(const casa::Array<Float> &data, const casa::IPosition &loc, bool accumulate)
	{
	    ASKAPASSERT(data.ndim() == this->itsShape.size());
	    ASKAPASSERT(loc.size() == this->itsShape.size());
	    ASKAPLOG_DEBUG_STR(logger, "Opening image " << this->itsImageName << " for writing");
	    casa::PagedImage<float> img(this->itsImageName);
	    ASKAPLOG_DEBUG_STR(logger, "Writing array of shape " << data.shape() << " to image " << this->itsImageName << " at location " << loc);
	    if(accumulate){
	      casa::Array<casa::Float> newdata = data + this->read(loc,data.shape());
	      img.putSlice(newdata,loc);
	    }
	    else img.putSlice(data, loc);

	}

	casa::Array<casa::Float> ImageWriter::read(const casa::IPosition& loc, const casa::IPosition &shape)
	{
	    ASKAPASSERT(loc.size() == shape.size());
	    casa::PagedImage<float> img(this->itsImageName);
	    return img.getSlice(loc,shape);
	}



    }

}
