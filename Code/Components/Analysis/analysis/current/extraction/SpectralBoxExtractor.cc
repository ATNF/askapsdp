/// @file
///
/// Class to handle extraction of a summed spectrum corresponding to a source.
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
/// @author Matthew Whiting <Matthew.Whiting@csiro.au>
///
#include <extraction/SpectralBoxExtractor.h>
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

ASKAP_LOGGER(logger, ".spectralboxextractor");

namespace askap {

namespace analysis {

SpectralBoxExtractor::SpectralBoxExtractor(const LOFAR::ParameterSet& parset):
    SourceDataExtractor(parset)
{

    itsBoxWidth = parset.getInt16("spectralBoxWidth", defaultSpectralExtractionBoxWidth);

    itsOutputFilenameBase = parset.getString("spectralOutputBase", "");
    ASKAPCHECK(itsOutputFilenameBase != "", "Extraction: " <<
               "No output base name has been provided for the spectral output. " <<
               "Use spectralOutputBase.");

}

SpectralBoxExtractor::SpectralBoxExtractor(const SpectralBoxExtractor& other)
{
    this->operator=(other);
}

SpectralBoxExtractor& SpectralBoxExtractor::operator=(const SpectralBoxExtractor& other)
{
    if (this == &other) return *this;
    ((SourceDataExtractor &) *this) = other;
    itsBoxWidth = other.itsBoxWidth;
    itsXloc = other.itsXloc;
    itsYloc = other.itsYloc;
    return *this;
}

void SpectralBoxExtractor::initialiseArray()
{
    // Form itsArray and initialise to zero
    itsInputCube = itsInputCubeList.at(0);
    if (this->openInput()) {
        int specsize = itsInputCubePtr->shape()(itsSpcAxis);
        casa::IPosition shape(4, 1, 1, itsStokesList.size(), specsize);
        ASKAPLOG_DEBUG_STR(logger,
                           "Extraction: Initialising array to zero with shape " << shape);
        itsArray = casa::Array<Float>(shape, 0.0);
        this->closeInput();
    } else {
        ASKAPLOG_ERROR_STR(logger, "Could not open image");
    }
}

void SpectralBoxExtractor::defineSlicer()
{

    if (this->openInput()) {
        IPosition shape = itsInputCubePtr->shape();
        ASKAPCHECK(itsInputCoords.hasSpectralAxis(),
                   "Input cube \"" << itsInputCube << "\" has no spectral axis");
        ASKAPCHECK(itsInputCoords.hasDirectionCoordinate(),
                   "Input cube \"" << itsInputCube << "\" has no spatial axes");

        // define the slicer based on the source's peak pixel location and the box width.
        // Make sure we don't go over the edges of the image.
        int xmin, ymin, xmax, ymax;
        if (itsBoxWidth > 0) {
            int hw = (itsBoxWidth - 1) / 2;
            int xloc = int(itsXloc);
            int yloc = int(itsYloc);
            int zero = 0;
            xmin = std::max(zero, xloc - hw);
            xmax = std::min(int(shape(itsLngAxis) - 1), xloc + hw);
            ymin = std::max(zero, yloc - hw);
            ymax = std::min(int(shape(itsLatAxis) - 1), yloc + hw);
        } else {
            // use the detected pixels of the source for the spectral
            // extraction, and the x/y ranges for slicer
            xmin = itsSource->getXmin();
            xmax = itsSource->getXmax();
            ymin = itsSource->getYmin();
            ymax = itsSource->getYmax();
        }
        casa::IPosition blc(shape.size(), 0), trc(shape.size(), 0);
        blc(itsLngAxis) = xmin;
        blc(itsLatAxis) = ymin;
        blc(itsSpcAxis) = 0;
        trc(itsLngAxis) = xmax;
        trc(itsLatAxis) = ymax;
        trc(itsSpcAxis) = shape(itsSpcAxis) - 1;
        if (itsStkAxis > -1) {
            casa::Stokes stk;
            blc(itsStkAxis) = trc(itsStkAxis) =
                itsInputCoords.stokesPixelNumber(stk.name(itsCurrentStokes));
        }
        ASKAPLOG_DEBUG_STR(logger, "Defining slicer for " << itsInputCubePtr->name() <<
                           " based on blc=" << blc << ", trc=" << trc);
        itsSlicer = casa::Slicer(blc, trc, casa::Slicer::endIsLast);

        this->closeInput();
    } else {
        ASKAPLOG_ERROR_STR(logger, "Could not open image");
    }
}


void SpectralBoxExtractor::writeImage()
{
    ASKAPLOG_INFO_STR(logger, "Writing spectrum to " << itsOutputFilename);
    accessors::CasaImageAccess ia;

    itsInputCube = itsInputCubeList[0];
    if (this->openInput()) {
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
        if (stkCoNum >= 0){
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
        outshape(spcAxis) = itsSlicer.length()(itsSpcAxis);
        outshape(stkAxis) = stkvec.size();
        casa::Vector<Float> shift(outshape.size(), 0);
        casa::Vector<Float> incrFrac(outshape.size(), 1);
        shift(lngAxis) = itsXloc;
        shift(latAxis) = itsYloc;
        casa::Vector<Int> newshape = outshape.asVector();
        newcoo.subImageInSitu(shift, incrFrac, newshape);

        Array<Float> newarray(itsArray.reform(outshape));

        ia.create(itsOutputFilename, newarray.shape(), newcoo);

        /// @todo save the new units - if units were per beam, remove this factor

        // write the array
        ia.write(itsOutputFilename, newarray);
        ia.setUnits(itsOutputFilename, itsInputCubePtr->units().getName());

        this->closeInput();
    } else ASKAPLOG_ERROR_STR(logger, "Could not open image");
}


}

}
