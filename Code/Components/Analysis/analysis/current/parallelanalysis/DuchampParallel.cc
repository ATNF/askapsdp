/// @file
///
/// @brief Base class for parallel applications
/// @details
/// Supports algorithms by providing methods for initialization
/// of MPI connections, sending and models around.
/// There is assumed to be one master and many workers.
///
/// @copyright (c) 2007 CSIRO
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
/// @author Matthew Whiting <matthew.whiting@csiro.au>
///

#include <Common/LofarTypedefs.h>
#include <Common/ParameterSet.h>
#include <Common/KVpair.h>
using namespace LOFAR::TYPES;
#include <Blob/BlobString.h>
#include <Blob/BlobIBufString.h>
#include <Blob/BlobOBufString.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>
#include <Common/Exceptions.h>

#include <casa/OS/Timer.h>
#include <casa/Utilities/Regex.h>
#include <casa/BasicSL/String.h>
#include <casa/OS/Path.h>
#include <images/Images/ImageOpener.h>
#include <images/Images/FITSImage.h>
#include <images/Images/MIRIADImage.h>
#include <images/Images/SubImage.h>
#include <casa/aipstype.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/ArrayPartMath.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Slicer.h>

// boost includes
#include <boost/shared_ptr.hpp>

#include <askap_analysis.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <askapparallel/AskapParallel.h>
#include <imageaccess/CasaImageAccess.h>

#include <parallelanalysis/DuchampParallel.h>
#include <parallelanalysis/Weighter.h>
#include <parallelanalysis/ParallelStats.h>
#include <parallelanalysis/ObjectParameteriser.h>
#include <preprocessing/VariableThresholder.h>
#include <extraction/ExtractionFactory.h>
#include <analysisutilities/AnalysisUtilities.h>
#include <sourcefitting/RadioSource.h>
#include <sourcefitting/FittingParameters.h>
#include <sourcefitting/CurvatureMapCreator.h>
#include <parametrisation/OptimisedGrower.h>
#include <preprocessing/Wavelet2D1D.h>
#include <outputs/CataloguePreparation.h>
#include <outputs/AskapAsciiCatalogueWriter.h>
#include <outputs/AskapComponentParsetWriter.h>
#include <outputs/AskapVOTableCatalogueWriter.h>
#include <outputs/ImageWriter.h>
#include <outputs/ResultsWriter.h>

#include <casainterface/CasaInterface.h>
#include <analysisparallel/SubimageDef.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <time.h>
#include <vector>
#include <map>

#include <duchamp/duchamp.hh>
#include <duchamp/param.hh>
#include <duchamp/Cubes/cubes.hh>
#include <duchamp/Utils/Statistics.hh>
#include <duchamp/Utils/utils.hh>
#include <duchamp/Utils/VOParam.hh>
#include <duchamp/Detection/detection.hh>
// #include <duchamp/Detection/columns.hh>
#include <duchamp/Outputs/columns.hh>
#include <duchamp/Outputs/CatalogueSpecification.hh>
#include <duchamp/Outputs/KarmaAnnotationWriter.hh>
#include <duchamp/Outputs/DS9AnnotationWriter.hh>
#include <duchamp/Outputs/CasaAnnotationWriter.hh>
#include <duchamp/PixelMap/Voxel.hh>
#include <duchamp/PixelMap/Object3D.hh>

///@brief Where the log messages go.
ASKAP_LOGGER(logger, ".parallelanalysis");

using namespace std;
using namespace askap;
using namespace askap::askapparallel;
using namespace askap::analysisutilities;

using namespace duchamp;

namespace askap {
namespace analysis {

void reportDim(std::vector<size_t> dim)
{

    std::stringstream ss;
    for (size_t i = 0; i < dim.size(); i++) {
        ss << dim[i];
        if (i < dim.size() - 1) ss << " x ";
    }

    ASKAPLOG_INFO_STR(logger, "Dimensions of input image = " << ss.str());

}

//**************************************************************//

bool DuchampParallel::is2D()
{
    int numDim = 0;
    size_t *dim = itsCube.getDimArray();

    for (int i = 0; i < itsCube.getNumDim(); i++){
        if (dim[i] > 1) {
            numDim++;
        }
    }

    return numDim <= 2;
}

//**************************************************************//


DuchampParallel::DuchampParallel(askap::askapparallel::AskapParallel& comms)
    : itsComms(comms)
{
    itsFitParams = sourcefitting::FittingParameters(LOFAR::ParameterSet());
}

//**************************************************************//

DuchampParallel::DuchampParallel(askap::askapparallel::AskapParallel& comms,
                                 const LOFAR::ParameterSet& parset)
    : itsComms(comms),
      itsParset(parset),
      itsWeighter(new Weighter(itsComms, itsParset.makeSubset("Weights."))),
      itsVarThresher(new VariableThresholder(itsComms,
                                             itsParset.makeSubset("VariableThreshold.")))
{

    if (itsComms.isMaster()) {
        ASKAPLOG_INFO_STR(logger,
                          "Initialising parallel finder, based on Duchamp v" <<
                          duchamp::VERSION);
    }

    this->deprecatedParameters();

    // First do the setup needed for both workers and master
    itsCube.pars() = parseParset(itsParset);
    ImageOpener::ImageTypes imageType = ImageOpener::imageType(itsCube.pars().getImageFile());
    itsIsFITSFile = (imageType == ImageOpener::FITS);
    bool useCasa = itsParset.getBool("useCASAforFITS", true);
    itsIsFITSFile = itsIsFITSFile && !useCasa;
    if (itsIsFITSFile){
        ASKAPLOG_DEBUG_STR(logger, "Using the Duchamp FITS-IO tasks");
    }

    bool flagSubsection = itsParset.getBool("flagSubsection", false);
    itsBaseSubsection = itsParset.getString("subsection", "");
    if (!flagSubsection) {
        itsBaseSubsection = "";
    } else {
        ASKAPLOG_DEBUG_STR(logger, "Requested subsection " << itsBaseSubsection);
    }
    if (itsBaseSubsection == "") {
        std::vector<size_t> dim = getCASAdimensions(itsCube.pars().getImageFile());
        itsBaseSubsection = duchamp::nullSection(dim.size());
    }

    itsBaseStatSubsection = itsParset.getBool("flagStatSec", false) ?
                            itsParset.getString("statSec", "") : "" ;

    itsFlagThresholdPerWorker = itsParset.getBool("thresholdPerWorker", false);

//    itsWeighter = new Weighter(itsComms, itsParset.makeSubset("Weights."));

    itsFlagVariableThreshold = itsParset.getBool("VariableThreshold", false);
//    itsVarThresher = new VariableThresholder(itsComms,
//                                             itsParset.makeSubset("VariableThreshold."));

    itsFlagOptimiseMask = itsParset.getBool("optimiseMask", false);

    itsFlagWavelet2D1D = itsParset.getBool("recon2D1D", false);
    itsCube.pars().setFlagATrous(itsCube.pars().getFlagATrous() || itsFlagWavelet2D1D);

    LOFAR::ParameterSet fitParset = itsParset.makeSubset("Fitter.");
    itsFitParams = sourcefitting::FittingParameters(fitParset);
    itsFlagDistribFit = itsParset.getBool("distribFit", true);

    itsFlagFindSpectralTerms = itsParset.getBoolVector("findSpectralTerms",
                               std::vector<bool>(2, itsFitParams.doFit()));
    for (size_t i = itsFlagFindSpectralTerms.size(); i < 2; i++) {
        itsFlagFindSpectralTerms.push_back(false);
    }

    itsSpectralTermImages = itsParset.getStringVector("spectralTermImages",
                            std::vector<std::string>(2, ""));
    for (size_t i = itsSpectralTermImages.size(); i < 2; i++) {
        itsSpectralTermImages.push_back("");
    }

    if (itsFlagFindSpectralTerms[0]) {
        if (!itsFitParams.doFit()) {
            ASKAPLOG_WARN_STR(logger, "No fitting is to be done, " <<
                              "so the spectral indices will not be found. " <<
                              "Setting findSpectralIndex=false.");
            itsFlagFindSpectralTerms = std::vector<bool>(2, false);
        } else {
            this->checkSpectralTermImages();
        }
    } else {
        itsFlagFindSpectralTerms[1] = false;
    }

    itsFlagExtractSpectra = itsParset.getBool("extractSpectra", false);
    if (itsFlagExtractSpectra) {
        if (!itsParset.isDefined("extractSpectra.spectralCube")) {
            ASKAPLOG_WARN_STR(logger, "Source cube not defined for extracting spectra. " <<
                              "Please use the \"spectralCube\" parameter. " <<
                              "Turning off spectral extraction.");
            itsFlagExtractSpectra = false;
            itsParset.replace("extractSpectra", "false");
        } else {
            ASKAPLOG_INFO_STR(logger, "Extracting spectra for detected sources from " <<
                              itsParset.getString("extractSpectra.spectralCube", ""));
        }
    }

    itsFlagExtractNoiseSpectra = itsParset.getBool("extractNoiseSpectra", false);
    if (itsFlagExtractNoiseSpectra) {
        if (!itsParset.isDefined("extractNoiseSpectra.spectralCube")) {
            ASKAPLOG_WARN_STR(logger, "Source cube not defined for extracting noise spectra. " <<
                              "Please use the \"spectralCube\" parameter. " <<
                              "Turning off noise spectra extraction.");
            itsFlagExtractNoiseSpectra = false;
            itsParset.replace("extractNoiseSpectra", "false");
        } else {
            ASKAPLOG_INFO_STR(logger, "Extracting noise spectra for detected sources from " <<
                              itsParset.getString("extractNoiseSpectra.spectralCube", ""));
        }
    }

    if (itsComms.isParallel()) {
        itsSubimageDef = SubimageDef(itsParset);
        int ovx = itsSubimageDef.overlapx();
        int ovy = itsSubimageDef.overlapy();
        int ovz = itsSubimageDef.overlapz();

        // Need the overlap to be at least the boxPadSize that is used
        // by the fitting
        if (itsFitParams.doFit()) {
            if (itsSubimageDef.nsubx() > 1) {
                itsSubimageDef.setOverlapX(std::max(itsSubimageDef.overlapx(),
                                                    itsFitParams.boxPadSize()));
            }
            if (itsSubimageDef.nsuby() > 1) {
                itsSubimageDef.setOverlapY(std::max(itsSubimageDef.overlapy(),
                                                    itsFitParams.boxPadSize()));
            }
            // Don't need to change overlapz, as the fitting box only
            // affects the spatial directions
        }

        // Need the overlap to be at least the full box width so we
        // get full coverage in the variable threshold case.
        if (itsFlagVariableThreshold) {
            if (itsCube.pars().getSearchType() == "spatial") {
                if (itsSubimageDef.nsubx() > 1) {
                    itsSubimageDef.setOverlapX(std::max(itsSubimageDef.overlapx(),
                                                        2 * itsVarThresher->boxSize() + 1));
                }
                if (itsSubimageDef.nsuby() > 1) {
                    itsSubimageDef.setOverlapY(std::max(itsSubimageDef.overlapy(),
                                                        2 * itsVarThresher->boxSize() + 1));
                }
            } else {
                if (itsSubimageDef.nsubz() > 1) {
                    itsSubimageDef.setOverlapZ(std::max(itsSubimageDef.overlapz(),
                                                        2 * itsVarThresher->boxSize()));
                }
            }
        }

        // If values have changed, alert user and update parset.
        if ((itsSubimageDef.overlapx() != ovx) ||
                (itsSubimageDef.overlapy() != ovy) ||
                (itsSubimageDef.overlapz() != ovz)) {
            ASKAPLOG_INFO_STR(logger, "Changed Subimage overlaps to " <<
                              itsSubimageDef.overlapx() << "," <<
                              itsSubimageDef.overlapy() << "," <<
                              itsSubimageDef.overlapz());
            itsParset.replace(LOFAR::KVpair("overlapx", itsSubimageDef.overlapx()));
            itsParset.replace(LOFAR::KVpair("overlapy", itsSubimageDef.overlapy()));
            itsParset.replace(LOFAR::KVpair("overlapz", itsSubimageDef.overlapz()));
        }
    } else {
        itsSubimageDef = SubimageDef();
    }

}

void DuchampParallel::checkAndWarn(std::string oldParam, std::string newParam)
{

    if (itsParset.isDefined(oldParam)) {
        if (newParam == "") { // there is no equivalent anymore
            ASKAPLOG_WARN_STR(logger, "The parameter \"" << oldParam <<
                              "\" has been deprecated and has no equivalent. " <<
                              "Remove it from your parset!");
        } else {
            if (!itsParset.isDefined(newParam)) {
                std::string val = itsParset.getString(oldParam);
                ASKAPLOG_WARN_STR(logger, "The parameter \"" << oldParam <<
                                  "\" should now be given as \"" << newParam <<
                                  "\". Setting this to " << val <<
                                  ", but you should change your parset!");
                itsParset.replace(newParam, val);
            } else {
                ASKAPLOG_WARN_STR(logger, "The parameter \"" << oldParam <<
                                  "\" should now be given as \"" << newParam <<
                                  "\". Your parset has this defined, " <<
                                  " so no change is made, but you should remove " <<
                                  oldParam << " from your parset.");
            }
        }
    }
}

void DuchampParallel::deprecatedParameters()
{

    this->checkAndWarn("doFit", "Fitter.doFit");
    this->checkAndWarn("fitJustDetection", "Fitter.fitJustDetection");
    this->checkAndWarn("doMedianSearch", "VariableThreshold");
    this->checkAndWarn("medianBoxWidth", "VariableThreshold.boxSize");
    this->checkAndWarn("flagWriteSNRimage", "");
    this->checkAndWarn("SNRimageName", "VariableThreshold.SNRimageName");
    this->checkAndWarn("flagWriteThresholdImage", "");
    this->checkAndWarn("ThresholdImageName", "VariableThreshold.ThresholdImageName");
    this->checkAndWarn("flagWriteNoiseImage", "");
    this->checkAndWarn("NoiseImageName", "VariableThreshold.NoiseImageName");
    this->checkAndWarn("weightsimage", "Weights.weightsImage");
}


void DuchampParallel::checkSpectralTermImages()
{

    std::string termname[3] = {".taylor.1", ".taylor.2"};

    for (size_t i = 0; i < 2; i++) {

        if (itsFlagFindSpectralTerms[i]) {

            if (itsSpectralTermImages[i] == "") {
                // if it hasn't been specified, set it to the
                // .taylor.n image, but only if the input is a
                // .taylor.0 image
                size_t pos = itsCube.pars().getImageFile().rfind(".taylor.0");
                if (pos == std::string::npos) {
                    // image provided is not a Taylor series term -
                    // notify and do nothing
                    ASKAPLOG_WARN_STR(logger, "Image name provided (" <<
                                      itsCube.pars().getImageFile() <<
                                      ") is not a Taylor term. Cannot find spectral information.");

                    // set flag for this and higher terms to false
                    for (size_t j = i; j < 2; j++) {
                        itsFlagFindSpectralTerms[j] = false;
                    }

                } else {
                    // it is a taylor.0 image, so set current term's
                    // image appropriately
                    itsSpectralTermImages[i] = itsCube.pars().getImageFile();
                    itsSpectralTermImages[i].replace(pos, 9, termname[i]);
                }

            }

        }

    }

}

//**************************************************************//

void DuchampParallel::setSubimageDefForFITS()
{

    itsSubimageDef.defineFITS(itsCube.pars().getImageFile());
    itsSubimageDef.setImage(itsCube.pars().getImageFile());
    itsSubimageDef.setInputSubsection(itsBaseSubsection);
    std::vector<size_t> dim = getFITSdimensions(itsCube.pars().getImageFile());
    reportDim(dim);
    itsSubimageDef.setImageDim(dim);

    if (!itsCube.pars().getFlagSubsection() || itsCube.pars().getSubsection() == "") {
        itsCube.pars().setFlagSubsection(true);
        itsCube.pars().setSubsection(nullSection(itsSubimageDef.getImageDim().size()));
    }

}

//**************************************************************//

int DuchampParallel::getMetadata()
{
    int returnCode;
    if (itsIsFITSFile) {

        this->setSubimageDefForFITS();

        if (itsCube.pars().verifySubsection() == duchamp::FAILURE) {
            ASKAPTHROW(AskapError, "Cannot parse the subsection string " <<
                       itsCube.pars().getSubsection());
        }

        returnCode = itsCube.getMetadata();
        if (returnCode == duchamp::FAILURE) {
            ASKAPTHROW(AskapError, "Something went wrong with itsCube.getMetadata()");
        }

        // check the true dimensionality and set the 2D flag in the cube header.
        int numDim = 0;
        size_t *dim = itsCube.getDimArray();

        for (int i = 0; i < itsCube.getNumDim(); i++) {
            if (dim[i] > 1) {
                numDim++;
            }
        }

        itsCube.header().set2D(numDim <= 2);

        // set up the various flux units
        if (itsCube.header().getWCS()->spec >= 0) {
            itsCube.header().fixSpectralUnits(itsCube.pars().getSpectralUnits());
        }

    } else {
        returnCode = this->getCASA(METADATA, false);
    }

    return returnCode;
}

//**************************************************************//

std::vector<float> DuchampParallel::getBeamInfo()
{
    std::vector<float> beam(3);
    beam[0] = itsCube.header().beam().maj();
    beam[1] = itsCube.header().beam().min();
    beam[2] = itsCube.header().beam().pa();
    return beam;
}
//**************************************************************//


void DuchampParallel::readData()
{
    if (itsComms.isParallel() && itsComms.isMaster()) {

        ASKAPLOG_INFO_STR(logger,  "About to read metadata from image " <<
                          itsCube.pars().getImageFile());

        int result = this->getMetadata();

        itsSubimageDef.writeAnnotationFile(itsCube.header(), itsComms);

        if (result == duchamp::FAILURE) {
            ASKAPLOG_ERROR_STR(logger, "Could not read in metadata from image " <<
                               itsCube.pars().getImageFile() << ".");
            ASKAPTHROW(AskapError, "Unable to read image " << itsCube.pars().getImageFile())
        } else {
            ASKAPLOG_INFO_STR(logger,  "Read metadata from image " <<
                              itsCube.pars().getImageFile());
        }

        ASKAPLOG_INFO_STR(logger, "Dimensions are " <<
                          itsCube.getDimX() << " " <<
                          itsCube.getDimY() << " " <<
                          itsCube.getDimZ());

        if (itsCube.getDimZ() == 1) {
            itsCube.pars().setMinChannels(0);
        }

    } else if (itsComms.isWorker()) {

        int result;

        if (itsIsFITSFile) {

            this->setSubimageDefForFITS();

            if (itsComms.isParallel()) {
                itsSubimageDef.setInputSubsection(itsBaseSubsection);
                duchamp::Section subsection = itsSubimageDef.section(itsComms.rank() - 1);
                ASKAPLOG_DEBUG_STR(logger,
                                   "Starting with base section = |" << itsBaseSubsection <<
                                   "| and node #" << itsComms.rank() - 1 <<
                                   " we get section " << subsection.getSection());
                itsCube.pars().setFlagSubsection(true);
                itsCube.pars().section() = subsection;
                ASKAPLOG_INFO_STR(logger, "Subsection = " <<
                                  itsCube.pars().section().getSection());
                if (itsCube.pars().getFlagStatSec()) {
                    if (itsCube.pars().statsec().isValid()) {
                        ASKAPLOG_INFO_STR(logger, "Statistics section = " <<
                                          itsCube.pars().statsec().getSection());
                    } else {
                        ASKAPLOG_INFO_STR(logger,
                                          " Worker #" << itsComms.rank() <<
                                          " does not contribute to the statistics section");
                    }
                }
            } else {
                itsCube.pars().setSubsection(itsBaseSubsection);
                ASKAPLOG_INFO_STR(logger, "Subsection = " <<
                                  itsCube.pars().section().getSection());
            }

            if (itsCube.pars().verifySubsection() == duchamp::FAILURE) {
                ASKAPTHROW(AskapError, "Cannot parse the subsection string " <<
                           itsCube.pars().getSubsection());
            }

            ASKAPLOG_INFO_STR(logger, "Using subsection " << itsCube.pars().getSubsection());
            ASKAPLOG_INFO_STR(logger, "About to read data from image " <<
                              itsCube.pars().getFullImageFile());

            bool flag = itsCube.pars().getFlagATrous();
            if (itsFlagVariableThreshold || itsWeighter->doScaling()) {
                itsCube.pars().setFlagATrous(true);
            }
            result = itsCube.getCube();
            if (itsFlagVariableThreshold || itsWeighter->doScaling()) {
                itsCube.pars().setFlagATrous(flag);
            }

        } else { // if it's a CASA image
            result = getCASA(IMAGE);
        }

        if (result == duchamp::FAILURE) {
            ASKAPLOG_ERROR_STR(logger, "Could not read in data from image " <<
                               itsCube.pars().getImageFile());
            ASKAPTHROW(AskapError, "Unable to read image " << itsCube.pars().getImageFile());
        } else {
            ASKAPLOG_INFO_STR(logger, "Dimensions are " <<
                              itsCube.getDimX() << " " <<
                              itsCube.getDimY() << " " <<
                              itsCube.getDimZ());

            if (itsCube.getDimZ() == 1) {
                itsCube.pars().setMinChannels(0);
            }
        }

    }
}

//**************************************************************//

void DuchampParallel::setupLogfile(int argc, const char** argv)
{
    if (itsCube.pars().getFlagLog()) {
        if (itsComms.isParallel()) {
            std::string inputLog = itsCube.pars().getLogFile();
            size_t loc = inputLog.rfind(".");
            std::string suffix = inputLog.substr(loc, inputLog.length());
            std::string addition;
            if (itsComms.isMaster()) addition = ".Master";
            else addition = itsComms.substitute(".%w");
            if (loc != std::string::npos) {
                itsCube.pars().setLogFile(inputLog.insert(loc, addition));
            } else {
                itsCube.pars().setLogFile(inputLog + addition);
            }
        } else {
            // In case the user has put %w in the logfile name but is
            // running in serial mode
            std::string inputLog = itsCube.pars().getLogFile();
            size_t loc;
            while (loc = inputLog.find("%w"), loc != std::string::npos) {
                inputLog.replace(loc, 2, "");
            }
            while (loc = inputLog.find("%n"), loc != std::string::npos) {
                inputLog.replace(loc, 2, "1");
            }
            itsCube.pars().setLogFile(inputLog);
        }
        ASKAPLOG_INFO_STR(logger, "Setting up logfile " << itsCube.pars().getLogFile());
        std::ofstream logfile(itsCube.pars().getLogFile().c_str());
        logfile << "New run of the Selavy sourcefinder: ";
        time_t now = time(NULL);
        logfile << asctime(localtime(&now));
        // Write out the command-line statement
        logfile << "Executing statement : ";

        for (int i = 0; i < argc; i++) {
            logfile << argv[i] << " ";
        }

        logfile << std::endl;
        logfile << itsCube.pars();
        logfile.close();
    }
}

//**************************************************************//

void DuchampParallel::preprocess()
{

    if (itsComms.isParallel() && itsComms.isMaster()) {
        if (itsWeighter->isValid()) {
            itsWeighter->initialise(itsCube, !(itsComms.isParallel() && itsComms.isMaster()));
        }
        if (itsFlagVariableThreshold) {
            itsVarThresher->initialise(itsCube, itsSubimageDef);
            itsVarThresher->calculate();
        }
        // If we are doing fitting, and want to use the curvature map,
        // need to define/calculate this here.
        if (itsFitParams.doFit() && itsFitParams.useCurvature()) {

            CurvatureMapCreator curv(itsComms, itsParset.makeSubset("Fitter."));
            curv.initialise(itsCube, itsSubimageDef);
            ASKAPLOG_DEBUG_STR(logger, "Calling curv.write()");
            curv.write();
        }
    }

    if (itsComms.isWorker()) {

        if (itsWeighter->isValid()) {
            ASKAPLOG_INFO_STR(logger, "Preparing weights image");
            itsWeighter->initialise(itsCube);
            itsWeighter->applyCutoff();
        }

        if (itsCube.pars().getFlagNegative()) {
            ASKAPLOG_INFO_STR(logger, "Inverting cube");
            itsCube.invert();
        }

        if (itsFlagVariableThreshold) {
            ASKAPLOG_INFO_STR(logger, "Defining the variable threshold maps");
            itsVarThresher->initialise(itsCube, itsSubimageDef);
            itsVarThresher->setWeighter(itsWeighter);
            itsVarThresher->calculate();
        } else if (itsFlagWavelet2D1D) {
            ASKAPLOG_INFO_STR(logger, "Reconstructing with the 2D1D wavelet algorithm");
            Recon2D1D recon2d1d(itsParset.makeSubset("recon2D1D."));
            recon2d1d.setCube(&itsCube);
            recon2d1d.reconstruct();
        } else if (itsCube.pars().getFlagATrous()) {
            ASKAPLOG_INFO_STR(logger,  "Reconstructing with dimension " <<
                              itsCube.pars().getReconDim());
            itsCube.ReconCube();
        } else if (itsCube.pars().getFlagSmooth()) {
            ASKAPLOG_INFO_STR(logger,  "Smoothing");
            itsCube.SmoothCube();
        }

        // If we are doing fitting, and want to use the curvature map,
        // need to define/calculate this here.
        if (itsFitParams.doFit() && itsFitParams.useCurvature()) {

            CurvatureMapCreator curv(itsComms, itsParset.makeSubset("Fitter."));
            curv.initialise(itsCube, itsSubimageDef);
            curv.calculate();
            itsFitParams.setSigmaCurv(curv.sigmaCurv());
            ASKAPLOG_DEBUG_STR(logger, "Fitting parameters now think sigma_curv is " <<
                               itsFitParams.sigmaCurv());
            curv.write();
        }

    }

}

//**************************************************************//

void DuchampParallel::findSources()
{

    if (itsComms.isWorker()) {
        // remove mininum size criteria, so we don't miss anything on the borders.
        int minpix = itsCube.pars().getMinPix();
        int minchan = itsCube.pars().getMinChannels();
        int minvox = itsCube.pars().getMinVoxels();

        if (itsComms.isParallel()) {
            itsCube.pars().setMinPix(1);
            itsCube.pars().setMinChannels(1);
            itsCube.pars().setMinVoxels(1);
        }


        if (itsCube.getSize() > 0) {
            if (itsFlagVariableThreshold) {
                ASKAPLOG_INFO_STR(logger, "Searching with a variable threshold");
                itsVarThresher->search();
            } else if (itsWeighter->doScaling()) {
                ASKAPLOG_INFO_STR(logger, "Searching after weight scaling");
                itsWeighter->search();
            } else if (itsCube.pars().getFlagATrous()) {
                ASKAPLOG_INFO_STR(logger,  "Searching with reconstruction first");
                itsCube.ReconSearch();
            } else if (itsCube.pars().getFlagSmooth()) {
                ASKAPLOG_INFO_STR(logger,  "Searching with smoothing first");
                itsCube.SmoothSearch();
            } else {
                ASKAPLOG_INFO_STR(logger,  "Searching, no smoothing or reconstruction done.");
                itsCube.CubicSearch();
            }
        }

        if (itsWeighter->isValid()) {
//            delete itsWeighter;
            itsWeighter.reset();
        }

        ASKAPLOG_INFO_STR(logger,  "Intermediate list has " << itsCube.getNumObj() <<
                          " objects. Now merging.");

        // merge the objects, and grow them if necessary.
        itsCube.ObjectMerger();

        ASKAPLOG_INFO_STR(logger,  "Merged list has " << itsCube.getNumObj() << " objects.");

        if (itsFlagOptimiseMask) {
            // Use the mask optimisation routine provided by WALLABY
            itsCube.calcObjectWCSparams();
            OptimisedGrower grower(itsParset.makeSubset("optimiseMask."));
            ASKAPLOG_DEBUG_STR(logger, "Defining the optimised grower");
            grower.define(&itsCube);
            ASKAPLOG_DEBUG_STR(logger, "Optimising the mask for all " <<
                               itsCube.getNumObj() << " objects");
            double x, y, z;
            for (size_t o = 0; o < itsCube.getNumObj(); o++) {
                Detection *det = itsCube.pObject(o);
                ASKAPLOG_DEBUG_STR(logger, "Object #" << o <<
                                   ", at (RA,DEC)=(" << det->getRA() <<
                                   "," << det->getDec() <<
                                   ") and velocity=" << det->getVel() <<
                                   ". W50 = " << det->getW50() <<
                                   " so the spectral range is from " <<
                                   itsCube.header().velToSpec(det->getV50Min()) <<
                                   " to " << itsCube.header().velToSpec(det->getV50Max()));

                double vel = itsCube.header().velToSpec(det->getVel() + det->getW50());
                itsCube.header().wcsToPix(det->getRA(), det->getDec(), vel, x, y, z);
                int zmax = std::max(0, std::min(int(itsCube.getDimZ() - 1), int(z)));

                vel = itsCube.header().velToSpec(det->getVel() - det->getW50());
                itsCube.header().wcsToPix(det->getRA(), det->getDec(), vel, x, y, z);
                int zmin = std::min(int(itsCube.getDimZ() - 1), std::max(0, int(z)));

                if (zmin > zmax) {
                    std::swap(zmin, zmax);
                }
                grower.setMaxMinZ(zmax, zmin);
                ASKAPLOG_DEBUG_STR(logger, "Central pixel (" << det->getXcentre() <<
                                   "," << det->getYcentre() <<
                                   "," << det->getZcentre() <<
                                   ") with " << det->getSize() <<
                                   " pixels, filling z range " <<
                                   zmin << " to " << zmax);
                grower.grow(det);
                ASKAPLOG_DEBUG_STR(logger, "Now has central pixel (" << det->getXcentre() <<
                                   "," << det->getYcentre() <<
                                   "," << det->getZcentre()
                                   << ") with " << det->getSize() <<
                                   " pixels");
            }
            ASKAPLOG_DEBUG_STR(logger, "Updating the detection map");
            grower.updateDetectMap(itsCube.getDetectMap());
            ASKAPLOG_DEBUG_STR(logger, "Merging objects");
            bool growthflag = itsCube.pars().getFlagGrowth();
            // don't do any further growing in the second lot of
            // merging :
            itsCube.pars().setFlagGrowth(false);
            // do a second merging to clean up any objects that have
            // joined together:
            itsCube.ObjectMerger();
            itsCube.pars().setFlagGrowth(growthflag);
            ASKAPLOG_DEBUG_STR(logger, "Finished mask optimisation");
        }


        if (itsComms.isParallel()) {
            itsCube.pars().setMinPix(minpix);
            itsCube.pars().setMinChannels(minchan);
            itsCube.pars().setMinVoxels(minvox);
        }

        this->finaliseDetection();
    }
}

void DuchampParallel::finaliseDetection()
{

    std::vector<duchamp::Detection> edgelist, goodlist;
    for (size_t i = 0; i < itsCube.getNumObj(); i++) {
        sourcefitting::RadioSource src(itsCube.getObject(i));
        src.setAtEdge(itsCube, itsSubimageDef, itsComms.rank() - 1);
        if (src.isAtEdge()) edgelist.push_back(itsCube.getObject(i));
        else goodlist.push_back(itsCube.getObject(i));
    }
    duchamp::finaliseList(goodlist, itsCube.pars());
    size_t ngood = goodlist.size();
    size_t nedge = edgelist.size();
    itsCube.clearDetectionList();
    for (size_t i = 0; i < edgelist.size(); i++) {
        goodlist.push_back(edgelist[i]);
    }
    itsCube.ObjectList() = goodlist;
    //-------

    ASKAPLOG_DEBUG_STR(logger, "Calculating WCS params");
    itsCube.calcObjectWCSparams();
    if (itsFlagVariableThreshold) {
        // Need to set the peak SNR for each object
        for (size_t i = 0; i < itsCube.getNumObj(); i++) {
            std::vector<Voxel> voxlist = itsCube.getObject(i).getPixelSet();
            for (size_t v = 0; v < voxlist.size(); v++) {
                float snr = itsCube.getReconValue(voxlist[v].getX(),
                                                  voxlist[v].getY(),
                                                  voxlist[v].getZ());
                if (v == 0 || snr > itsCube.getObject(i).getPeakSNR()) {
                    itsCube.pObject(i)->setPeakSNR(snr);
                }
            }
        }
    }
    ASKAPLOG_INFO_STR(logger,  "Found " << itsCube.getNumObj() <<
                      " objects, of which " << nedge <<
                      " are on the boundary and " << ngood << " are good.");

}

//**************************************************************//

void DuchampParallel::fitSources()
{
    if (itsComms.isWorker()) {
        // don't do fit if we have a spectral axis.
        bool flagIs2D = !itsCube.header().canUseThirdAxis() || this->is2D();
        itsFitParams.setFlagDoFit(itsFitParams.doFit() && flagIs2D);

        if (itsFitParams.doFit()) {
            ASKAPLOG_INFO_STR(logger, "Fitting source profiles.");
        }

        for (size_t i = 0; i < itsCube.getNumObj(); i++) {
            if (itsFitParams.doFit()) {
                ASKAPLOG_INFO_STR(logger, "Setting up source #" << i + 1 <<
                                  " / " << itsCube.getNumObj() <<
                                  ", size " << itsCube.getObject(i).getSize() <<
                                  ", peaking at (x,y)=(" <<
                                  itsCube.getObject(i).getXPeak() +
                                  itsCube.getObject(i).getXOffset() <<
                                  "," <<
                                  itsCube.getObject(i).getYPeak() +
                                  itsCube.getObject(i).getYOffset() << ")");
            }

            sourcefitting::RadioSource src(itsCube.getObject(i));
            src.setFitParams(itsFitParams);
            src.defineBox(itsCube.pars().section(),
                          itsCube.header().getWCS()->spec);
            src.setDetectionThreshold(itsCube,
                                      itsFlagVariableThreshold,
                                      itsVarThresher->snrImage());
            src.prepareForFit(itsCube, true);
            // Only do fit if object is not next to boundary
            src.setAtEdge(itsCube, itsSubimageDef, itsComms.rank() - 1);

            if (itsComms.nProcs() == 1) {
                src.setAtEdge(false);
            }

            if (!src.isAtEdge() && itsFitParams.doFit()) {
                this->fitSource(src);
            }

            itsSourceList.push_back(src);
        }
    }
}

//**************************************************************//

void DuchampParallel::fitSource(sourcefitting::RadioSource &src)
{

    if (itsFitParams.fitJustDetection()) {
        ASKAPLOG_DEBUG_STR(logger, "Fitting to detected pixels");
        std::vector<PixelInfo::Voxel> voxlist =
            src.getPixelSet(itsCube.getArray(), itsCube.getDimArray());
        src.fitGauss(voxlist, itsFitParams);
    } else {
        src.fitGauss(itsCube, itsFitParams);
    }

    for (int t = 1; t <= 2; t++) {
        src.findSpectralTerm(itsSpectralTermImages[t - 1], t, itsFlagFindSpectralTerms[t - 1]);
    }

}


//**************************************************************//

void DuchampParallel::sendObjects()
{
    if (itsComms.isWorker()) {
        int32 num = itsCube.getNumObj();
        int16 rank = itsComms.rank();

        if (itsComms.isParallel()) {
            LOFAR::BlobString bs;
            bs.resize(0);
            LOFAR::BlobOBufString bob(bs);
            LOFAR::BlobOStream out(bob);
            out.putStart("detW2M", 1);
            out << rank << num;
            // send the start positions of the subimage
            out << itsCube.pars().section().getStart(0)
                << itsCube.pars().section().getStart(1)
                << itsCube.pars().section().getStart(itsCube.header().getWCS()->spec);
            std::vector<sourcefitting::RadioSource>::iterator src = itsSourceList.begin();

            for (; src < itsSourceList.end(); src++) {
                // for each RadioSource object, send to master
                out << *src;
            }

            out.putEnd();
            itsComms.sendBlob(bs, 0);
            ASKAPLOG_INFO_STR(logger, "Sent detection list to the master");
        }
    }
}

//**************************************************************//

void DuchampParallel::receiveObjects()
{
    if (!itsComms.isParallel() || itsComms.isMaster()) {
        ASKAPLOG_INFO_STR(logger,  "Retrieving lists from workers");

        if (itsComms.isParallel()) {
            LOFAR::BlobString bs;
            int16 rank;
            int32 numObj;

            // don't do fit if we have a spectral axis.
            bool flagIs2D = !itsCube.header().canUseThirdAxis() || this->is2D();
            itsFitParams.setFlagDoFit(itsFitParams.doFit() && flagIs2D);

            // list of fit types, for use in correcting positions of fitted components
            std::vector<std::string>::iterator fittype;
            std::vector<std::string> fittypelist = sourcefitting::availableFitTypes;
            fittypelist.push_back("best");
            fittypelist.push_back("guess");

            for (int i = 1; i < itsComms.nProcs(); i++) {
                ASKAPLOG_DEBUG_STR(logger, "In loop #" << i << " of reading from workers");
                itsComms.receiveBlob(bs, i);
                LOFAR::BlobIBufString bib(bs);
                LOFAR::BlobIStream in(bib);
                int version = in.getStart("detW2M");
                ASKAPASSERT(version == 1);
                in >> rank >> numObj;
                ASKAPLOG_INFO_STR(logger, "Starting to read " << numObj <<
                                  " objects from worker #" << rank);
                int xstart, ystart, zstart;
                in >> xstart >> ystart >> zstart;

                for (int obj = 0; obj < numObj; obj++) {
                    sourcefitting::RadioSource src;
                    in >> src;
                    // Correct for any offsets.  If the full cube is a
                    // subsection of a larger one, then we need to
                    // correct for what the master offsets are.
                    src.setXOffset(xstart - itsCube.pars().getXOffset());
                    src.setYOffset(ystart - itsCube.pars().getYOffset());
                    src.setZOffset(zstart - itsCube.pars().getZOffset());
                    src.addOffsets();
                    src.calcParams();
                    src.calcWCSparams(itsCube.header());

                    // And now set offsets to those of the full image
                    // as we are in the master cube
                    src.setOffsets(itsCube.pars());
                    src.setFitParams(itsFitParams);
                    src.defineBox(itsCube.pars().section(),
                                  itsCube.header().getWCS()->spec);
                    if (src.isAtEdge()) {
                        itsEdgeSourceList.push_back(src);
                    } else {
                        src.setHeader(itsCube.header());
                        if (src.hasEnoughChannels(itsCube.pars().getMinChannels())
                                && (src.getSpatialSize() >= itsCube.pars().getMinPix())) {
                            // Only add the source if it meets the true criteria for size
                            itsSourceList.push_back(src);
                        }
                    }

                }
                ASKAPLOG_INFO_STR(logger, "Received list of size " << numObj <<
                                  " from worker #" << rank);
                ASKAPLOG_INFO_STR(logger, "Now have " <<
                                  itsSourceList.size() << " good objects and " <<
                                  itsEdgeSourceList.size() << " edge objects");
                in.getEnd();
            }
        }

    }

}

//**************************************************************//

void DuchampParallel::cleanup()
{

    if (itsComms.isParallel() && itsComms.isWorker()) {
        // need to call ObjectParameteriser only, so that the distributed calculation works

        ASKAPLOG_DEBUG_STR(logger, "Parameterising edge objects in distributed manner");
        ObjectParameteriser objParam(itsComms);
        objParam.initialise(this);
        objParam.distribute();
        objParam.parameterise();
        objParam.gather();

    }


    if (!itsComms.isParallel() || itsComms.isMaster()) {
        ASKAPLOG_INFO_STR(logger, "Beginning the cleanup");

        std::vector<sourcefitting::RadioSource>::iterator src;

        ASKAPLOG_INFO_STR(logger, "num edge sources in cube = " << itsEdgeSourceList.size());

        itsCube.clearDetectionList();

        if (itsEdgeSourceList.size() > 0) { // if there are edge sources
            for (src = itsEdgeSourceList.begin();
                    src < itsEdgeSourceList.end();
                    src++) {
                itsCube.addObject(*src);
            }

            ASKAPLOG_INFO_STR(logger, "num edge sources in cube = " << itsCube.getNumObj());
            bool growthflag = itsCube.pars().getFlagGrowth();
            // can't grow as don't have flux array in itsCube
            itsCube.pars().setFlagGrowth(false);

            ///@todo Need to grow edge sources before sending to
            ///master, which means finding objects at edge above
            ///growth threshold but below detection threshold

            ASKAPLOG_INFO_STR(logger, "Merging edge sources");
            itsCube.ObjectMerger();
            ASKAPLOG_INFO_STR(logger, "num edge sources in cube after merging = " <<
                              itsCube.getNumObj());
            itsCube.pars().setFlagGrowth(growthflag);

            itsEdgeSourceList.clear();
            for (size_t i = 0; i < itsCube.getNumObj(); i++) {
                sourcefitting::RadioSource src(itsCube.getObject(i));

                src.setFitParams(itsFitParams);
                src.defineBox(itsCube.pars().section(),
                              itsCube.header().getWCS()->spec);

                itsEdgeSourceList.push_back(src);
            }

        }

        ObjectParameteriser objParam(itsComms);
        objParam.initialise(this);
        objParam.distribute();
        objParam.parameterise();
        objParam.gather();

        ASKAPLOG_INFO_STR(logger, "Finished parameterising " << itsEdgeSourceList.size()
                          << " edge sources");

        for (src = itsEdgeSourceList.begin(); src < itsEdgeSourceList.end(); src++) {
            ASKAPLOG_DEBUG_STR(logger, "'Edge' source, name " << src->getName());
            itsSourceList.push_back(*src);
        }
        itsEdgeSourceList.clear();

        ASKAPLOG_INFO_STR(logger, "Now have a total of " << itsSourceList.size() << " sources.");

        SortDetections(itsSourceList, itsCube.pars().getSortingParam());

        itsCube.clearDetectionList();

        for (src = itsSourceList.begin(); src < itsSourceList.end(); src++) {
            src->setID(src - itsSourceList.begin() + 1);
            src->setAtEdge(itsCube, itsSubimageDef, itsComms.rank() - 1);

            if (src->isAtEdge()) src->addToFlagText("E");
            else src->addToFlagText("-");

            itsCube.addObject(duchamp::Detection(*src));
        }

        ASKAPLOG_INFO_STR(logger, "Finished adding sources to cube. Now have " <<
                          itsCube.getNumObj() << " objects.");

    }

}

//**************************************************************//

void DuchampParallel::printResults()
{
    if (itsComms.isMaster()) {

        itsCube.sortDetections();

        std::vector<std::string> outtypes = itsFitParams.fitTypes();
        outtypes.push_back("best");

        if (itsCube.pars().getFlagNegative()) {
            itsCube.invert(false, true);

            std::vector<sourcefitting::RadioSource>::iterator src;
            for (src = itsSourceList.begin(); src < itsSourceList.end(); src++) {
                for (size_t t = 0; t < outtypes.size(); t++) {
                    for (size_t i = 0; i < src->numFits(outtypes[t]); i++) {
                        Double f = src->fitset(outtypes[t])[i].flux();
                        src->fitset(outtypes[t])[i].setFlux(f * -1);
                    }
                }
            }

        }
        ASKAPLOG_INFO_STR(logger, "Found " << itsCube.getNumObj() << " sources.");

        ResultsWriter writer(this);
        writer.duchampOutput();
        writer.writeIslandCatalogue();
        writer.writeComponentCatalogue();
        writer.writeFitResults();
        writer.writeFitAnnotations();
        writer.writeComponentParset();


    } // end of 'if isMaster'
}

//**************************************************************//

void DuchampParallel::extract()
{

    for (size_t i = 0; i < itsSourceList.size(); i++) {
        // make sure the boxes are defined for each of the sources prior to distribution
        itsSourceList[i].defineBox(itsCube.pars().section(),
                                   itsCube.header().getWCS()->spec);
    }

    ExtractionFactory extractor(itsComms, itsParset);
    extractor.setParams(itsCube.pars());
    extractor.setSourceList(itsSourceList);
    extractor.distribute();
    extractor.extract();

}

void DuchampParallel::writeToFITS()
{
    if (!itsIsFITSFile) {
        if (itsComms.isMaster()) {
            ASKAPLOG_WARN_STR(logger, "Writing the Duchamp-style FITS arrays " <<
                              "currently requires the input file to be FITS, " <<
                              "which is not the case here.");
        }
    } else if (!itsComms.isParallel()) {
        itsCube.pars().setFlagBlankPix(false);
        itsCube.writeToFITS();
    }

}


//**************************************************************//

void DuchampParallel::gatherStats()
{
    if (itsFlagVariableThreshold) {
        if (itsCube.pars().getFlagUserThreshold()) {
            ASKAPLOG_WARN_STR(logger,
                              "Since a variable threshold has been requested, " <<
                              " the threshold given (" << itsCube.pars().getThreshold() <<
                              ") is changed to a S/N-based one of " << itsCube.pars().getCut() <<
                              " sigma");
        }

        ASKAPLOG_DEBUG_STR(logger, "Setting user threshold to " << itsCube.pars().getCut());
        itsCube.pars().setThreshold(itsCube.pars().getCut());
        itsCube.pars().setFlagUserThreshold(true);
        if (itsCube.pars().getFlagGrowth()) {
            ASKAPLOG_DEBUG_STR(logger, "Setting user growth threshold to " <<
                               itsCube.pars().getGrowthCut());
            itsCube.pars().setGrowthThreshold(itsCube.pars().getGrowthCut());
            itsCube.pars().setFlagUserGrowthThreshold(true);
        }
        itsCube.stats().setThreshold(itsCube.pars().getCut());

    } else if (!itsComms.isParallel() || itsFlagThresholdPerWorker) {
        if (itsComms.isWorker()) {
            if (itsComms.isParallel()) {
                ASKAPLOG_DEBUG_STR(logger, "Calculating stats for each worker individually");
            } else {
                ASKAPLOG_DEBUG_STR(logger, "Calculating stats");
            }
            itsCube.setCubeStats();
            ASKAPLOG_INFO_STR(logger, "Stats are as follows:");
            std::cout << itsCube.stats();
        }
        if (itsComms.isParallel() && itsComms.isMaster()) {
            itsCube.stats().setThreshold(itsCube.pars().getCut());
            itsCube.pars().setThreshold(itsCube.pars().getCut());
        } else {
            itsCube.pars().setThreshold(itsCube.stats().getThreshold());
        }
        itsCube.pars().setFlagUserThreshold(true);
        ASKAPLOG_INFO_STR(logger, "Threshold = " << itsCube.stats().getThreshold());
    } else if (!itsFlagVariableThreshold &&
               (!itsCube.pars().getFlagUserThreshold() ||
                (itsCube.pars().getFlagGrowth() &&
                 !itsCube.pars().getFlagUserGrowthThreshold()))) {

        ParallelStats parstats(itsComms, &itsCube);
        parstats.findDistributedStats();

    } else {
        itsCube.stats().setThreshold(itsCube.pars().getThreshold());
    }
}

//**************************************************************//

void DuchampParallel::setThreshold()
{

    if (!itsFlagThresholdPerWorker) {
        // when doing a threshold per worker, have already set the threshold.

        double threshold, mean, stddev;
        if (itsComms.isParallel()) {
            if (itsComms.isMaster()) {
                LOFAR::BlobString bs;
                bs.resize(0);
                LOFAR::BlobOBufString bob(bs);
                LOFAR::BlobOStream out(bob);
                out.putStart("threshM2W", 1);
                threshold = itsCube.stats().getThreshold();
                mean = itsCube.stats().getMiddle();
                stddev = itsCube.stats().getSpread();
                out << threshold << mean << stddev;
                out.putEnd();
                itsComms.broadcastBlob(bs, 0);
                ASKAPLOG_INFO_STR(logger, "Threshold = " << itsCube.stats().getThreshold());
            } else if (itsComms.isWorker()) {
                LOFAR::BlobString bs;
                itsComms.broadcastBlob(bs, 0);
                LOFAR::BlobIBufString bib(bs);
                LOFAR::BlobIStream in(bib);
                int version = in.getStart("threshM2W");
                ASKAPASSERT(version == 1);
                in >> threshold >> mean >> stddev;
                in.getEnd();
                itsCube.stats().setRobust(false);
                itsCube.stats().setMean(mean);
                itsCube.stats().setStddev(stddev);
                itsCube.stats().define(itsCube.stats().getMiddle(), 0.F,
                                       itsCube.stats().getSpread(), 1.F);

                if (!itsCube.pars().getFlagUserThreshold()) {
                    itsCube.stats().setThresholdSNR(itsCube.pars().getCut());
                    itsCube.pars().setFlagUserThreshold(true);
                    itsCube.pars().setThreshold(itsCube.stats().getThreshold());
                }
            } else ASKAPTHROW(AskapError, "Neither Master nor Worker!");
        } else {
            // serial case
            if (itsCube.pars().getFlagUserThreshold()) {
                threshold = itsCube.pars().getThreshold();
            } else {
                threshold = itsCube.stats().getMiddle() +
                            itsCube.stats().getSpread() * itsCube.pars().getCut();
            }
        }
        ASKAPLOG_INFO_STR(logger, "Setting threshold to be " << threshold);
        itsCube.pars().setThreshold(threshold);
    }
}


//**************************************************************//


duchamp::OUTCOME DuchampParallel::getCASA(DATATYPE typeOfData, bool useSubimageInfo)
{

    const boost::shared_ptr<ImageInterface<Float> > imagePtr =
        openImage(itsCube.pars().getImageFile());

    // Define the subimage - need to be done before metadata, as the
    // latter needs the subsection & offsets
    const boost::shared_ptr<SubImage<Float> > sub =
        this->getSubimage(imagePtr, useSubimageInfo);

    if (this->getCasaMetadata(sub, typeOfData) == duchamp::FAILURE) {
        return duchamp::FAILURE;
    }

    ASKAPLOG_DEBUG_STR(logger, "Have subimage with shape " <<
                       sub->shape() << " and subsection " <<
                       itsCube.pars().section().getSection());

    if (typeOfData == IMAGE) {

        ASKAPLOG_INFO_STR(logger, "Reading data from image " << itsCube.pars().getImageFile());

        casa::Array<Float> subarray(sub->shape());
        const casa::MaskedArray<Float> msub(sub->get(), sub->getMask());
        float minval = min(msub) - 10.;
        subarray = msub;
        if (sub->hasPixelMask()) {
            subarray(!sub->getMask()) = minval;
            itsCube.pars().setBlankPixVal(minval);
            itsCube.pars().setBlankKeyword(0);
            itsCube.pars().setBscaleKeyword(1.);
            itsCube.pars().setBzeroKeyword(minval);
            itsCube.pars().setFlagBlankPix(true);
        }

        std::vector<size_t> dim = getDim(sub);
        // A HACK TO ENSURE THE RECON ARRAY IS ALLOCATED IN THE CASE
        // OF VARIABLE THRESHOLD OR WEIGHTS IMAGE SCALING
        bool flag = itsCube.pars().getFlagATrous();
        if (itsFlagVariableThreshold || itsWeighter->doScaling()) {
            itsCube.pars().setFlagATrous(true);
        }
        itsCube.initialiseCube(dim.data());
        if (itsFlagVariableThreshold || itsWeighter->doScaling()) {
            itsCube.pars().setFlagATrous(flag);
        }
        if (itsCube.getDimZ() == 1) {
            itsCube.pars().setMinChannels(0);
        }
        itsCube.saveArray(subarray.data(), subarray.size());

    }

    return duchamp::SUCCESS;

}

//**************************************************************//

const boost::shared_ptr<SubImage<Float> >
DuchampParallel::getSubimage(const boost::shared_ptr<ImageInterface<Float> > imagePtr, bool useSubimageInfo)
{

    wcsprm *wcs = casaImageToWCS(imagePtr);
    itsSubimageDef.define(wcs);
    itsSubimageDef.setImage(itsCube.pars().getImageFile());
    itsSubimageDef.setInputSubsection(itsBaseSubsection);
    std::vector<size_t> dim = getDim(imagePtr);
    reportDim(dim);
    itsSubimageDef.setImageDim(dim);

    if (useSubimageInfo && (!itsComms.isParallel() || itsComms.isWorker())) {
        itsCube.pars().section() = itsSubimageDef.section(itsComms.rank() - 1);
    } else if (!itsCube.pars().getFlagSubsection() || itsCube.pars().getSubsection() == "") {
        itsCube.pars().setSubsection(nullSection(itsSubimageDef.getImageDim().size()));
    }
    itsCube.pars().setFlagSubsection(true);

    // Now parse the sections to get them properly set up
    if (itsCube.pars().parseSubsections(dim) == duchamp::FAILURE) {
        // if here, something went wrong - try to detect and throw appropriately
        if (itsCube.pars().section().parse(dim) == duchamp::FAILURE) {
            ASKAPTHROW(AskapError, "Cannot parse the subsection string " <<
                       itsCube.pars().section().getSection());
        }
        if (itsCube.pars().getFlagStatSec() &&
                itsCube.pars().statsec().parse(dim) == duchamp::FAILURE) {
            ASKAPTHROW(AskapError, "Cannot parse the statistics subsection string " <<
                       itsCube.pars().statsec().getSection());
        }
        if (!itsCube.pars().section().isValid()) {
            ASKAPTHROW(AskapError, "Pixel subsection " << itsBaseSubsection <<
                       " has no pixels");
        }
        if (itsCube.pars().getFlagStatSec() && !itsCube.pars().statsec().isValid()) {
            ASKAPTHROW(AskapError, "Statistics subsection " << itsBaseStatSubsection <<
                       " has no pixels in common with the image or " <<
                       "the pixel subsection requested");
        }
    }

    if (itsComms.isMaster() & itsCube.pars().getFlagStatSec() &&
            !itsCube.pars().statsec().isValid()) {
        ASKAPTHROW(AskapError, "Statistics subsection has no valid pixels");
    }

    ASKAPLOG_INFO_STR(logger, "Using subsection " << itsCube.pars().section().getSection());
    if (itsCube.pars().getFlagStatSec() && itsCube.pars().statsec().isValid()) {
        ASKAPLOG_INFO_STR(logger, "Using stat-subsection " <<
                          itsCube.pars().statsec().getSection());
    }

    Slicer slice = subsectionToSlicer(itsCube.pars().section());
    fixSlicer(slice, wcs);

    const boost::shared_ptr<SubImage<Float> > sub(new SubImage<Float>(*imagePtr, slice));

    return sub;
}

//**************************************************************//

duchamp::OUTCOME
DuchampParallel::getCasaMetadata(const boost::shared_ptr<ImageInterface<Float> > imagePtr,
                                 DATATYPE typeOfData)
{

    std::vector<size_t> dim = getDim(imagePtr);
    wcsprm *wcs = casaImageToWCS(imagePtr);
    ASKAPLOG_DEBUG_STR(logger, "Defining WCS and putting into type \"" <<
                       itsCube.pars().getSpectralType() << "\"");
    itsCube.header().defineWCS(wcs, 1, dim.data(), itsCube.pars());
    itsCube.pars().setOffsets(wcs);
    readBeamInfo(imagePtr, itsCube.header(), itsCube.pars());
    itsCube.header().setFluxUnits(imagePtr->units().getName());

    // check the true dimensionality and set the 2D flag in the cube header.
    itsCube.header().set2D(imagePtr->shape().nonDegenerate().size() <= 2);

    // set up the various flux units
    if (wcs->spec >= 0) {
        itsCube.header().fixSpectralUnits(itsCube.pars().getSpectralUnits());
    }

    itsCube.header().setIntFluxUnits();

    if (typeOfData == METADATA) {
        itsCube.initialiseCube(dim.data(), false);
    }

    return duchamp::SUCCESS;


}


}
}
