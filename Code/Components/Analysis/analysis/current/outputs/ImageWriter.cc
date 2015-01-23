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

ImageWriter::ImageWriter(duchamp::Cube *cube, std::string imageName)
{
    this->copyMetadata(cube);
    itsImageName = imageName;
}

void ImageWriter::copyMetadata(duchamp::Cube *cube)
{

    const boost::shared_ptr<ImageInterface<Float> > imagePtr =
        analysisutilities::openImage(cube->pars().getImageFile());

    itsCoordSys = imagePtr->coordinates();
    itsShape = imagePtr->shape();
    itsBunit = imagePtr->units();
    itsImageInfo = imagePtr->imageInfo();

    // set the default tileshape based on the overall image shape.
    // this can be changed later if preferred (for smaller subsection writing).
    this->setTileshapeFromShape(itsShape);

}

void ImageWriter::setTileshapeFromShape(casa::IPosition &shape)
{

    int specAxis = itsCoordSys.spectralAxisNumber();
    int lngAxis = itsCoordSys.directionAxesNumbers()[0];
    int latAxis = itsCoordSys.directionAxesNumbers()[1];
    itsTileshape = casa::IPosition(shape.size(), 1);
    itsTileshape(lngAxis) = std::min(128L, shape(lngAxis));
    itsTileshape(latAxis) = std::min(128L, shape(latAxis));
    if (itsCoordSys.hasSpectralAxis()) {
        itsTileshape(specAxis) = std::min(16L, shape(specAxis));
    }
}


void ImageWriter::create()
{
    if (itsImageName != "") {
        ASKAPLOG_DEBUG_STR(logger,
                           "Creating image named " << itsImageName <<
                           " with shape " << itsShape <<
                           " and tileshape " << itsTileshape);

        casa::PagedImage<float> img(casa::TiledShape(itsShape, itsTileshape),
                                    itsCoordSys, itsImageName);
        img.setUnits(itsBunit);
        img.setImageInfo(itsImageInfo);
    }
}


void ImageWriter::write(float *data, const casa::IPosition &shape, bool accumulate)
{
    ASKAPASSERT(shape.size() == itsShape.size());
    casa::Array<Float> arr(shape, data, casa::SHARE);
    casa::IPosition location(itsShape.size(), 0);
    this->write(arr, location);
}

void ImageWriter::write(float *data, const casa::IPosition &shape,
                        const casa::IPosition &loc, bool accumulate)
{
    ASKAPASSERT(shape.size() == itsShape.size());
    ASKAPASSERT(loc.size() == itsShape.size());
    casa::Array<Float> arr(shape, data, casa::SHARE);
    this->write(arr, loc);
}

void ImageWriter::write(const casa::Array<Float> &data, bool accumulate)
{
    ASKAPASSERT(data.ndim() == itsShape.size());
    casa::IPosition location(itsShape.size(), 0);
    this->write(data, location);
}

void ImageWriter::write(const casa::Array<Float> &data,
                        const casa::IPosition &loc, bool accumulate)
{
    ASKAPASSERT(data.ndim() == itsShape.size());
    ASKAPASSERT(loc.size() == itsShape.size());
    ASKAPLOG_DEBUG_STR(logger, "Opening image " << itsImageName << " for writing");
    casa::PagedImage<float> img(itsImageName);
    ASKAPLOG_DEBUG_STR(logger,
                       "Writing array of shape " << data.shape() <<
                       " to image " << itsImageName <<
                       " at location " << loc);
    if (accumulate) {
        casa::Array<casa::Float> newdata = data + this->read(loc, data.shape());
        img.putSlice(newdata, loc);
    } else {
        img.putSlice(data, loc);
    }

}

casa::Array<casa::Float>
ImageWriter::read(const casa::IPosition& loc, const casa::IPosition &shape)
{
    ASKAPASSERT(loc.size() == shape.size());
    casa::PagedImage<float> img(itsImageName);
    return img.getSlice(loc, shape);
}



}

}
