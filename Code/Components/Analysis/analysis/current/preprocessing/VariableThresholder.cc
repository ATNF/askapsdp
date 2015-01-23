/// @file VariableThresholder.cc
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
#include <preprocessing/VariableThresholder.h>
#include <preprocessing/VariableThresholdingHelpers.h>
#include <outputs/ImageWriter.h>
#include <outputs/DistributedImageWriter.h>
#include <analysisparallel/SubimageDef.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <Common/ParameterSet.h>
#include <Blob/BlobString.h>
#include <Blob/BlobIBufString.h>
#include <Blob/BlobOBufString.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>
#include <duchamp/Cubes/cubes.hh>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Slicer.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <images/Images/ImageOpener.h>
#include <images/Images/FITSImage.h>
#include <images/Images/MIRIADImage.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/PagedImage.h>
#include <images/Images/SubImage.h>

#include <casainterface/CasaInterface.h>

#include <string>

///@brief Where the log messages go.
ASKAP_LOGGER(logger, ".varthresh");

namespace askap {

namespace analysis {

VariableThresholder::VariableThresholder(askap::askapparallel::AskapParallel& comms,
        const LOFAR::ParameterSet &parset):
    itsComms(&comms),
    itsParset(parset)
{
    itsBoxSize = parset.getInt16("boxSize", 50);
    itsSNRimageName = parset.getString("SNRimageName", "");
    itsThresholdImageName = parset.getString("ThresholdImageName", "");
    itsNoiseImageName = parset.getString("NoiseImageName", "");
    itsAverageImageName = parset.getString("AverageImageName", "");
    itsBoxSumImageName = parset.getString("BoxSumImageName", "");
    itsFlagWriteImages = (itsSNRimageName != "" ||
                          itsThresholdImageName != "" ||
                          itsNoiseImageName != "" ||
                          itsAverageImageName != "" ||
                          itsBoxSumImageName != "");
    itsInputImage = "";
    itsSearchType = "spatial";
    itsCube = 0;
    itsFlagRobustStats = true;
    itsFlagReuse = parset.getBool("reuse", false);
    if (itsSNRimageName == "") {
        ASKAPLOG_WARN_STR(logger,
                          "Variable Thresholder: reuse=true, but no SNR image name given. " <<
                          "Turning reuse off.");
        itsFlagReuse = false;
    }
}

void VariableThresholder::setFilenames(askap::askapparallel::AskapParallel& comms)
{
    if (comms.isParallel()) {
        std::stringstream suffix;
        suffix << "_" << comms.rank() << "_" << comms.nProcs();
        if (itsSNRimageName != "") {
            itsSNRimageName += suffix.str();
        }
        if (itsNoiseImageName != "") {
            itsNoiseImageName += suffix.str();
        }
        if (itsBoxSumImageName != "") {
            itsBoxSumImageName += suffix.str();
        }
        if (itsAverageImageName != "") {
            itsAverageImageName += suffix.str();
        }
        if (itsThresholdImageName != "") {
            itsThresholdImageName += suffix.str();
        }
    }
}


void VariableThresholder::initialise(duchamp::Cube &cube,
                                     analysisutilities::SubimageDef &subdef)
{
    itsCube = &cube;
    itsSubimageDef = &subdef;
    itsInputImage = cube.pars().getImageFile();
    itsFlagRobustStats = cube.pars().getFlagRobustStats();
    itsSNRthreshold = cube.pars().getCut();
    itsSearchType = cube.pars().getSearchType();
    ASKAPCHECK((itsSearchType == "spectral") || (itsSearchType == "spatial"),
               "SearchType needs to be either 'spectral' or 'spatial' - you have " <<
               itsSearchType);

    itsSlicer = analysisutilities::subsectionToSlicer(cube.pars().section());
    analysisutilities::fixSlicer(itsSlicer, cube.header().getWCS());

    const boost::shared_ptr<SubImage<Float> > sub =
        analysisutilities::getSubImage(cube.pars().getImageFile(), itsSlicer);
    itsInputCoordSys = sub->coordinates();
    itsInputShape = sub->shape();

    ASKAPLOG_DEBUG_STR(logger , "About to get the section for rank " << itsComms->rank());
    duchamp::Section sec = itsSubimageDef->section(itsComms->rank() - 1);
    ASKAPLOG_DEBUG_STR(logger, "It is " << sec.getSection());
    sec.parse(itsInputShape.asStdVector());
    itsLocation = casa::IPosition(sec.getStartList());
    ASKAPLOG_DEBUG_STR(logger,
                       "Reference location for rank " << itsComms->rank() <<
                       " is " << itsLocation <<
                       " since local subsection = " << sec.getSection() <<
                       " and input shape = " << itsInputShape);


}


void VariableThresholder::calculate()
{

    if (itsFlagReuse) {

        ASKAPLOG_INFO_STR(logger, "Reusing SNR map from file " << itsSNRimageName);

        casa::Array<Float> snr = analysisutilities::getPixelsInBox(itsSNRimageName,
                                 itsSlicer);

        if (itsCube->getRecon() == 0) {
            ASKAPLOG_ERROR_STR(logger,
                               "The Cube's recon array not defined - cannot save SNR map");
        } else {
            for (size_t i = 0; i < itsCube->getSize(); i++) {
                itsCube->getRecon()[i] = snr.data()[i];
            }
        }


    } else {

        ASKAPLOG_INFO_STR(logger, "Will calculate the pixel-by-pixel signal-to-noise map");
        if (itsSNRimageName != "") {
            ASKAPLOG_INFO_STR(logger, "Will write the SNR map to " <<
                              itsSNRimageName);
        }
        if (itsBoxSumImageName != "") {
            ASKAPLOG_INFO_STR(logger, "Will write the box sum map to " <<
                              itsBoxSumImageName);
        }
        if (itsNoiseImageName != "") {
            ASKAPLOG_INFO_STR(logger, "Will write the noise map to " <<
                              itsNoiseImageName);
        }
        if (itsAverageImageName != "") {
            ASKAPLOG_INFO_STR(logger, "Will write the average background map to " <<
                              itsAverageImageName);
        }
        if (itsThresholdImageName != "") {
            ASKAPLOG_INFO_STR(logger, "Will write the flux threshold map to " <<
                              itsThresholdImageName);
        }

        int specAxis = itsInputCoordSys.spectralAxisNumber();
        int lngAxis = itsInputCoordSys.directionAxesNumbers()[0];
        int latAxis = itsInputCoordSys.directionAxesNumbers()[1];
        size_t spatsize = itsInputShape(lngAxis) * itsInputShape(latAxis);
        size_t specsize = (specAxis >= 0) ? itsInputShape(specAxis) : 1;

        casa::IPosition chunkshape = itsInputShape;
        casa::IPosition box;
        size_t maxCtr;
        if (itsSearchType == "spatial") {
            if (specAxis >= 0) chunkshape(specAxis) = 1;
            box = casa::IPosition(2, itsBoxSize, itsBoxSize);
            maxCtr = specsize;
        } else {
            if (lngAxis >= 0) chunkshape(lngAxis) = 1;
            if (latAxis >= 0) chunkshape(latAxis) = 1;
            box = casa::IPosition(1, itsBoxSize);
            maxCtr = spatsize;
        }

        ASKAPLOG_INFO_STR(logger,
                          "Will calculate box-wise signal-to-noise " <<
                          "in image of shape " << itsInputShape <<
                          " using  '" << itsSearchType <<
                          "' mode with chunks of shape " << chunkshape <<
                          " and a box of shape " << box);

        for (size_t ctr = 0; ctr < maxCtr; ctr++) {
            if (maxCtr > 1) {
                ASKAPLOG_DEBUG_STR(logger, "Variable Thresholder calculation: Iteration " <<
                                   ctr << " of " << maxCtr);
            }
            bool isStart = (ctr == 0);
            casa::Array<Float> inputChunk(chunkshape, 0.);
            casa::MaskedArray<Float>
            inputMaskedChunk(inputChunk, casa::LogicalArray(chunkshape, true));
            casa::Array<Float> middle(chunkshape, 0.);
            casa::Array<Float> spread(chunkshape, 0.);
            casa::Array<Float> snr(chunkshape, 0.);
            casa::Array<Float> boxsum(chunkshape, 0.);

            casa::IPosition loc(itsLocation.size(), 0);
            if (itsSearchType == "spatial") {
                if (specAxis >= 0) {
                    loc(specAxis) = ctr;
                }
            } else {
                if (lngAxis >= 0) {
                    loc(lngAxis) = ctr % itsCube->getDimX();
                }
                if (latAxis >= 0) {
                    loc(latAxis) = ctr / itsCube->getDimX();
                }
            }
            // loc = loc + itsSlicer.start();
            loc = loc + itsLocation;

            if (itsComms->isWorker()) {
                this->defineChunk(inputChunk, inputMaskedChunk, ctr);
                //slidingBoxStats(inputChunk, middle, spread, box, itsFlagRobustStats);
                slidingBoxMaskedStats(inputMaskedChunk, middle, spread, box,
                                      itsFlagRobustStats);
                // snr = calcSNR(inputChunk,middle,spread);
                snr = calcMaskedSNR(inputMaskedChunk, middle, spread);
                if (itsBoxSumImageName != "") {
                    // boxsum = slidingArrayMath(inputChunk, box, SumFunc<Float>());
                    boxsum = slidingArrayMath(inputMaskedChunk, box, MaskedSumFunc<Float>());
                }
            }

            if (itsFlagWriteImages) {
                this->writeImages(middle, spread, snr, boxsum, loc, isStart);
            }

            if (itsComms->isWorker()) {
                ASKAPLOG_DEBUG_STR(logger,
                                   "About to store the SNR map to the cube for iteration " <<
                                   ctr << " of " << maxCtr);
                this->saveSNRtoCube(snr, ctr);
            }
        }

    }

    itsCube->setReconFlag(true);

}

void VariableThresholder::defineChunk(casa::Array<Float> &inputChunkArr,
                                      casa::MaskedArray<Float> &outputChunk, size_t ctr)
{
    casa::Array<Float>::iterator iter(inputChunkArr.begin());
    int lngAxis = itsInputCoordSys.directionAxesNumbers()[0];
    int latAxis = itsInputCoordSys.directionAxesNumbers()[1];
    size_t spatsize = itsInputShape(lngAxis) * itsInputShape(latAxis);
    casa::LogicalArray theMask(inputChunkArr.shape(), true);
    casa::LogicalArray::iterator itermask = theMask.begin();
    if (itsSearchType == "spatial") {
        for (size_t i = 0; iter != inputChunkArr.end(); iter++, i++, itermask++) {
            size_t pos = i + ctr * spatsize;
            *iter = itsCube->getArray()[pos];
            *itermask = (!itsCube->isBlank(pos) && itsWeighter->isValid(pos));
        }
    } else {
        for (size_t z = 0; iter != inputChunkArr.end(); iter++, z++, itermask++) {
            size_t pos = ctr + z * spatsize;
            *iter = itsCube->getArray()[pos];
            *itermask = (!itsCube->isBlank(pos) && itsWeighter->isValid(pos));
        }
    }
    outputChunk.setData(inputChunkArr, theMask);
}

void VariableThresholder::saveSNRtoCube(casa::Array<Float> &snr, size_t ctr)
{
    if (itsCube->getRecon() == 0) {
        ASKAPLOG_ERROR_STR(logger, "The Cube's recon array not defined - cannot save SNR map");
    } else {
        casa::Array<Float>::iterator iter(snr.begin());
        int lngAxis = itsInputCoordSys.directionAxesNumbers()[0];
        int latAxis = itsInputCoordSys.directionAxesNumbers()[1];
        size_t spatsize = itsInputShape(lngAxis) * itsInputShape(latAxis);
        if (itsSearchType == "spatial") {
            for (size_t i = 0; iter != snr.end(); iter++, i++) {
                itsCube->getRecon()[i + ctr * spatsize] = *iter;
            }
        } else {
            for (size_t z = 0; iter != snr.end(); iter++, z++) {
                itsCube->getRecon()[ctr + z * spatsize] = *iter;
            }
        }
    }

}

void VariableThresholder::writeImages(casa::Array<Float> &middle,
                                      casa::Array<Float> &spread,
                                      casa::Array<Float> &snr,
                                      casa::Array<Float> &boxsum,
                                      casa::IPosition &loc,
                                      bool doCreate)
{

    bool addToImage = true;

    if (itsNoiseImageName != "") {
        DistributedImageWriter noiseWriter(*itsComms, itsCube, itsNoiseImageName);
        noiseWriter.create();
        noiseWriter.write(spread, loc, addToImage);
    }

    if (itsAverageImageName != "") {
        DistributedImageWriter averageWriter(*itsComms, itsCube, itsAverageImageName);
        averageWriter.create();
        averageWriter.write(middle, loc, addToImage);
    }

    if (itsThresholdImageName != "") {
        DistributedImageWriter threshWriter(*itsComms, itsCube, itsThresholdImageName);
        threshWriter.create();
        casa::Array<Float> thresh = middle + itsSNRthreshold * spread;
        threshWriter.write(thresh, loc, addToImage);
    }

    if (itsSNRimageName != "") {
        DistributedImageWriter snrWriter(*itsComms, itsCube, itsSNRimageName);
        snrWriter.create();
        snrWriter.write(snr, loc, addToImage);
    }

    if (itsBoxSumImageName != "") {
        DistributedImageWriter boxWriter(*itsComms, itsCube, itsBoxSumImageName);
        boxWriter.create();
        boxWriter.write(boxsum, loc, addToImage);
    }


}


void VariableThresholder::search()
{

    if (itsCube->getRecon() == 0) {
        ASKAPLOG_ERROR_STR(logger,
                           "The Cube's recon array not defined - cannot search for sources.");
    } else {
        if (!itsCube->pars().getFlagUserThreshold()) {
            ASKAPLOG_DEBUG_STR(logger, "Setting user threshold to " <<
                               itsCube->pars().getCut() << " sigma");
            itsCube->pars().setThreshold(itsCube->pars().getCut());
            itsCube->pars().setFlagUserThreshold(true);
            if (itsCube->pars().getFlagGrowth()) {
                ASKAPLOG_DEBUG_STR(logger, "Setting user growth threshold to " <<
                                   itsCube->pars().getGrowthCut() << " sigma");
                itsCube->pars().setGrowthThreshold(itsCube->pars().getGrowthCut());
                itsCube->pars().setFlagUserGrowthThreshold(true);
            }
        }

        ASKAPLOG_DEBUG_STR(logger, "Searching SNR map");
        itsCube->ObjectList() =
            searchReconArray(itsCube->getDimArray(), itsCube->getArray(),
                             itsCube->getRecon(), itsCube->pars(),
                             itsCube->stats());
        ASKAPLOG_DEBUG_STR(logger, "Number of sources found = " << itsCube->getNumObj());
        itsCube->updateDetectMap();
        if (itsCube->pars().getFlagLog()) {
            itsCube->logDetectionList();
        }
    }
}


}

}
