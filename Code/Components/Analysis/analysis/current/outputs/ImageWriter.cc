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

	void ImageWriter::copyMetadata(duchamp::Cube *cube)
	{
	    const LatticeBase* lattPtr = ImageOpener::openImage(cube->pars().getImageFile());
	    if (lattPtr == 0)
		ASKAPTHROW(AskapError, "Requested image \"" << cube->pars().getImageFile() << "\" does not exist or could not be opened.");
	    const ImageInterface<Float>* imagePtr = dynamic_cast<const ImageInterface<Float>*>(lattPtr);
	    casa::Slicer slicer = analysisutilities::subsectionToSlicer(cube->pars().section());
	    analysisutilities::fixSlicer(slicer, cube->header().getWCS());

	    const SubImage<Float> *sub = new SubImage<Float>(*imagePtr, slicer);
	    this->itsCoordSys = sub->coordinates();
	    this->itsShape = sub->shape();
	    this->itsBunit = sub->units();
	    this->itsImageInfo = sub->imageInfo();

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


	void ImageWriter::create(std::string filename)
	{
	    this->itsImageName = filename;
	    casa::PagedImage<float> img(casa::TiledShape(this->itsShape,this->itsTileshape), this->itsCoordSys, this->itsImageName);
	    img.setUnits(this->itsBunit);
	    img.setImageInfo(this->itsImageInfo);
	    
	}


	void ImageWriter::write(float *data, casa::IPosition &shape)
	{
	    ASKAPASSERT(shape.size() == this->itsShape.size());
	    casa::Array<Float> arr(shape,data,casa::SHARE);
	    casa::IPosition location(this->itsShape.size(),0);
	    this->write(arr,location);
	}

	void ImageWriter::write(float *data, casa::IPosition &shape, casa::IPosition &loc)
	{
	    ASKAPASSERT(shape.size() == this->itsShape.size());
	    ASKAPASSERT(loc.size() == this->itsShape.size());
	    casa::Array<Float> arr(shape,data,casa::SHARE);
	    this->write(arr,loc);
	}

	void ImageWriter::write(casa::Array<Float> &data)
	{
	    ASKAPASSERT(data.ndim() == this->itsShape.size());
	    casa::IPosition location(this->itsShape.size(),0);
	    this->write(data,location);
	}

	void ImageWriter::write(casa::Array<Float> &data, casa::IPosition &loc)
	{
	    ASKAPASSERT(data.ndim() == this->itsShape.size());
	    ASKAPASSERT(loc.size() == this->itsShape.size());
	    casa::PagedImage<float> img(this->itsImageName);
	    img.putSlice(data, loc);

	}

    }

}
