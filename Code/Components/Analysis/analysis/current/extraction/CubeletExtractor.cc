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
    std::vector<unsigned int>
    padsizes = parset.getUintVector("padSize", std::vector<unsigned int>(2, 5));

    if (padsizes.size() > 2) {
        ASKAPLOG_WARN_STR(logger,
                          "Only using the first two elements of the padSize vector");
    }

    itsSpatialPad = padsizes[0];
    if (padsizes.size() > 1) {
        itsSpectralPad = padsizes[1];
    } else {
        itsSpectralPad = padsizes[0];
    }

    itsOutputFilenameBase = parset.getString("cubeletOutputBase", "");

}

CubeletExtractor::CubeletExtractor(const CubeletExtractor& other)
{
    this->operator=(other);
}

CubeletExtractor& CubeletExtractor::operator= (const CubeletExtractor& other)
{
    if (this == &other) return *this;
    ((SourceDataExtractor &) *this) = other;
    itsSpatialPad = other.itsSpatialPad;
    itsSpectralPad = other.itsSpectralPad;
    return *this;
}

void CubeletExtractor::defineSlicer()
{

    if (this->openInput()) {
        IPosition shape = itsInputCubePtr->shape();
        casa::IPosition blc(shape.size(), 0);
        casa::IPosition trc = shape - 1;

        long zero = 0;
        blc(itsLngAxis) = std::max(zero, itsSource->getXmin() - itsSpatialPad);
        blc(itsLatAxis) = std::max(zero, itsSource->getYmin() - itsSpatialPad);
        blc(itsSpcAxis) = std::max(zero, itsSource->getZmin() - itsSpectralPad);

        trc(itsLngAxis) = std::min(shape(itsLngAxis) - 1,
                                   itsSource->getXmax() + itsSpatialPad);
        trc(itsLatAxis) = std::min(shape(itsLatAxis) - 1,
                                   itsSource->getYmax() + itsSpatialPad);
        trc(itsSpcAxis) = std::min(shape(itsSpcAxis) - 1,
                                   itsSource->getZmax() + itsSpectralPad);
        /// @todo Not yet dealing with Stokes axis properly.

        itsSlicer = casa::Slicer(blc, trc, casa::Slicer::endIsLast);
        this->closeInput();
        this->initialiseArray();
    } else {
        ASKAPLOG_ERROR_STR(logger, "Could not open image");
    }
}

void CubeletExtractor::initialiseArray()
{
    if (this->openInput()) {
        int lngsize = itsSlicer.length()(itsLngAxis);
        int latsize = itsSlicer.length()(itsLatAxis);
        int spcsize = itsSlicer.length()(itsSpcAxis);
        casa::IPosition shape(itsInputCubePtr->shape().size(), 1);
        shape(itsLngAxis) = lngsize;
        shape(itsLatAxis) = latsize;
        shape(itsSpcAxis) = spcsize;
        ASKAPLOG_DEBUG_STR(logger,
                           "Cubelet extraction: Initialising array to zero with shape " <<
                           shape);
        itsArray = casa::Array<Float>(shape, 0.0);
        this->closeInput();
    } else {
        ASKAPLOG_ERROR_STR(logger, "Could not open image");
    }
}

void CubeletExtractor::extract()
{
    this->defineSlicer();
    if (this->openInput()) {

        ASKAPLOG_INFO_STR(logger,
                          "Extracting noise spectrum from " << itsInputCube <<
                          " surrounding source ID " << itsSource->getID());

        const boost::shared_ptr<SubImage<Float> >
        sub(new SubImage<Float>(*itsInputCubePtr, itsSlicer));

        ASKAPASSERT(sub->size() > 0);
        const casa::MaskedArray<Float> msub(sub->get(), sub->getMask());
        ASKAPASSERT(itsArray.size() == msub.size());
        itsArray = msub;

        this->closeInput();
    } else {
        ASKAPLOG_ERROR_STR(logger, "Could not open image");
    }
}

void CubeletExtractor::writeImage()
{
    ASKAPLOG_INFO_STR(logger, "Writing cube cutout to " << itsOutputFilename);
    accessors::CasaImageAccess ia;

    itsInputCube = itsInputCubeList[0];
    if (this->openInput()) {
        IPosition inshape = itsInputCubePtr->shape();
        casa::CoordinateSystem newcoo = casa::CoordinateUtil::defaultCoords4D();

        int dirCoNum = itsInputCoords.findCoordinate(casa::Coordinate::DIRECTION);
        int spcCoNum = itsInputCoords.findCoordinate(casa::Coordinate::SPECTRAL);
        int stkCoNum = itsInputCoords.findCoordinate(casa::Coordinate::STOKES);

        casa::DirectionCoordinate dircoo(itsInputCoords.directionCoordinate(dirCoNum));
        casa::SpectralCoordinate spcoo(itsInputCoords.spectralCoordinate(spcCoNum));
        casa::Vector<Int> stkvec(itsStokesList.size());
        for (size_t i = 0; i < stkvec.size(); i++) {
            stkvec[i] = itsStokesList[i];
        }
        casa::StokesCoordinate stkcoo(stkvec);
        newcoo.replaceCoordinate(dircoo, newcoo.findCoordinate(casa::Coordinate::DIRECTION));
        newcoo.replaceCoordinate(spcoo, newcoo.findCoordinate(casa::Coordinate::SPECTRAL));
        if(stkCoNum >= 0){
            newcoo.replaceCoordinate(stkcoo, newcoo.findCoordinate(casa::Coordinate::STOKES));
        }

        // shift the reference pixel for the spatial coords, so that
        // the RA/DEC (or whatever) are correct. Leave the
        // spectral/stokes axes untouched.
        int lngAxis = newcoo.directionAxesNumbers()[0];
        int latAxis = newcoo.directionAxesNumbers()[1];
        int spcAxis = newcoo.spectralAxisNumber();
        int stkAxis = newcoo.polarizationAxisNumber();
        casa::IPosition outshape(4, 1);
        outshape(lngAxis) = itsSlicer.length()(itsLngAxis);
        outshape(latAxis) = itsSlicer.length()(itsLatAxis);
        outshape(spcAxis) = itsSlicer.length()(itsSpcAxis);
        outshape(stkAxis) = stkvec.size();
        casa::Vector<Float> shift(outshape.size(), 0);
        casa::Vector<Float> incrFac(outshape.size(), 1);
        shift(lngAxis) = itsSource->getXmin() - itsSpatialPad;
        shift(latAxis) = itsSource->getYmin() - itsSpatialPad;
        shift(spcAxis) = itsSource->getZmin() - itsSpectralPad;
        casa::Vector<Int> newshape = outshape.asVector();

        newcoo.subImageInSitu(shift, incrFac, newshape);

        Array<Float> newarray(itsArray.reform(outshape));

        ia.create(itsOutputFilename, newarray.shape(), newcoo);

        // write the array
        ia.write(itsOutputFilename, newarray);

        // write the flux units
        std::string units = itsInputCubePtr->units().getName();
        ia.setUnits(itsOutputFilename, units);

        this->writeBeam(itsOutputFilename);

        casa::LogicalArray
        mask(itsInputCubePtr->pixelMask().getSlice(itsSlicer).reform(outshape));
        
        casa::PagedImage<float> img(itsOutputFilename);
        img.makeMask("mask");
        img.pixelMask().put(mask);

        this->closeInput();

    } else {
        ASKAPLOG_ERROR_STR(logger, "Could not open image");
    }
}

}

}
