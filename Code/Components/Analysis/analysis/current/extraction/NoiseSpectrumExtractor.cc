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
#include <extraction/NoiseSpectrumExtractor.h>
#include <askap_analysis.h>
#include <extraction/SourceDataExtractor.h>
#include <extraction/SpectralBoxExtractor.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <sourcefitting/RadioSource.h>

#include <imageaccess/CasaImageAccess.h>

#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Slicer.h>
#include <casa/Arrays/ArrayPartMath.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/ImageOpener.h>
#include <images/Images/FITSImage.h>
#include <images/Images/MIRIADImage.h>
#include <images/Images/SubImage.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <measures/Measures/Stokes.h>

#include <Common/ParameterSet.h>

#include <duchamp/Utils/Statistics.hh>

using namespace askap::analysis::sourcefitting;

ASKAP_LOGGER(logger, ".noiseSpectrumExtractor");

namespace askap {

namespace analysis {

NoiseSpectrumExtractor::NoiseSpectrumExtractor(const LOFAR::ParameterSet& parset):
    SpectralBoxExtractor(parset)
{

    itsAreaInBeams = parset.getFloat("noiseArea", 50);
    itsRobustFlag = parset.getBool("robust", true);

    casa::Stokes stk;
    itsCurrentStokes = itsStokesList[0];
    if (itsStokesList.size() > 1) {
        ASKAPLOG_WARN_STR(logger, "Noise Extractor: " <<
                          "Will only use the first provided Stokes parameter: " <<
                          stk.name(itsCurrentStokes));
        itsStokesList = casa::Vector<casa::Stokes::StokesTypes>(1, itsCurrentStokes);
    }
    itsInputCube = itsInputCubeList[0];
    if (itsInputCubeList.size() > 1) {
        ASKAPLOG_WARN_STR(logger, "Noise Extractor: " <<
                          "Will only use the first provided input cube: " <<
                          itsInputCubeList[0]);
        itsInputCubeList = std::vector<std::string>(1, itsInputCube);
    }

    this->initialiseArray();

    this->setBoxWidth();

}

NoiseSpectrumExtractor::NoiseSpectrumExtractor(const NoiseSpectrumExtractor& other)
{
    this->operator=(other);
}

NoiseSpectrumExtractor& NoiseSpectrumExtractor::operator=(const NoiseSpectrumExtractor& other)
{
    if (this == &other) return *this;
    ((SpectralBoxExtractor &) *this) = other;
    itsAreaInBeams = other.itsAreaInBeams;
    itsRobustFlag = other.itsRobustFlag;
    return *this;
}

void NoiseSpectrumExtractor::setBoxWidth()
{

    if (this->openInput()) {
        Vector<Quantum<Double> >
        inputBeam = itsInputCubePtr->imageInfo().restoringBeam();
        ASKAPLOG_DEBUG_STR(logger, "Beam for input cube = " << inputBeam);
        if (inputBeam.size() == 0) {
            ASKAPLOG_WARN_STR(logger, "Input image \"" << itsInputCube <<
                              "\" has no beam information. " <<
                              "Using box width value from parset of " <<
                              itsBoxWidth << "pix");
        } else {

            int dirCoNum = itsInputCoords.findCoordinate(casa::Coordinate::DIRECTION);
            casa::DirectionCoordinate dirCoo = itsInputCoords.directionCoordinate(dirCoNum);
            double fwhmMajPix = inputBeam[0].getValue(dirCoo.worldAxisUnits()[0]) /
                                fabs(dirCoo.increment()[0]);
            double fwhmMinPix = inputBeam[1].getValue(dirCoo.worldAxisUnits()[1]) /
                                fabs(dirCoo.increment()[1]);
            double beamAreaInPix = M_PI * fwhmMajPix * fwhmMinPix;

            itsBoxWidth = int(ceil(sqrt(itsAreaInBeams * beamAreaInPix)));

            ASKAPLOG_INFO_STR(logger, "Noise Extractor: Using box of area " <<
                              itsAreaInBeams << " beams (of area " <<
                              beamAreaInPix << " pix), or a square of " <<
                              itsBoxWidth << " pix on the side");

        }

        this->closeInput();
    } else ASKAPLOG_ERROR_STR(logger, "Could not open image");
}


void NoiseSpectrumExtractor::extract()
{

    this->defineSlicer();
    if (this->openInput()) {

        ASKAPLOG_INFO_STR(logger, "Extracting noise spectrum from " << itsInputCube <<
                          " surrounding source ID " << itsSource->getID());

        ASKAPLOG_DEBUG_STR(logger, "Constructing subimage from slicer " << itsSlicer);
        const boost::shared_ptr<SubImage<Float> >
            sub(new SubImage<Float>(*itsInputCubePtr, itsSlicer));
        casa::Array<Float> subarray = sub->get();
        
        casa::IPosition outBLC(subarray.ndim(), 0);
        casa::IPosition outTRC(itsArray.shape() - 1);

        casa::Array<Float> noisearray;
        if (itsRobustFlag) {
            noisearray = partialMadfms(subarray, IPosition(2, 0, 1)).
                reform(itsArray(outBLC, outTRC).shape()) /
                Statistics::correctionFactor;
        } else {
            noisearray = partialRmss(subarray, IPosition(2, 0, 1)).
                reform(itsArray(outBLC, outTRC).shape());
        }

        itsArray(outBLC, outTRC) = noisearray;

        this->closeInput();
    } else {
        ASKAPLOG_ERROR_STR(logger, "Could not open image");
    }
}


}

}
