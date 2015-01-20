/// @file
///
/// @brief General utility functions to support the analysis software
/// @details
/// These functions are unattached to any classes, but provide simple
/// support for the rest of the analysis package.
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

#include <askap_analysis.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <askapparallel/AskapParallel.h>

#include <analysisutilities/AnalysisUtilities.h>

#include <gsl/gsl_sf_gamma.h>

#include <casa/namespace.h>
#include <scimath/Functionals/Gaussian2D.h>
#include <images/Images/FITSImage.h>
#include <images/Images/MIRIADImage.h>
#include <images/Images/ImageOpener.h>
#include <images/Images/SubImage.h>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

#include <duchamp/fitsHeader.hh>
#include <duchamp/Utils/Statistics.hh>
#include <duchamp/Utils/Section.hh>
#include <duchamp/FitsIO/Beam.hh>
#include <duchamp/param.hh>

#define WCSLIB_GETWCSTAB // define this so that we don't try and redefine 
//  wtbarr (this is a problem when using gcc v.4+)
#include <fitsio.h>

///@brief Where the log messages go.
ASKAP_LOGGER(logger, ".analysisutilities");

namespace askap {
namespace analysis {


std::vector<size_t> getFITSdimensions(std::string filename)
{
    /// @details A simple function to open a FITS file and read the
    /// axis dimensions, returning the array of values.
    int numAxes, status = 0;  /* MUST initialize status */
    fitsfile *fptr;
    std::vector<size_t> dim;
    // Open the FITS file
    status = 0;

    if (fits_open_file(&fptr, filename.c_str(), READONLY, &status)) {
        fits_report_error(stderr, status);
        ASKAPTHROW(AskapError, "FITS Error opening file")
    } else {
        // Read the size of the FITS file -- number and sizes of the axes
        status = 0;

        if (fits_get_img_dim(fptr, &numAxes, &status)) {
            fits_report_error(stderr, status);
        }

        std::vector<long> dimAxes(numAxes, 1);

        status = 0;

        if (fits_get_img_size(fptr, numAxes, dimAxes.data(), &status)) {
            fits_report_error(stderr, status);
        }

        // Close the FITS file -- not needed any more in this function.
        status = 0;
        fits_close_file(fptr, &status);

        if (status) {
            fits_report_error(stderr, status);
        }

        dim = std::vector<size_t>(numAxes);

        for (int i = 0; i < numAxes; i++) {
            dim[i] = dimAxes[i];
        }

    }

    return dim;
}


void checkUnusedParameter(const LOFAR::ParameterSet& parset, const std::string &paramName)
{

    if (parset.isDefined(paramName)) {
        ASKAPLOG_WARN_STR(logger, "Parameter '" << paramName <<
                          "' is not used by the ASKAP duchamp implementation");
    }

}

duchamp::Param parseParset(const LOFAR::ParameterSet& parset)
{

    duchamp::Param par;

    if (parset.isDefined("image")) {
        par.setImageFile(parset.getString("image"));
    } else if (parset.isDefined("imageFile")) {
        par.setImageFile(parset.getString("imageFile"));
    } else {
        ASKAPLOG_ERROR_STR(logger,
                           "No image defined - use either 'imageFile' or 'image' " <<
                           "parameters (the former is for consistency with Duchamp parameters)");
    }
    par.setFlagSubsection(parset.getBool("flagSubsection", false));
    par.setSubsection(parset.getString("subsection", ""));
    if (!par.getFlagSubsection()) par.setSubsection("");
    checkUnusedParameter(parset, "flagReconExists");
    checkUnusedParameter(parset, "reconFile");
    checkUnusedParameter(parset, "flagSmoothExists");
    checkUnusedParameter(parset, "smoothFile");
    par.setFlagUsePrevious(parset.getBool("usePrevious", par.getFlagUsePrevious()));
    par.setObjectList(parset.getString("objectList", par.getObjectList()));

    par.setFlagLog(parset.getBool("flagLog", par.getFlagLog()));
    par.setLogFile(parset.getString("logFile", par.getLogFile()));
    if (parset.isDefined("resultsFile")) {
        par.setOutFile(parset.getString("resultsFile", par.getOutFile()));
    } else {
        par.setOutFile(parset.getString("outFile", par.getOutFile()));
    }
    par.setFlagSeparateHeader(parset.getBool("flagSeparateHeader",
                              par.getFlagSeparateHeader()));
    par.setHeaderFile(parset.getString("headerFile", par.getHeaderFile()));
    par.setFlagWriteBinaryCatalogue(parset.getBool("flagWriteBinaryCatalogue",
                                    par.getFlagWriteBinaryCatalogue()));
    par.setBinaryCatalogue(parset.getString("binaryCatalogue", par.getBinaryCatalogue()));
    par.setFlagPlotSpectra(false); // no graphics, so not plotting
    checkUnusedParameter(parset, "flagPlotSpectra"); // no graphics
    checkUnusedParameter(parset, "flagPlotIndividualSpectra"); // no graphics
    checkUnusedParameter(parset, "spectraFile"); // no graphics
    par.setFlagTextSpectra(parset.getBool("flagTextSpectra", par.getFlagTextSpectra()));
    par.setSpectraTextFile(parset.getString("spectraTextFile", par.getSpectraTextFile()));
    checkUnusedParameter(parset, "flagOutputBaseline"); //
    checkUnusedParameter(parset, "fileOutputBaseline"); //
    par.setFlagOutputMomentMask(parset.getBool("flagOutputMomentMask",
                                par.getFlagOutputMomentMask()));
    par.setFileOutputMomentMask(parset.getString("fileOutputMomentMask",
                                par.getFileOutputMomentMask()));
    par.setFlagOutputMask(parset.getBool("flagOutputMask", par.getFlagOutputMask()));
    par.setFileOutputMask(parset.getString("fileOutputMask", par.getFileOutputMask()));
    par.setFlagMaskWithObjectNum(parset.getBool("flagMaskWithObjectNum",
                                 par.getFlagMaskWithObjectNum()));
    par.setFlagOutputSmooth(parset.getBool("flagOutputSmooth", par.getFlagOutputSmooth()));
    par.setFileOutputSmooth(parset.getString("fileOutputSmooth", par.getFileOutputSmooth()));
    par.setFlagOutputRecon(parset.getBool("flagOutputRecon", par.getFlagOutputRecon()));
    par.setFileOutputRecon(parset.getString("fileOutputRecon", par.getFileOutputRecon()));
    par.setFlagOutputResid(parset.getBool("flagOutputResid", par.getFlagOutputResid()));
    par.setFileOutputResid(parset.getString("fileOutputResid", par.getFileOutputResid()));
    par.setFlagVOT(parset.getBool("flagVOT", true));
    par.setVOTFile(parset.getString("votFile", par.getVOTFile()));
    par.setFlagKarma(parset.getBool("flagKarma", true)); // different from Duchamp default
    par.setKarmaFile(parset.getString("karmaFile", par.getKarmaFile()));
    par.setFlagDS9(parset.getBool("flagDS9", true)); // different from Duchamp default
    par.setDS9File(parset.getString("ds9File", par.getDS9File()));
    par.setFlagCasa(parset.getBool("flagCasa", true)); // different from Duchamp default
    par.setCasaFile(parset.getString("casaFile", par.getCasaFile()));
    //
    par.setFlagMaps(false); // flagMaps
    checkUnusedParameter(parset, "flagMaps"); //  - not using X
    checkUnusedParameter(parset, "detectMap"); //  - not using X
    checkUnusedParameter(parset, "momentMap"); //  - not using X
    par.setFlagXOutput(false);
    checkUnusedParameter(parset, "flagXOutput"); //  - not using X
    checkUnusedParameter(parset, "newFluxUnits"); //  - not using - caused confusion...
    par.setPrecFlux(parset.getInt16("precFlux", par.getPrecFlux()));
    par.setPrecVel(parset.getInt16("precVel", par.getPrecVel()));
    par.setPrecSNR(parset.getInt16("precSNR", par.getPrecSNR()));

    checkUnusedParameter(parset, "flagTrim"); // Not clear if this is necessary for our case.
    par.setFlaggedChannelList(parset.getString("flaggedChannels", ""));

    // Need additional infrastructure to pass baseline values to Master.
    checkUnusedParameter(parset, "flagBaseline");
    checkUnusedParameter(parset, "baselineType");
    checkUnusedParameter(parset, "baselineBoxWidth");

    par.setFlagStatSec(parset.getBool("flagStatSec", par.getFlagStatSec()));
    par.setStatSec(parset.getString("statsec", par.getStatSec()));
    par.setFlagRobustStats(parset.getBool("flagRobustStats", par.getFlagRobustStats()));
    par.setFlagNegative(parset.getBool("flagNegative", par.getFlagNegative()));
    par.setCut(parset.getFloat("snrCut", par.getCut()));
    if (parset.isDefined("threshold")) {
        par.setFlagUserThreshold(true);
        par.setThreshold(parset.getFloat("threshold", par.getThreshold()));
    } else {
        par.setFlagUserThreshold(false);
    }
    par.setFlagGrowth(parset.getBool("flagGrowth", par.getFlagGrowth()));
    par.setGrowthCut(parset.getFloat("growthCut", par.getGrowthCut()));
    if (parset.isDefined("growthThreshold")) {
        par.setGrowthThreshold(parset.getFloat("growthThreshold", par.getGrowthThreshold()));
        par.setFlagUserGrowthThreshold(true);
    }
    if (parset.isDefined("beamSize")) {
        par.setBeamSize(parset.getFloat("beamSize"));
        ASKAPLOG_WARN_STR(logger, "Parset has beamSize parameter. " <<
                          "This is deprecated from Duchamp 1.1.9 onwards - use beamArea instead."
                          << " Setting beamArea=" << par.getBeamSize());
    }
    par.setBeamSize(parset.getFloat("beamArea", par.getBeamSize()));
    par.setBeamFWHM(parset.getFloat("beamFWHM", par.getBeamFWHM()));
    par.setSearchType(parset.getString("searchType", par.getSearchType()));

    //

    par.setFlagATrous(parset.getBool("flagATrous", par.getFlagATrous()));
    par.setReconDim(parset.getInt16("reconDim", par.getReconDim()));
    par.setMinScale(parset.getInt16("scaleMin", par.getMinScale()));
    par.setMaxScale(parset.getInt16("scaleMax", par.getMaxScale()));
    par.setAtrousCut(parset.getFloat("snrRecon", par.getAtrousCut()));
    par.setReconConvergence(parset.getFloat("reconConvergence", par.getReconConvergence()));
    par.setFilterCode(parset.getInt16("filterCode", par.getFilterCode()));

    //

    if (par.getFlagATrous()) par.setFlagSmooth(false);
    else par.setFlagSmooth(parset.getBool("flagSmooth", false));
    par.setSmoothType(parset.getString("smoothType", par.getSmoothType()));
    par.setHanningWidth(parset.getInt16("hanningWidth", par.getHanningWidth()));
    par.setKernMaj(parset.getFloat("kernMaj", par.getKernMaj()));
    par.setKernMin(parset.getFloat("kernMin", par.getKernMin()));
    par.setKernPA(parset.getFloat("kernPA", par.getKernPA()));
    par.setSmoothEdgeMethod(parset.getString("smoothEdgeMethod", par.getSmoothEdgeMethod()));
    par.setSpatialSmoothCutoff(parset.getFloat("spatialSmoothCutoff",
                               par.getSpatialSmoothCutoff()));

    checkUnusedParameter(parset, "flagFDR"); // ? How to deal with distributed case?
    checkUnusedParameter(parset, "alphaFDR"); //  ?
    checkUnusedParameter(parset, "FDRnumCorChan"); //  ?

    par.setFlagAdjacent(parset.getBool("flagAdjacent", par.getFlagAdjacent()));
    par.setThreshS(parset.getFloat("threshSpatial", par.getThreshS()));
    par.setThreshV(parset.getFloat("threshVelocity", par.getThreshV()));
    par.setMinPix(parset.getInt16("minPix", par.getMinPix()));
    par.setMinChannels(parset.getInt16("minChannels", par.getMinChannels()));
    par.setMinVoxels(parset.getInt16("minVoxels", par.getMinVoxels()));
    par.setMaxPix(parset.getInt16("maxPix", par.getMaxPix()));
    par.setMaxChannels(parset.getInt16("maxChannels", par.getMaxChannels()));
    par.setMaxVoxels(parset.getInt16("maxVoxels", par.getMaxVoxels()));
    par.setFlagRejectBeforeMerge(parset.getBool("flagRejectBeforeMerge",
                                 par.getFlagRejectBeforeMerge()));
    par.setFlagTwoStageMerging(parset.getBool("flagTwoStageMerging",
                               par.getFlagTwoStageMerging()));

    //
    par.setSpectralUnits(parset.getString("spectralUnits", par.getSpectralUnits()));
    par.setSpectralType(parset.getString("spectralType", par.getSpectralType()));
    par.setRestFrequency(parset.getFloat("restFrequency", par.getRestFrequency()));

    par.setVerbosity(parset.getBool("verbose", false));
    par.setDrawBorders(parset.getBool("drawBorders", par.drawBorders()));
    checkUnusedParameter(parset, "drawBlankEdges"); // No graphics
    par.setPixelCentre(parset.getString("pixelCentre", "centroid"));
    checkUnusedParameter(parset, "spectralMethod"); // only used for graphical output.
    par.setSortingParam(parset.getString("sortingParam", "ra"));

    par.checkPars();

    // The next bit ensures that the output mask is put in the current directory
    std::string maskfileRequested = par.outputMaskFile();
    size_t loc = maskfileRequested.rfind('/');
    std::string maskfileUsed = (loc != std::string::npos) ?
                               maskfileRequested.substr(loc + 1, maskfileRequested.size()) :
                               maskfileRequested;
    if (maskfileRequested != maskfileUsed) {
        ASKAPLOG_INFO_STR(logger, "Changing the mask output file from " <<
                          maskfileRequested << " to " << maskfileUsed);
    }
    par.setFileOutputMask(maskfileUsed);

    return par;
}


std::string objectToSubsection(duchamp::Detection *object, long padding,
                               std::string imageName, duchamp::FitsHeader &header)
{

    std::vector<size_t> dim = analysisutilities::getCASAdimensions(imageName);
    const int lng = header.getWCS()->lng;
    const int lat = header.getWCS()->lat;
    const int spec = header.getWCS()->spec;

    ASKAPLOG_DEBUG_STR(logger, "Image dim size = " << dim.size());
    ASKAPLOG_DEBUG_STR(logger, "Image dim = " << dim[0] << " " << dim[1] <<
                       " " << dim[2] << " " << dim[3]);
    ASKAPLOG_DEBUG_STR(logger, object->getXmin() << " " << object->getYmin() <<
                       " " << object->getZmin() << "   " << padding);
    long zero = 0;
    size_t xmin = size_t(std::max(zero, object->getXmin() - padding));
    size_t ymin = size_t(std::max(zero, object->getYmin() - padding));
    size_t zmin = 0;
    size_t xmax = std::min(dim[0] - 1, size_t(object->getXmax() + padding));
    size_t ymax = std::min(dim[1] - 1, size_t(object->getYmax() + padding));
    size_t zmax = dim[spec] - 1;

    std::stringstream subsectionString;
    subsectionString << "[";
    for (int i = 0; i < int(dim.size()); i++) {
        if (i == lng) {
            subsectionString << xmin + 1 << ":" << xmax + 1;
        } else if (i == lat) {
            subsectionString << ymin + 1 << ":" << ymax + 1;
        } else if (i == spec) {
            subsectionString << zmin + 1 << ":" << zmax + 1;
        } else {
            subsectionString << "*";
        }
        if (i < int(dim.size() - 1)) {
            subsectionString << ",";
        }
    }
    subsectionString << "]";

    return subsectionString.str();

}


}
}
