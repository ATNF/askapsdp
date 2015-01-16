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

VariableThresholder::VariableThresholder(askap::askapparallel::AskapParallel& comms, const LOFAR::ParameterSet &parset):
    itsComms(&comms), itsParset(parset)
{
    /// @details Initialise from a LOFAR parset. Define all
    /// parameters save for the input image, the search type
    /// and the robust stats flag - all of which are set
    /// according to the duchamp::Cube parameters. If an
    /// output image name is not provided, it will not be
    /// written.

    this->itsBoxSize = parset.getInt16("boxSize", 50);
    this->itsSNRimageName = parset.getString("SNRimageName", "");
    this->itsThresholdImageName = parset.getString("ThresholdImageName", "");
    this->itsNoiseImageName = parset.getString("NoiseImageName", "");
    this->itsAverageImageName = parset.getString("AverageImageName", "");
    this->itsBoxSumImageName = parset.getString("BoxSumImageName", "");
    this->doWriteImages = this->itsSNRimageName != "" || this->itsThresholdImageName != "" || this->itsNoiseImageName != "" || this->itsAverageImageName != "" || this->itsBoxSumImageName != "";
    this->itsInputImage = "";
    this->itsSearchType = "spatial";
    this->itsCube = 0;
    this->itsFlagRobustStats = true;
    this->itsFlagReuse = parset.getBool("reuse", false);
    if (this->itsSNRimageName == "") {
        ASKAPLOG_WARN_STR(logger, "Variable Thresholder: reuse=true, but no SNR image name given. Turning reuse off.");
        this->itsFlagReuse = false;
    }
}

void VariableThresholder::setFilenames(askap::askapparallel::AskapParallel& comms)
{
    /// @details Updates the output image names in the case of
    /// distributed processing. The names will have the worker
    /// number appended to them (so that instead of something
    /// like "image_snr" it will become "image_snr_6_9" for
    /// worker #6 out of 9.
    if (comms.isParallel()) {
        std::stringstream suffix;
        suffix << "_" << comms.rank() << "_" << comms.nProcs();
        if (this->itsSNRimageName != "") this->itsSNRimageName += suffix.str();
        if (this->itsNoiseImageName != "") this->itsNoiseImageName += suffix.str();
        if (this->itsBoxSumImageName != "") this->itsBoxSumImageName += suffix.str();
        if (this->itsAverageImageName != "") this->itsAverageImageName += suffix.str();
        if (this->itsThresholdImageName != "") this->itsThresholdImageName += suffix.str();
    }
}


VariableThresholder::VariableThresholder(const VariableThresholder& other)
{
    this->operator=(other);
}

VariableThresholder& VariableThresholder::operator= (const VariableThresholder& other)
{
    if (this == &other) return *this;
    this->itsParset = other.itsParset;
    this->itsComms = other.itsComms;
    this->itsFlagRobustStats = other.itsFlagRobustStats;
    this->itsSNRthreshold = other.itsSNRthreshold;
    this->itsSearchType = other.itsSearchType;
    this->itsBoxSize = other.itsBoxSize;
    this->itsInputImage = other.itsInputImage;
    this->itsSNRimageName = other.itsSNRimageName;
    this->itsThresholdImageName = other.itsThresholdImageName;
    this->itsNoiseImageName = other.itsNoiseImageName;
    this->itsAverageImageName = other.itsAverageImageName;
    this->itsBoxSumImageName = other.itsAverageImageName;
    this->itsSubimageDef = other.itsSubimageDef;
    this->itsCube = other.itsCube;
    this->itsInputShape = other.itsInputShape;
    this->itsInputCoordSys = other.itsInputCoordSys;
    return *this;
}


void VariableThresholder::initialise(duchamp::Cube &cube, analysisutilities::SubimageDef &subdef)
{
    /// @details Initialise the class with information from
    /// the duchamp::Cube. This is done to avoid replicating
    /// parameters and preserving the parameter
    /// hierarchy. Once the input image is known, the output
    /// image names can be set with fixName() (if they have
    /// not been defined via the parset).

    this->itsCube = &cube;
    this->itsSubimageDef = &subdef;
    this->itsInputImage = cube.pars().getImageFile();
    this->itsFlagRobustStats = cube.pars().getFlagRobustStats();
    this->itsSNRthreshold = cube.pars().getCut();
    this->itsSearchType = cube.pars().getSearchType();
    ASKAPCHECK((this->itsSearchType == "spectral") || (this->itsSearchType == "spatial"),
               "SearchType needs to be either 'spectral' or 'spatial' - you have " <<
               this->itsSearchType);

    this->itsSlicer = analysisutilities::subsectionToSlicer(cube.pars().section());
    analysisutilities::fixSlicer(this->itsSlicer, cube.header().getWCS());

    const boost::shared_ptr<SubImage<Float> > sub =
        analysisutilities::getSubImage(cube.pars().getImageFile(), slicer);
    this->itsInputCoordSys = sub->coordinates();
    this->itsInputShape = sub->shape();

    ASKAPLOG_DEBUG_STR(logger , "About to get the section for rank " << this->itsComms->rank());
    duchamp::Section sec = this->itsSubimageDef->section(this->itsComms->rank() - 1);
    ASKAPLOG_DEBUG_STR(logger, "It is " << sec.getSection());
    sec.parse(this->itsInputShape.asStdVector());
    this->itsLocation = casa::IPosition(sec.getStartList());
    ASKAPLOG_DEBUG_STR(logger, "Reference location for rank " << this->itsComms->rank() <<
                       " is " << this->itsLocation << " since local subsection = " <<
                       sec.getSection() << " and input shape = " << this->itsInputShape);


}


void VariableThresholder::calculate()
{
    /// @details Calculate the signal-to-noise at each
    /// pixel. The cube (if it is a cube) is broken up into a
    /// series of lower dimensional data sets - the search
    /// type parameter defines whether this is done as a
    /// series of 2D images or 1D spectra. For each subset,
    /// the "middle" (mean or median) and "spread" (standard
    /// deviation or median absolute deviation from the
    /// median) for each pixel are calculated, and the
    /// signal-to-noise map is formed. At each stage, any
    /// outputs are made, with the subset being written to the
    /// appropriate image at the appropriate location. At the
    /// end, the signal-to-noise map is written to the Cube's
    /// reconstructed array, from where the detections can be
    /// made.


    if (this->itsFlagReuse) {

        ASKAPLOG_INFO_STR(logger, "Reusing SNR map from file " << this->itsSNRimageName);

        casa::Array<Float> snr = analysisutilities::getPixelsInBox(this->itsSNRimageName,
                                 this->itsSlicer);
        if (this->itsCube->getRecon() == 0)
            ASKAPLOG_ERROR_STR(logger,
                               "The Cube's recon array not defined - cannot save SNR map");
        else {
            for (size_t i = 0; i < this->itsCube->getSize(); i++)
                this->itsCube->getRecon()[i] = snr.data()[i];
        }


    } else {


        ASKAPLOG_INFO_STR(logger, "Will calculate the pixel-by-pixel signal-to-noise map");
        if (this->itsSNRimageName != "")
            ASKAPLOG_INFO_STR(logger, "Will write the SNR map to " <<
                              this->itsSNRimageName);
        if (this->itsBoxSumImageName != "")
            ASKAPLOG_INFO_STR(logger, "Will write the box sum map to " <<
                              this->itsBoxSumImageName);
        if (this->itsNoiseImageName != "")
            ASKAPLOG_INFO_STR(logger, "Will write the noise map to " <<
                              this->itsNoiseImageName);
        if (this->itsAverageImageName != "")
            ASKAPLOG_INFO_STR(logger, "Will write the average background map to " <<
                              this->itsAverageImageName);
        if (this->itsThresholdImageName != "")
            ASKAPLOG_INFO_STR(logger, "Will write the flux threshold map to " <<
                              this->itsThresholdImageName);

        int specAxis = this->itsInputCoordSys.spectralAxisNumber();
        int lngAxis = this->itsInputCoordSys.directionAxesNumbers()[0];
        int latAxis = this->itsInputCoordSys.directionAxesNumbers()[1];
        size_t spatsize = this->itsInputShape(lngAxis) * this->itsInputShape(latAxis);
        size_t specsize = (specAxis >= 0) ? this->itsInputShape(specAxis) : 1;
        if (specsize < 1) specsize = 1;
        casa::IPosition chunkshape = this->itsInputShape;
        casa::IPosition box;
        size_t maxCtr;
        if (this->itsSearchType == "spatial") {
            if (specAxis >= 0) chunkshape(specAxis) = 1;
            box = casa::IPosition(2, this->itsBoxSize, this->itsBoxSize);
            maxCtr = specsize;
        } else {
            if (lngAxis >= 0) chunkshape(lngAxis) = 1;
            if (latAxis >= 0) chunkshape(latAxis) = 1;
            box = casa::IPosition(1, this->itsBoxSize);
            maxCtr = spatsize;
        }

        ASKAPLOG_INFO_STR(logger, "Will calculate box-wise signal-to-noise in image of shape " <<
                          this->itsInputShape << " using  '" << this->itsSearchType <<
                          "' mode with chunks of shape " << chunkshape <<
                          " and a box of shape " << box);

        for (size_t ctr = 0; ctr < maxCtr; ctr++) {
            if (maxCtr > 1) ASKAPLOG_DEBUG_STR(logger, "Iteration " << ctr << " of " << maxCtr);
            bool isStart = (ctr == 0);
            casa::Array<Float> inputChunk(chunkshape, 0.);
            casa::MaskedArray<Float> inputMaskedChunk(inputChunk,
                    casa::LogicalArray(chunkshape, true));
            casa::Array<Float> middle(chunkshape, 0.);
            casa::Array<Float> spread(chunkshape, 0.);
            casa::Array<Float> snr(chunkshape, 0.);
            casa::Array<Float> boxsum(chunkshape, 0.);

            casa::IPosition loc(this->itsLocation.size(), 0);
            if (this->itsSearchType == "spatial") {
                if (specAxis >= 0) loc(specAxis) = ctr;
            } else {
                if (lngAxis >= 0) loc(lngAxis) = ctr % this->itsCube->getDimX();
                if (latAxis >= 0) loc(latAxis) = ctr / this->itsCube->getDimX();
            }
            // loc = loc + this->itsSlicer.start();
            loc = loc + this->itsLocation;

            if (this->itsComms->isWorker()) {
                this->defineChunk(inputChunk, inputMaskedChunk, ctr);
                //slidingBoxStats(inputChunk, middle, spread, box, this->itsFlagRobustStats);
                slidingBoxMaskedStats(inputMaskedChunk, middle, spread, box,
                                      this->itsFlagRobustStats);
                // snr = calcSNR(inputChunk,middle,spread);
                snr = calcMaskedSNR(inputMaskedChunk, middle, spread);
                if (this->itsBoxSumImageName != "") {
                    // boxsum = slidingArrayMath(inputChunk, box, SumFunc<Float>());
                    boxsum = slidingArrayMath(inputMaskedChunk, box, MaskedSumFunc<Float>());
                }
            }

            if (this->doWriteImages)
                this->writeImages(middle, spread, snr, boxsum, loc, isStart);

            if (this->itsComms->isWorker()) {
                ASKAPLOG_DEBUG_STR(logger,
                                   "About to store the SNR map to the cube for iteration " <<
                                   ctr << " of " << maxCtr);
                this->saveSNRtoCube(snr, ctr);
            }
        }

    }

    this->itsCube->setReconFlag(true);

}

void VariableThresholder::defineChunk(casa::Array<Float> &inputChunkArr,
                                      casa::MaskedArray<Float> &outputChunk, size_t ctr)
{
    casa::Array<Float>::iterator iter(inputChunkArr.begin());
    int lngAxis = this->itsInputCoordSys.directionAxesNumbers()[0];
    int latAxis = this->itsInputCoordSys.directionAxesNumbers()[1];
    size_t spatsize = this->itsInputShape(lngAxis) * this->itsInputShape(latAxis);
    casa::LogicalArray theMask(inputChunkArr.shape(), true);
    casa::LogicalArray::iterator itermask = theMask.begin();
    if (this->itsSearchType == "spatial") {
        for (size_t i = 0; iter != inputChunkArr.end(); iter++, i++, itermask++) {
            size_t pos = i + ctr * spatsize;
            *iter = this->itsCube->getArray()[pos];
            *itermask = (!this->itsCube->isBlank(pos) && this->itsWeighter->isValid(pos));
        }
    } else {
        for (size_t z = 0; iter != inputChunkArr.end(); iter++, z++, itermask++) {
            size_t pos = ctr + z * spatsize;
            *iter = this->itsCube->getArray()[pos];
            *itermask = (!this->itsCube->isBlank(pos) && this->itsWeighter->isValid(pos));
        }
    }
    outputChunk.setData(inputChunkArr, theMask);
}

void VariableThresholder::saveSNRtoCube(casa::Array<Float> &snr, size_t ctr)
{
    if (this->itsCube->getRecon() == 0)
        ASKAPLOG_ERROR_STR(logger, "The Cube's recon array not defined - cannot save SNR map");
    else {
        casa::Array<Float>::iterator iter(snr.begin());
        int lngAxis = this->itsInputCoordSys.directionAxesNumbers()[0];
        int latAxis = this->itsInputCoordSys.directionAxesNumbers()[1];
        size_t spatsize = this->itsInputShape(lngAxis) * this->itsInputShape(latAxis);
        if (this->itsSearchType == "spatial") {
            for (size_t i = 0; iter != snr.end(); iter++, i++) {
                this->itsCube->getRecon()[i + ctr * spatsize] = *iter;
            }
        } else {
            for (size_t z = 0; iter != snr.end(); iter++, z++) {
                this->itsCube->getRecon()[ctr + z * spatsize] = *iter;
            }
        }
    }

}

void VariableThresholder::writeImages(casa::Array<Float> &middle, casa::Array<Float> &spread,
                                      casa::Array<Float> &snr, casa::Array<Float> &boxsum,
                                      casa::IPosition &loc, bool doCreate)
{

    /// @details Writes the arrays as requested to images on disk. Where
    /// the appropriate image name is defined , the array (one of
    /// mean,noise,boxsum,snr or threshold) is written in distributed
    /// fashion to a CASA image on disk. The 'accumulate' method for
    /// DistributedImageWriter::write is used, taking into account any
    /// overlapping border regions.

    bool addToImage = true;

    if (this->itsNoiseImageName != "") {
        DistributedImageWriter noiseWriter(*this->itsComms, this->itsCube,
                                           this->itsNoiseImageName);
        noiseWriter.create();
        noiseWriter.write(spread, loc, addToImage);
    }

    if (this->itsAverageImageName != "") {
        DistributedImageWriter averageWriter(*this->itsComms, this->itsCube,
                                             this->itsAverageImageName);
        averageWriter.create();
        averageWriter.write(middle, loc, addToImage);
    }

    if (this->itsThresholdImageName != "") {
        DistributedImageWriter threshWriter(*this->itsComms, this->itsCube,
                                            this->itsThresholdImageName);
        threshWriter.create();
        casa::Array<Float> thresh = middle + this->itsSNRthreshold * spread;
        threshWriter.write(thresh, loc, addToImage);
    }

    if (this->itsSNRimageName != "") {
        DistributedImageWriter snrWriter(*this->itsComms, this->itsCube, this->itsSNRimageName);
        snrWriter.create();
        snrWriter.write(snr, loc, addToImage);
    }

    if (this->itsBoxSumImageName != "") {
        DistributedImageWriter boxWriter(*this->itsComms, this->itsCube,
                                         this->itsBoxSumImageName);
        boxWriter.create();
        boxWriter.write(boxsum, loc, addToImage);
    }


}


void VariableThresholder::search()
{
    /// @details Once the signal-to-noise array is defined, we
    /// extract objects from it based on the signal-to-noise
    /// threshold. The resulting object list is put directly
    /// into the duchamp::Cube object, where it can be
    /// accessed from elsewhere. The detection map is updated
    /// and the Duchamp log file can be written to (if
    /// required).

    if (this->itsCube->getRecon() == 0) {
        ASKAPLOG_ERROR_STR(logger,
                           "The Cube's recon array not defined - cannot search for sources.");
    } else {
        if (!this->itsCube->pars().getFlagUserThreshold()) {
            ASKAPLOG_DEBUG_STR(logger, "Setting user threshold to " <<
                               this->itsCube->pars().getCut() << " sigma");
            this->itsCube->pars().setThreshold(this->itsCube->pars().getCut());
            this->itsCube->pars().setFlagUserThreshold(true);
            if (this->itsCube->pars().getFlagGrowth()) {
                ASKAPLOG_DEBUG_STR(logger, "Setting user growth threshold to " <<
                                   this->itsCube->pars().getGrowthCut() << " sigma");
                this->itsCube->pars().setGrowthThreshold(this->itsCube->pars().getGrowthCut());
                this->itsCube->pars().setFlagUserGrowthThreshold(true);
            }
        }

        ASKAPLOG_DEBUG_STR(logger, "Searching SNR map");
        this->itsCube->ObjectList() =
            searchReconArray(this->itsCube->getDimArray(), this->itsCube->getArray(),
                             this->itsCube->getRecon(), this->itsCube->pars(),
                             this->itsCube->stats());
        ASKAPLOG_DEBUG_STR(logger, "Number of sources found = " << this->itsCube->getNumObj());
        this->itsCube->updateDetectMap();
        if (this->itsCube->pars().getFlagLog()) {
            this->itsCube->logDetectionList();
        }
    }
}


}

}
