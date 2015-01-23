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
#include <extraction/SourceSpectrumExtractor.h>
#include <askap_analysis.h>
#include <extraction/SourceDataExtractor.h>
#include <extraction/SpectralBoxExtractor.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <sourcefitting/RadioSource.h>

#include <imageaccess/CasaImageAccess.h>
#include <imageaccess/BeamLogger.h>

#include <duchamp/PixelMap/Object2D.hh>

#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/MaskedArray.h>
#include <casa/Arrays/Slicer.h>
#include <casa/Arrays/Vector.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/ImageOpener.h>
#include <images/Images/FITSImage.h>
#include <images/Images/MIRIADImage.h>
#include <images/Images/SubImage.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <measures/Measures/Stokes.h>

#include <Common/ParameterSet.h>

using namespace askap::analysis::sourcefitting;

ASKAP_LOGGER(logger, ".sourcespectrumextractor");

namespace askap {

namespace analysis {

SourceSpectrumExtractor::SourceSpectrumExtractor(const LOFAR::ParameterSet& parset):
    SpectralBoxExtractor(parset)
{

    itsFlagUseDetection = parset.getBool("useDetectedPixels", false);
    if (itsFlagUseDetection) {
        itsBoxWidth = -1;
        if (parset.isDefined("spectralBoxWidth")) {
            ASKAPLOG_WARN_STR(logger, "useDetectedPixels option selected, " <<
                              "so setting spectralBoxWidth=-1");
        }
    }

    itsFlagDoScale = parset.getBool("scaleSpectraByBeam", true);
    itsBeamLog = parset.getString("beamLog", "");

    this->initialiseArray();

}

SourceSpectrumExtractor::SourceSpectrumExtractor(const SourceSpectrumExtractor& other)
{
    this->operator=(other);
}

SourceSpectrumExtractor& SourceSpectrumExtractor::operator=(const SourceSpectrumExtractor& other)
{
    if (this == &other) return *this;
    ((SpectralBoxExtractor &) *this) = other;
    itsFlagDoScale = other.itsFlagDoScale;
    itsFlagUseDetection = other.itsFlagUseDetection;
    itsBeamScaleFactor = other.itsBeamScaleFactor;
    itsBeamLog = other.itsBeamLog;
    return *this;
}


void SourceSpectrumExtractor::setBeamScale()
{

    itsBeamScaleFactor = std::vector<float>();

    if (itsFlagDoScale) {

        if (this->openInput()) {

            std::vector< casa::Vector<Quantum<Double> > > beamvec;

            casa::Vector<Quantum<Double> >
            inputBeam = itsInputCubePtr->imageInfo().restoringBeam();

            ASKAPLOG_DEBUG_STR(logger, "Setting beam scaling factor. BeamLog=" <<
                               itsBeamLog << ", image beam = " << inputBeam);

            if (itsBeamLog == "") {
                if (inputBeam.size() == 0) {
                    ASKAPLOG_WARN_STR(logger, "Input image \"" << itsInputCube <<
                                      "\" has no beam information. Not scaling spectra by beam");
                    itsBeamScaleFactor.push_back(1.);
                } else {
                    beamvec.push_back(inputBeam);
                    ASKAPLOG_DEBUG_STR(logger, "Beam for input cube = " << inputBeam);
                }
            } else {
                accessors::BeamLogger beamlog(itsBeamLog);
                beamlog.read();
                beamvec = beamlog.beamlist();

                if (int(beamvec.size()) != itsInputCubePtr->shape()(itsSpcAxis)) {
                    ASKAPLOG_ERROR_STR(logger, "Beam log " << itsBeamLog <<
                                       " has " << beamvec.size() <<
                                       " entries - was expecting " <<
                                       itsInputCubePtr->shape()(itsSpcAxis));
                    beamvec = std::vector< Vector<Quantum<Double> > >(1, inputBeam);
                }
            }

            if (beamvec.size() > 0) {

                for (size_t i = 0; i < beamvec.size(); i++) {

                    int dirCoNum = itsInputCoords.findCoordinate(casa::Coordinate::DIRECTION);
                    casa::DirectionCoordinate
                    dirCoo = itsInputCoords.directionCoordinate(dirCoNum);
                    double fwhmMajPix = beamvec[i][0].getValue(dirCoo.worldAxisUnits()[0]) /
                                        fabs(dirCoo.increment()[0]);
                    double fwhmMinPix = beamvec[i][1].getValue(dirCoo.worldAxisUnits()[1]) /
                                        fabs(dirCoo.increment()[1]);

                    if (itsFlagUseDetection) {
                        double bpaDeg = beamvec[i][2].getValue("deg");
                        duchamp::DuchampBeam beam(fwhmMajPix, fwhmMinPix, bpaDeg);
                        itsBeamScaleFactor.push_back(beam.area());
                        if (itsBeamLog == "") {
                            ASKAPLOG_DEBUG_STR(logger, "Beam scale factor = " <<
                                               itsBeamScaleFactor << " using beam of " <<
                                               fwhmMajPix << "x" << fwhmMinPix);
                        }
                    } else {

                        double costheta = cos(beamvec[i][2].getValue("rad"));
                        double sintheta = sin(beamvec[i][2].getValue("rad"));

                        double majSDsq = fwhmMajPix * fwhmMajPix / 8. / M_LN2;
                        double minSDsq = fwhmMinPix * fwhmMinPix / 8. / M_LN2;

                        int hw = (itsBoxWidth - 1) / 2;
                        double scaleFactor = 0.;
                        for (int y = -hw; y <= hw; y++) {
                            for (int x = -hw; x <= hw; x++) {
                                double u = x * costheta + y * sintheta;
                                double v = x * sintheta - y * costheta;
                                scaleFactor += exp(-0.5 * (u * u / majSDsq + v * v / minSDsq));
                            }
                        }
                        itsBeamScaleFactor.push_back(scaleFactor);

                        if (itsBeamLog == "") {
                            ASKAPLOG_DEBUG_STR(logger, "Beam scale factor = " <<
                                               itsBeamScaleFactor);
                        }

                    }
                }

            }

            ASKAPLOG_DEBUG_STR(logger,
                               "Defined the beam scale factor vector of size " <<
                               itsBeamScaleFactor.size());

            this->closeInput();
        } else {
            ASKAPLOG_ERROR_STR(logger, "Could not open image");
        }
    }

}

void SourceSpectrumExtractor::extract()
{

    this->setBeamScale();

    for (size_t stokes = 0; stokes < itsStokesList.size(); stokes++) {

        // get either the matching image for the current stokes value,
        // or the first&only in the input list
        itsInputCube = itsInputCubeList[stokes % itsInputCubeList.size()];
        itsCurrentStokes = itsStokesList[stokes];
        this->defineSlicer();
        if (this->openInput()) {
            casa::Stokes stk;
            ASKAPLOG_INFO_STR(logger, "Extracting spectrum from " << itsInputCube <<
                              " with shape " << itsInputCubePtr->shape() <<
                              " for source ID " << itsSource->getID() <<
                              " using slicer " << itsSlicer <<
                              " and Stokes " << stk.name(itsCurrentStokes));

            const boost::shared_ptr<SubImage<Float> >
            sub(new SubImage<Float>(*itsInputCubePtr, itsSlicer));

            ASKAPASSERT(sub->size() > 0);
            const casa::MaskedArray<Float> msub(sub->get(), sub->getMask());
            casa::Array<Float> subarray(sub->shape());
            subarray = msub;

            casa::IPosition outBLC(4, 0), outTRC(itsArray.shape() - 1);
            outBLC(2) = outTRC(2) = stokes;

            if (!itsFlagUseDetection) {
                casa::Array<Float> sumarray = partialSums(subarray, IPosition(2, 0, 1));
                itsArray(outBLC, outTRC) = sumarray.reform(itsArray(outBLC, outTRC).shape());

            } else {
                ASKAPLOG_INFO_STR(logger,
                                  "Extracting integrated spectrum using " <<
                                  "all detected spatial pixels");
                IPosition shape = itsInputCubePtr->shape();

                PixelInfo::Object2D spatmap = itsSource->getSpatialMap();
                casa::IPosition blc(shape.size(), 0);
                casa::IPosition trc(shape.size(), 0);
                casa::IPosition inc(shape.size(), 1);

                trc(itsSpcAxis) = shape[itsSpcAxis] - 1;
                if (itsStkAxis > -1) {
                    casa::Stokes stk;
                    blc(itsStkAxis) = trc(itsStkAxis) =
                                          itsInputCoords.stokesPixelNumber(stk.name(itsCurrentStokes));
                }

                for (int x = itsSource->getXmin(); x <= itsSource->getXmax(); x++) {
                    for (int y = itsSource->getYmin(); y <= itsSource->getYmax(); y++) {
                        if (spatmap.isInObject(x, y)) {
                            blc(itsLngAxis) = trc(itsLngAxis) = x - itsSource->getXmin();
                            blc(itsLatAxis) = trc(itsLatAxis) = y - itsSource->getYmin();
                            casa::Array<Float> spec = subarray(blc, trc, inc);
                            spec = spec.reform(itsArray(outBLC, outTRC).shape());
                            itsArray(outBLC, outTRC) = itsArray(outBLC, outTRC) + spec;
                        }
                    }
                }
            }

            this->closeInput();
        } else {
            ASKAPLOG_ERROR_STR(logger, "Could not open image");
        }
    }


    if (itsFlagDoScale) {

        if (itsBeamScaleFactor.size() == 1) {
            itsArray /= itsBeamScaleFactor[0];
        } else {
            casa::IPosition start(itsArray.ndim(), 0);
            casa::IPosition end = itsArray.shape() - 1;
            start(itsLngAxis) = start(itsLatAxis) = 0;
            for (int z = 0; z < itsArray.shape()(itsSpcAxis); z++) {
                start(itsSpcAxis) = end(itsSpcAxis) = z;
                itsArray(start, end) = itsArray(start, end) / itsBeamScaleFactor[z];
            }
        }

    }

}


}
}
