/// @file
///
/// Provides base class for handling the creation of FITS files
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
#include <askap_simulations.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <FITS/FITSfile.h>
#include <simulationutilities/SimulationUtilities.h>
#include <simulationutilities/FluxGenerator.h>

#include <modelcomponents/Continuum.h>
#include <modelcomponents/ContinuumNVSS.h>
#include <modelcomponents/ContinuumS3SEX.h>
#include <modelcomponents/ContinuumSelavy.h>
#include <modelcomponents/FullStokesContinuum.h>
#include <modelcomponents/HIprofile.h>
#include <modelcomponents/HIprofileS3SEX.h>
#include <modelcomponents/HIprofileS3SAX.h>
#include <modelcomponents/GaussianProfile.h>
#include <modelcomponents/FLASHProfile.h>
#include <modelcomponents/ModelFactory.h>
#include <modelcomponents/BeamCorrector.h>
#include <coordutils/PositionUtilities.h>
#include <coordutils/SpectralUtilities.h>
#include <casainterface/CasaInterface.h>
#include <mathsutils/MathsUtils.h>

#include <Common/ParameterSet.h>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/pointer_cast.hpp>

#include <duchamp/Utils/Section.hh>

#include <scimath/Functionals/Gaussian1D.h>
#include <scimath/Functionals/Gaussian2D.h>
#include <scimath/Functionals/Gaussian3D.h>
#include <casa/namespace.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/Unit.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <images/Images/PagedImage.h>
#include <images/Images/ImageInfo.h>
#include <casa/Arrays/ArrayBase.h>

#include <gsl/gsl_multifit.h>

#include <wcslib/wcs.h>
#include <wcslib/wcshdr.h>
#include <wcslib/wcsunits.h>
#include <wcslib/wcsfix.h>
#define WCSLIB_GETWCSTAB // define this so that we don't try to redefine wtbarr
// (this is a problem when using wcslib-4.2)
#include <fitsio.h>
#include <duchamp/duchamp.hh>
#include <duchamp/Utils/utils.hh>
#include <duchamp/Utils/GaussSmooth2D.hh>

#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <utility>
#include <string>
#include <math.h>

ASKAP_LOGGER(logger, ".fitsfile");
using namespace askap::analysisutilities;

namespace askap {

namespace simulations {

namespace FITS {

FITSfile::FITSfile()
{
    this->itsWCSAllocated = false;
    this->itsWCSsourcesAllocated = false;
    this->itsCreateTaylorTerms = false;
    this->itsWriteFullImage = true;
}

//--------------------------------------------------------

FITSfile::~FITSfile()
{
    int nwcs = 1;

    if (this->itsWCSAllocated) {
        wcsvfree(&nwcs, &this->itsWCS);
    }

    if (this->itsWCSsourcesAllocated) {
        wcsvfree(&nwcs, &this->itsWCSsources);
    }

}


FITSfile::FITSfile(const FITSfile &f)
{
    operator=(f);
}

FITSfile& FITSfile::operator=(const FITSfile &f)
{
    if (this == &f) return *this;

    this->itsFileName = f.itsFileName;
    this->itsFITSOutput = f.itsFITSOutput;
    this->itsCasaOutput = f.itsCasaOutput;
    this->itsFlagWriteByChannel = f.itsFlagWriteByChannel;
    this->itsWriteFullImage = f.itsWriteFullImage;
    this->itsCreateTaylorTerms = f.itsCreateTaylorTerms;
    this->itsMaxTaylorTerm = f.itsMaxTaylorTerm;
    this->itsTTmaps = f.itsTTmaps;
    this->itsTTlogevery = f.itsTTlogevery;
    this->itsSourceList = f.itsSourceList;
    this->itsSourceListType = f.itsSourceListType;
    this->itsSourceLogevery = f.itsSourceLogevery;
    this->itsDatabaseOrigin = f.itsDatabaseOrigin;
    this->itsFlagVerboseSources = f.itsFlagVerboseSources;
    this->itsModelFactory = f.itsModelFactory;
    this->itsPosType = f.itsPosType;
    this->itsMinMinorAxis = f.itsMinMinorAxis;
    this->itsPAunits = f.itsPAunits;
    this->itsSourceFluxUnits = f.itsSourceFluxUnits;
    this->itsAxisUnits = f.itsAxisUnits;
    this->itsFlagIntegrateGaussians = f.itsFlagIntegrateGaussians;
    this->itsNumPix = f.itsNumPix;

    this->itsArray = f.itsArray;

    this->itsNoiseRMS = f.itsNoiseRMS;
    this->itsDim = f.itsDim;
    this->itsAxes = f.itsAxes;
    this->itsSourceSection = f.itsSourceSection;
    this->itsHaveBeam = f.itsHaveBeam;
    this->itsBeamInfo = f.itsBeamInfo;
    this->itsBeamCorrector = f.itsBeamCorrector;
    this->itsBaseFreq = f.itsBaseFreq;
    this->itsRestFreq = f.itsRestFreq;
    this->itsAddSources = f.itsAddSources;
    this->itsDryRun = f.itsDryRun;
    this->itsEquinox = f.itsEquinox;
    this->itsBunit = f.itsBunit;
    this->itsUnitScl = f.itsUnitScl;
    this->itsUnitOff = f.itsUnitOff;
    this->itsUnitPwr = f.itsUnitPwr;

    int nwcs = 1;

    if (this->itsWCSAllocated) {
        wcsvfree(&nwcs, &this->itsWCS);
    }

    this->itsWCSAllocated = f.itsWCSAllocated;

    if (this->itsWCSAllocated) {
        this->itsWCS = (struct wcsprm *)calloc(1, sizeof(struct wcsprm));
        this->itsWCSAllocated = true;
        this->itsWCS->flag     = -1;
        wcsini(true, f.itsWCS->naxis, this->itsWCS);
        wcscopy(true, f.itsWCS, this->itsWCS);
        wcsset(this->itsWCS);
    }

    this->itsFlagPrecess = f.itsFlagPrecess;

    if (this->itsWCSsourcesAllocated) {
        wcsvfree(&nwcs, &this->itsWCSsources);
    }

    this->itsWCSsourcesAllocated = f.itsWCSsourcesAllocated;

    if (this->itsFlagPrecess) {
        if (this->itsWCSsourcesAllocated) {
            this->itsWCSsources = (struct wcsprm *)calloc(1, sizeof(struct wcsprm));
            this->itsWCSsourcesAllocated = true;
            this->itsWCSsources->flag     = -1;
            wcsini(true, f.itsWCSsources->naxis, this->itsWCSsources);
            wcscopy(true, f.itsWCSsources, this->itsWCSsources);
            wcsset(this->itsWCSsources);
        }
    }

    this->itsFlagOutputList = f.itsFlagOutputList;
    this->itsFlagOutputListGoodOnly = f.itsFlagOutputListGoodOnly;
    this->itsOutputSourceList = f.itsOutputSourceList;

    return *this;

}

//--------------------------------------------------------

FITSfile::FITSfile(const LOFAR::ParameterSet& parset, bool allocateMemory)
{

    ASKAPLOG_DEBUG_STR(logger, "Defining the FITSfile");
    itsFileName = parset.getString("filename", "");
    itsFITSOutput = parset.getBool("fitsOutput", true);
    itsCasaOutput = parset.getBool("casaOutput", false);
    itsFlagWriteByChannel = parset.getBool("flagWriteByChannel", false);
    itsWriteFullImage = parset.getBool("writeFullImage", true);
    itsCreateTaylorTerms = parset.getBool("createTaylorTerms", false);
    itsMaxTaylorTerm = parset.getInt16("maxTaylorTerm", 2);
    itsTTmaps = std::vector<casa::Array<Float> >(itsMaxTaylorTerm + 1);
    itsTTlogevery = parset.getInt32("TTlogevery", 10);
    ASKAPLOG_DEBUG_STR(logger, "createTaylorTerms=" << itsCreateTaylorTerms <<
                       ", maxTaylorTerm=" << itsMaxTaylorTerm);

    itsBunit = casa::Unit(parset.getString("bunit", "Jy/beam"));
    itsSourceList = parset.getString("sourcelist", "");
    std::ifstream file;
    file.open(itsSourceList.c_str(), std::ifstream::in);
    file.close();

    if (file.fail()) {
        ASKAPTHROW(AskapError,
                   "Source list " << itsSourceList << " could not be opened. Exiting.");
    }

    itsSourceListType = parset.getString("sourcelisttype", "continuum");

    if (itsSourceListType != "continuum" && itsSourceListType != "spectralline") {
        itsSourceListType = "continuum";
        ASKAPLOG_WARN_STR(logger, "Input parameter sourcelisttype needs to be " <<
                          "*either* 'continuum' or 'spectralline'. Setting to 'continuum'.");
    }

    itsAddSources = parset.getBool("addSources", true);
    itsDryRun = parset.getBool("dryRun", false);
    itsSourceLogevery = parset.getInt32("sourceLogevery", 1000);

    itsModelFactory = ModelFactory(parset);
    itsDatabaseOrigin = parset.getString("database", "Continuum");
    if (!itsModelFactory.checkType()) {
        ASKAPLOG_ERROR_STR(logger, "Input parameter databaseorigin (" <<
                           itsDatabaseOrigin << ") not a valid type.");
    }
    ASKAPLOG_DEBUG_STR(logger, "database origin = " << itsDatabaseOrigin);
    itsUseGaussians = true;
    if (itsDatabaseOrigin == "POSSUM" || itsDatabaseOrigin == "POSSUMHI") {
        itsUseGaussians = parset.getBool("useGaussians", false);
        if (itsUseGaussians) {
            ASKAPLOG_DEBUG_STR(logger, "Expressing disc components as 2D gaussians");
        }        else {
            ASKAPLOG_DEBUG_STR(logger, "Leaving disc components as discs");
        }
    }
    if (this->databaseSpectral()) {
        itsSourceListType = "spectralline";
    }
    ASKAPLOG_DEBUG_STR(logger, "source list type = " << itsSourceListType);
    itsFlagVerboseSources = parset.getBool("verboseSources", true);

    itsPosType = parset.getString("posType", "dms");
    itsMinMinorAxis = parset.getFloat("minMinorAxis", 0.);
    itsPAunits = casa::Unit(parset.getString("PAunits", "rad"));
    if (itsPAunits != "rad" && itsPAunits != "deg") {
        ASKAPLOG_WARN_STR(logger, "Input parameter PAunits needs to be " <<
                          "*either* 'rad' *or* 'deg'. Setting to rad.");
        itsPAunits = "rad";
    }
    if (itsDatabaseOrigin == "Selavy" && itsPAunits != "deg") {
        if (parset.isDefined("PAunits")) {
            ASKAPLOG_WARN_STR(logger, "With Selavy, PAunits must be 'deg'.");
        }
        itsPAunits = "deg";
    }

    itsFlagIntegrateGaussians = parset.getBool("integrateGaussians", true);
    // For the Selavy case, we want to default to false, unless specified in the parset.
    if (itsDatabaseOrigin == "Selavy" && !parset.isDefined("integrateGaussians")) {
        itsFlagIntegrateGaussians = false;
    }

    itsAxisUnits = casa::Unit(parset.getString("axisUnits", "arcsec"));
    itsSourceFluxUnits = casa::Unit(parset.getString("sourceFluxUnits", ""));

    if (itsSourceFluxUnits != "") {
        char *base = (char *)itsBunit.getName().c_str();
        wcsutrn(0, base);
        char *src = (char *)itsSourceFluxUnits.getName().c_str();
        wcsutrn(0, src);
        int status = wcsunits(src, base,
                              &itsUnitScl, &itsUnitOff, &itsUnitPwr);

        if (status) {
            ASKAPTHROW(AskapError, "The parameters bunit (\"" << base
                       << "\") and sourceFluxUnits (\"" << src
                       << "\") are not interconvertible.");
        }
        ASKAPLOG_INFO_STR(logger, "Converting from " << src << " to " << base <<
                          ": " << itsUnitScl <<
                          "," << itsUnitOff <<
                          "," << itsUnitPwr);
    } else {
        itsSourceFluxUnits = itsBunit;
        itsUnitScl = 1.;
        itsUnitOff = 0.;
        itsUnitPwr = 1.;
    }

    itsNoiseRMS = parset.getFloat("noiserms", 0.001);

    itsDim = parset.getUint16("dim", 2);
    itsAxes = parset.getUint32Vector("axes");
    std::string sectionString = parset.getString("subsection",
                                duchamp::nullSection(itsDim));
    itsSourceSection.setSection(sectionString);
    std::vector<int> axes(itsDim);

    for (uint i = 0; i < itsDim; i++) axes[i] = itsAxes[i];

    itsSourceSection.parse(axes);

    if (itsAxes.size() != itsDim) {
        ASKAPTHROW(AskapError, "Dimension mismatch: dim = " << itsDim
                   << ", but axes has " << itsAxes.size() << " dimensions.");
    }
    for (uint i = 0; i < itsDim; i++) {
        itsAxes[i] = itsSourceSection.getDim(i);
    }

    std::stringstream ss;
    itsNumPix = itsAxes[0];
    ss << itsAxes[0];

    for (uint i = 1; i < itsDim; i++) {
        itsNumPix *= itsAxes[i];
        ss << "x" << itsAxes[i];
    }


    itsHaveBeam = parset.isDefined("beam");
    if (itsHaveBeam) {
        itsBeamInfo = parset.getFloatVector("beam");
    }

    if (parset.getBool("correctForBeam", false) &&
            !parset.getBool("useDeconvolvedSizes", false)) {
        itsBeamCorrector = BeamCorrector(parset.makeSubset("correctForBeam."));
        if (!itsHaveBeam) {
            itsBeamInfo = itsBeamCorrector.beam();
            itsHaveBeam = true;
        }
    }

    if (itsHaveBeam) {
        ASKAPLOG_DEBUG_STR(logger, "Using beam " << itsBeamInfo[0] << " " <<
                           itsBeamInfo[1] << " " << itsBeamInfo[2]);
    } else {
        ASKAPLOG_DEBUG_STR(logger, "No beam used");
    }

    itsEquinox = parset.getFloat("equinox", 2000.);
    itsRestFreq = parset.getFloat("restFreq", -1.);
    if (itsRestFreq > 0.) {
        ASKAPLOG_DEBUG_STR(logger, "Rest freq = " << itsRestFreq);
    }

    LOFAR::ParameterSet subset(parset.makeSubset("WCSimage."));
    itsWCSAllocated = false;
    this->setWCS(true, subset);
    itsFlagPrecess = parset.getBool("WCSsources", false);
    itsWCSsourcesAllocated = false;

    if (itsFlagPrecess) {
        LOFAR::ParameterSet subset(parset.makeSubset("WCSsources."));
        this->setWCS(false, subset);
    }

    itsBaseFreq = parset.getFloat("baseFreq", itsWCS->crval[itsWCS->spec]);
    ASKAPLOG_DEBUG_STR(logger, "Base freq = " << itsBaseFreq);

    if (itsDryRun) {
        itsFITSOutput = false;
        itsCasaOutput = false;
        ASKAPLOG_INFO_STR(logger, "Just a DRY RUN - no sources being added or images created.");
    }

    itsFlagOutputList = parset.getBool("outputList", false);
    itsFlagOutputListGoodOnly = parset.getBool("outputListGoodOnly", false);

    if (itsSourceList.size() == 0) {
        itsFlagOutputList = false;
    }

    itsOutputSourceList = parset.getString("outputSourceList", "");

    if (allocateMemory && !itsDryRun) {
        ASKAPLOG_DEBUG_STR(logger, "Allocating array of dimensions " << ss.str() <<
                           " with " << itsNumPix << " pixels, each of size " <<
                           sizeof(float) << " bytes, for total size of " <<
                           itsNumPix * sizeof(float) / 1024. / 1024. / 1024. << "GB");
        itsArray = std::vector<float>(itsNumPix, 0.);
        ASKAPLOG_DEBUG_STR(logger, "Allocation done.");

    }

    ASKAPLOG_DEBUG_STR(logger, "FITSfile defined.");
}

//--------------------------------------------------------

bool FITSfile::databaseSpectral()
{
    bool val = ((itsDatabaseOrigin == "S3SEX" &&
                 itsSourceListType == "spectralline") ||
                itsDatabaseOrigin == "S3SAX" ||
                itsDatabaseOrigin == "Gaussian" ||
                itsDatabaseOrigin == "FLASH");
    return val;
}

//--------------------------------------------------------

int FITSfile::getNumStokes()
{
    // first find which axis is the STOKES axis. Return its dimension,
    // or 1 if there isn't one.
    bool haveStokes = false;
    unsigned int stokesAxis = -1;
    for (unsigned int i = 0; i < itsDim && !haveStokes; i++) {
        haveStokes = (std::string(itsWCS->ctype[i]) == "STOKES");
        if (haveStokes) {
            stokesAxis = i;
        }
    }

    int returnVal;
    if (haveStokes) {
        returnVal = itsAxes[stokesAxis];
    } else {
        returnVal = 1;
    }
    return returnVal;
}


//--------------------------------------------------------

size_t FITSfile::getNumChan()
{
    size_t val;
    if (this->getSpectralAxisIndex() > 0) {
        val =  itsAxes[this->getSpectralAxisIndex()];
    } else val = 1;
    return val;
}

//--------------------------------------------------------

void FITSfile::setWCS(bool isImage, const LOFAR::ParameterSet& parset)
{
    int stat[NWCSFIX];
    int axes[itsAxes.size()];

    for (uint i = 0; i < itsAxes.size(); i++) axes[i] = itsAxes[i];

    int nwcs = 1;
    struct wcsprm *wcs;

    if (isImage) {
        if (itsWCSAllocated) {
            wcsvfree(&nwcs, &itsWCS);
        }
        wcs = parsetToWCS(parset, itsAxes, itsEquinox,
                          itsRestFreq, itsSourceSection);
        itsWCS = (struct wcsprm *)calloc(1, sizeof(struct wcsprm));
        itsWCSAllocated = true;
        itsWCS->flag = -1;
        wcsini(true, wcs->naxis, itsWCS);
        wcsfix(1, (const int*)axes, wcs, stat);
        wcscopy(true, wcs, itsWCS);
        wcsset(itsWCS);
    } else {
        if (itsWCSsourcesAllocated) {
            wcsvfree(&nwcs, &itsWCSsources);
        }
        wcs = parsetToWCS(parset, itsAxes, itsEquinox,
                          itsRestFreq, itsSourceSection);
        itsWCSsources = (struct wcsprm *)calloc(1, sizeof(struct wcsprm));
        itsWCSsourcesAllocated = true;
        itsWCSsources->flag = -1;
        wcsini(true, wcs->naxis, itsWCSsources);
        wcsfix(1, (const int*)axes, wcs, stat);
        wcscopy(true, wcs, itsWCSsources);
        wcsset(itsWCSsources);
    }

    wcsvfree(&nwcs, &wcs);

}

//--------------------------------------------------------

void FITSfile::makeNoiseArray()
{
    if (itsArray.size() > 0) {
        ASKAPLOG_DEBUG_STR(logger, "Making the noise array");

        for (size_t i = 0; i < itsNumPix; i++) {
            itsArray[i] = analysisutilities::normalRandomVariable(0., itsNoiseRMS);
        }
    }
}

//--------------------------------------------------------

void FITSfile::addNoise()
{
    if (itsArray.size() > 0) {
        ASKAPLOG_DEBUG_STR(logger, "Adding noise");

        for (size_t i = 0; i < itsNumPix; i++) {
            itsArray[i] += normalRandomVariable(0., itsNoiseRMS);
        }
    }
}

//--------------------------------------------------------

void FITSfile::processSources()
{

    if (itsSourceList.size() > 0) { // if the source list is defined.
        ASKAPLOG_DEBUG_STR(logger, "Adding sources from file " << itsSourceList);
        std::ifstream srclist(itsSourceList.c_str());
        std::string line;
        std::vector<double> wld(3);
        std::vector<double> pix(3);
        std::vector<double> newwld(3);
        std::ofstream outfile;

        int countLines = 0, countAdded = 0;
        int countGauss = 0, countPoint = 0, countMiss = 0, countDud = 0;

        boost::shared_ptr<Spectrum> src;

        FluxGenerator fluxGen(this->getNumChan(), this->getNumStokes());
        ASKAPLOG_DEBUG_STR(logger, "Defining flux generator with " << fluxGen.nChan() <<
                           " channels and " << fluxGen.nStokes() << " Stokes parameters");

        casa::Gaussian2D<casa::Double> gauss;
        analysisutilities::Disc disc;
        const double arcsecToPixel = 3600. * sqrt(fabs(itsWCS->cdelt[0] * itsWCS->cdelt[1]));

        if (itsFlagOutputList) {
            outfile.open(itsOutputSourceList.c_str(), std::ios::app);
        }

        while (getline(srclist, line),
                !srclist.eof()) {
//          ASKAPLOG_DEBUG_STR(logger, "input = " << line);

            fluxGen.zero();

            if (line[0] == '#') {
                // Write all commented lines directly into the output file
                if (itsFlagOutputList) {
                    outfile << line << "\n";
                }

            } else {
                // ignore commented lines

                countLines++;

                src = itsModelFactory.read(line);

                // convert fluxes to correct units according to the image BUNIT keyword
                casa::Quantity flux0(src->fluxZero(), itsSourceFluxUnits);
                src->setFluxZero(flux0.getValue(itsBunit));
                casa::Quantity maj(src->maj(), itsAxisUnits);
                src->setMaj(maj.getValue("arcsec") / arcsecToPixel);
                casa::Quantity min;
                if (src->maj() > 0 && !(src->min() > itsMinMinorAxis)) {
                    min = casa::Quantity(itsMinMinorAxis, itsAxisUnits);
                } else {
                    min = casa::Quantity(src->min(), itsAxisUnits);
                }
                src->setMin(min.getValue("arcsec") / arcsecToPixel);
                casa::Quantity pa(src->pa(), itsPAunits);

                // convert sky position to pixels
                if (itsPosType == "dms") {
                    wld[0] = analysisutilities::dmsToDec(src->ra()) * 15.;
                    wld[1] = analysisutilities::dmsToDec(src->dec());
                } else if (itsPosType == "deg") {
                    wld[0] = atof(src->ra().c_str());
                    wld[1] = atof(src->dec().c_str());
                } else {
                    ASKAPLOG_ERROR_STR(logger, "Incorrect position type: " << itsPosType);
                }

                if (itsDim > 2) {
                    wld[2] = itsBaseFreq;
                } else {
                    wld[2] = 0.;
                }

                if (itsFlagPrecess) {
                    wcsToPixSingle(itsWCSsources, wld.data(), pix.data());
                } else {
                    wcsToPixSingle(itsWCS, wld.data(), pix.data());
                }

                if (itsFlagOutputList) {
                    pixToWCSSingle(itsWCS, pix.data(), newwld.data());

                    if (!itsFlagOutputListGoodOnly) {
                        if (itsPosType == "dms") {
                            src->print(outfile,
                                       analysisutilities::decToDMS(newwld[0], "RA"),
                                       analysisutilities::decToDMS(newwld[1], "DEC"));
                        } else {
                            src->print(outfile, newwld[0], newwld[1]);
                        }
                    }
                }

                bool lookAtSource = (itsArray.size() > 0 && itsAddSources) || itsDryRun;

                ComponentType sourceType = src->type();

                if (sourceType == POINT) {
                    lookAtSource = lookAtSource && doAddPointSource(itsAxes, pix);
                } else if (sourceType == GAUSSIAN || itsUseGaussians) {

                    if (src->fluxZero() == 0.) {
                        src->setFluxZero(1.e-99);
                    }

                    gauss.setXcenter(pix[0]);
                    gauss.setYcenter(pix[1]);
                    // need this so that we never have the minor axis > major axis
                    gauss.setMinorAxis(std::min(gauss.majorAxis(), src->maj()));
                    gauss.setMajorAxis(src->maj());
                    gauss.setMinorAxis(src->min());
                    gauss.setPA(pa.getValue("rad"));
                    gauss.setFlux(src->fluxZero());

                    lookAtSource = lookAtSource && doAddGaussian(itsAxes, gauss);
                } else if (sourceType == DISC) {
                    disc.setup(pix[0], pix[1], src->maj(), src->min(), pa.getValue("rad"));
                    lookAtSource = lookAtSource && doAddDisc(itsAxes, disc);
                }

                lookAtSource = lookAtSource &&
                               src->freqRangeOK(this->minFreq(), this->maxFreq());

                if (lookAtSource) {

                    src->prepareForUse();

                    if (this->databaseSpectral() && itsDatabaseOrigin != "Gaussian") {
                        fluxGen.addSpectrumInt(src, pix[0], pix[1], itsWCS);
                    } else {
                        fluxGen.addSpectrum(src, pix[0], pix[1], itsWCS);
                    }

                    boost::shared_ptr<FullStokesContinuum> pol;
                    if (itsDatabaseOrigin == "POSSUM") {
                        pol = boost::shared_ptr<FullStokesContinuum>(
                                  boost::dynamic_pointer_cast<FullStokesContinuum>(src));
                    }

                    bool addedSource = false;
                    if (itsFlagVerboseSources && sourceType != POINT) {
                        ASKAPLOG_DEBUG_STR(logger, "Source " << src->id() <<
                                           " has axes " << src->maj() << " x "
                                           << src->min() << " pix");
                    }

                    if (sourceType == POINT) {
                        if (!itsDryRun) {
                            addedSource = addPointSource(itsArray,
                                                         itsAxes,
                                                         pix,
                                                         fluxGen,
                                                         itsFlagVerboseSources);
                        } else {
                            addedSource = doAddPointSource(itsAxes, pix);
                            if (addedSource) {
                                countPoint++;
                                if (itsDatabaseOrigin == "POSSUM") {
                                    if (itsFlagVerboseSources) {
                                        ASKAPLOG_DEBUG_STR(logger, "Point Source at RA=" <<
                                                           src->ra() << ", Dec=" <<
                                                           src->dec() << ", angle=" <<
                                                           pol->polAngle());
                                    }
                                }
                            } else countMiss++;
                        }
                    } else if (sourceType == GAUSSIAN || itsUseGaussians) {
                        if (!itsDryRun) {
                            addedSource = addGaussian(itsArray,
                                                      itsAxes,
                                                      gauss, fluxGen,
                                                      itsFlagIntegrateGaussians,
                                                      itsFlagVerboseSources);
                        } else {
                            addedSource = doAddGaussian(itsAxes, gauss);
                            if (addedSource) {
                                countGauss++;
                                if (itsDatabaseOrigin == "POSSUM") {
                                    if (itsFlagVerboseSources) {
                                        ASKAPLOG_DEBUG_STR(logger, "Gaussian Source at RA=" <<
                                                           src->ra() << ", Dec=" <<
                                                           src->dec() << ", angle=" <<
                                                           pol->polAngle());
                                    }
                                }
                            } else countMiss++;
                        }
                    } else if (sourceType == DISC) {
                        if (!itsDryRun) {
                            addedSource = addDisc(itsArray, itsAxes, disc,
                                                  fluxGen, itsFlagVerboseSources);
                        } else {
                            addedSource = doAddDisc(itsAxes, disc);
                            if (addedSource) {
                                countPoint++;
                                if (itsDatabaseOrigin == "POSSUM") {
                                    if (itsFlagVerboseSources) {
                                        ASKAPLOG_DEBUG_STR(logger, "Point Source at RA=" <<
                                                           src->ra() << ", Dec=" <<
                                                           src->dec() << ", angle=" <<
                                                           pol->polAngle());
                                    }
                                }
                            } else countMiss++;
                        }
                    }

                    if (addedSource) {
                        if (itsFlagOutputList &&
                                itsFlagOutputListGoodOnly &&
                                doAddPointSource(itsAxes, pix)) {
                            if (itsPosType == "dms")
                                src->print(outfile,
                                           analysisutilities::decToDMS(newwld[0], "RA"),
                                           analysisutilities::decToDMS(newwld[1], "DEC"));
                            else
                                src->print(outfile, newwld[0], newwld[1]);
                        }

                        countAdded++;


                    }


                } else {
                    if (itsDryRun) countDud++;
                }

                if (countLines % itsSourceLogevery == 0) {
                    ASKAPLOG_INFO_STR(logger, "Read " << countLines <<
                                      " sources and have added " << countAdded <<
                                      " to the image");
                }

            }

        }

        if (itsFlagOutputList) {
            outfile.close();
        }

        srclist.close();

        if (itsDryRun) {
            ASKAPLOG_INFO_STR(logger, "Would add " << countPoint <<
                              " point sources and " << countGauss <<
                              " Gaussians, with " << countMiss <<
                              " misses and " << countDud << " duds");
        }

        ASKAPLOG_DEBUG_STR(logger, "Finished adding sources");

    }
}


//--------------------------------------------------------

void FITSfile::convolveWithBeam()
{
    if (!itsHaveBeam) {
        ASKAPLOG_WARN_STR(logger,
                          "Cannot convolve with beam as the beam was not " <<
                          "specified in the parset.");
    } else {
        ASKAPLOG_DEBUG_STR(logger, "Convolving with the beam");
        float maj = itsBeamInfo[0] / fabs(itsWCS->cdelt[0]);
        float min = itsBeamInfo[1] / fabs(itsWCS->cdelt[1]);
        float pa = itsBeamInfo[2];
        GaussSmooth2D<float> smoother(maj, min, pa);
        ASKAPLOG_DEBUG_STR(logger, "Defined the smoother with beam=(" << maj << ","
                           << min << "," << pa << "), now to do the smoothing");
        ASKAPLOG_DEBUG_STR(logger, "Smoothing kernel width = " << smoother.getKernelWidth() <<
                           ", stddev scale = " << smoother.getStddevScale());

        float scaleFactor = 1.;
        if (itsBunit.getName() == "Jy/beam") {
            duchamp::Beam beam(maj, min, pa);
            scaleFactor = 1. / beam.area();
            ASKAPLOG_DEBUG_STR(logger, "Since bunit=" << itsBunit.getName() <<
                               " we scale by the area of the beam, which is " << scaleFactor);
        }

        ASKAPASSERT(itsDim <= 4);
        size_t xySize = itsAxes[0] * itsAxes[1];
        std::vector<float> image(xySize);
        int specdim = (itsDim > 2) ? itsAxes[2] : 1;
        int stokesdim = (itsDim > 3) ? itsAxes[3] : 1;
        for (int z = 0; z < specdim; z++) {
            for (int j = 0; j < stokesdim; j++) {
                for (size_t pix = 0; pix < xySize; pix++) {
                    image[pix] = itsArray[z * xySize + pix + j * specdim * xySize];
                }
                boost::scoped_ptr<float>
                newArray(smoother.smooth(image.data(), itsAxes[0],
                                         itsAxes[1], SCALEBYCOVERAGE));

                for (size_t pix = 0; pix < xySize; pix++) {
                    itsArray[z * xySize + pix + j * specdim * xySize] =
                        newArray.get()[pix] / scaleFactor;
                }
            }
        }

        ASKAPLOG_DEBUG_STR(logger, "Convolving done.");

    }
}


//--------------------------------------------------------

char *numerateKeyword(std::string key, int num)
{
    std::stringstream ss;
    ss << key << num;
    return (char *)ss.str().c_str();
}

//--------------------------------------------------------

void FITSfile::writeFITSimage(bool createFile, bool saveData, bool useOffset)
{

    if (itsFITSOutput) {

        ASKAPLOG_INFO_STR(logger, "Saving the FITS file to " << itsFileName);


        int status = 0;

        fitsfile *fptr;

        if (createFile) {
            ASKAPLOG_INFO_STR(logger, "Creating the FITS file");

            if (fits_create_file(&fptr, itsFileName.c_str(), &status)) {
                ASKAPLOG_ERROR_STR(logger, "Error opening FITS file:");
                fits_report_error(stderr, status);
                ASKAPTHROW(AskapError, "Error opening FITS file.");
            }

            status = 0;
            std::vector<long> dim(itsDim);

            for (uint i = 0; i < itsDim; i++) dim[i] = itsAxes[i];

            if (fits_create_img(fptr, FLOAT_IMG, itsDim, dim.data(), &status)) {
                ASKAPLOG_ERROR_STR(logger, "Error creating the FITS image:");
                fits_report_error(stderr, status);
            }

            status = 0;

            std::string header = "EQUINOX";

            if (fits_update_key(fptr, TFLOAT, (char *)header.c_str(),
                                &(itsEquinox), NULL, &status)) {
                fits_report_error(stderr, status);
            }

            if (itsHaveBeam) {
                status = 0;

                header = "BMAJ";

                if (fits_update_key(fptr, TFLOAT, (char *)header.c_str(),
                                    &(itsBeamInfo[0]), NULL, &status)) {
                    fits_report_error(stderr, status);
                }

                status = 0;

                header = "BMIN";

                if (fits_update_key(fptr, TFLOAT, (char *)header.c_str(),
                                    &(itsBeamInfo[1]), NULL, &status)) {
                    fits_report_error(stderr, status);
                }

                status = 0;

                header = "BPA";

                if (fits_update_key(fptr, TFLOAT, (char *)header.c_str(),
                                    &(itsBeamInfo[2]), NULL, &status)) {
                    fits_report_error(stderr, status);
                }
            }

            status = 0;

            char *unit = (char *)itsBunit.getName().c_str();

            header = "BUNIT";

            if (fits_update_key(fptr, TSTRING, (char *)header.c_str(),
                                unit,  NULL, &status)) {
                fits_report_error(stderr, status);
            }

            if ((itsSourceListType == "spectralline") && (itsRestFreq > 0.)) {
                status = 0;

                header = "RESTFREQ";

                if (fits_update_key(fptr, TFLOAT, (char *)header.c_str(),
                                    &itsRestFreq, NULL, &status)) {
                    fits_report_error(stderr, status);
                }
            }

            float val;

            for (uint d = 0; d < itsDim; d++) {
                status = 0;

                if (fits_update_key(fptr, TSTRING, numerateKeyword("CTYPE", d + 1),
                                    itsWCS->ctype[d],  NULL, &status)) {
                    fits_report_error(stderr, status);
                }

                status = 0;

                if (fits_update_key(fptr, TSTRING, numerateKeyword("CUNIT", d + 1),
                                    itsWCS->cunit[d],  NULL, &status)) {
                    fits_report_error(stderr, status);
                }

                status = 0;
                val = itsWCS->crval[d];

                if (fits_update_key(fptr, TFLOAT, numerateKeyword("CRVAL", d + 1),
                                    &val, NULL, &status)) {
                    fits_report_error(stderr, status);
                }

                val = itsWCS->cdelt[d];
                status = 0;

                if (fits_update_key(fptr, TFLOAT, numerateKeyword("CDELT", d + 1),
                                    &val, NULL, &status)) {
                    fits_report_error(stderr, status);
                }

                val = itsWCS->crpix[d];
                status = 0;

                if (fits_update_key(fptr, TFLOAT, numerateKeyword("CRPIX", d + 1),
                                    &val, NULL, &status)) {
                    fits_report_error(stderr, status);
                }

                val = itsWCS->crota[d];
                status = 0;

                if (fits_update_key(fptr, TFLOAT, numerateKeyword("CROTA", d + 1),
                                    &val, NULL, &status)) {
                    fits_report_error(stderr, status);
                }
            }

        }

        if (saveData) {

            ASKAPLOG_INFO_STR(logger, "Saving the data to the FITS file");

            if (!createFile) {
                status = 0;
                std::string filename = itsFileName;

                if (filename[0] == '!') {
                    filename = filename.substr(1);
                }

                ASKAPLOG_DEBUG_STR(logger, "Opening " << filename);

                if (fits_open_file(&fptr, filename.c_str(), READWRITE, &status)) {
                    ASKAPLOG_ERROR_STR(logger, "Error opening FITS file:");
                    fits_report_error(stderr, status);
                    ASKAPTHROW(AskapError, "Error opening FITS file.");
                }
            }

            int ndim = 4;
            long axes[ndim];
            fits_get_img_size(fptr, ndim, axes, &status);
            ASKAPLOG_DEBUG_STR(logger, "Image dimensions are " << axes[0] << "x" <<
                               axes[1] << "x" << axes[2] << "x" << axes[3]);

            ASKAPLOG_INFO_STR(logger, "Opened the FITS file, preparing to write data");

            std::vector<long> fpixel(itsDim, 1);
            std::vector<long> lpixel(itsDim);

            for (uint i = 0; i < itsDim; i++) {
                if (useOffset) {
                    fpixel[i] = itsSourceSection.getStart(i) + 1;
                    lpixel[i] = itsSourceSection.getEnd(i) + 1;
                } else {
                    lpixel[i] = itsAxes[i];
                }
            }

            status = 0;

            if (fits_write_subset(fptr, TFLOAT, fpixel.data(),
                                  lpixel.data(), itsArray.data(), &status)) {
                fits_report_error(stderr, status);
            }

        } //end of if(saveData)

        if (saveData || createFile) {
            ASKAPLOG_DEBUG_STR(logger, "Closing fits file");
            status = 0;

            if (fits_close_file(fptr, &status)) {
                ASKAPLOG_ERROR_STR(logger, "Error closing file:");
                fits_report_error(stderr, status);
            }
        }


    }
}

//--------------------------------------------------------

std::string casafy(std::string fitsName)
{

    std::string casaname;
    size_t pos = fitsName.rfind(".fits");

    if (pos == std::string::npos) { // imageName doesn't have a .fits extension
        casaname = fitsName + ".casa";
    } else { // just remove the .fits extension
        casaname = fitsName.substr(0, pos);
    }

    if (casaname[0] == '!') {
        casaname = casaname.substr(1);
    }

    return casaname;
}


//--------------------------------------------------------

void FITSfile::writeCASAimage(bool createFile, bool saveData, bool useOffset)
{

    if (itsCasaOutput) {

        std::string newName = casafy(itsFileName);
        casa::IPosition shape(itsDim);
        casa::IPosition ttshape;

        for (uint i = 0; i < itsDim; i++) shape(i) = itsAxes[i];

        if (createFile) {

            int nstokes = this->getNumStokes();
            ASKAPLOG_DEBUG_STR(logger, "Dimension of stokes axis = " << nstokes <<
                               ", databaseOrigin = " << itsDatabaseOrigin);
            casa::IPosition tileshape(shape.size(), 1);
            tileshape(itsWCS->lng) = std::min(128L, shape(itsWCS->lng));
            tileshape(itsWCS->lat) = std::min(128L, shape(itsWCS->lat));
            if (itsWCS->spec >= 0) {
                tileshape(itsWCS->spec) = std::min(16L, shape(itsWCS->spec));
            }

            casa::CoordinateSystem csys = analysisutilities::wcsToCASAcoord(itsWCS, nstokes);
            casa::ImageInfo ii;

            if (itsHaveBeam) {
                ii.setRestoringBeam(casa::Quantity(itsBeamInfo[0], "deg"),
                                    casa::Quantity(itsBeamInfo[1], "deg"),
                                    casa::Quantity(itsBeamInfo[2], "deg"));
            }

            if (itsWriteFullImage) {

                ASKAPLOG_INFO_STR(logger, "Creating a new CASA image " << newName <<
                                  " with the shape " << shape <<
                                  " and tileshape " << tileshape);
                casa::PagedImage<float> img(casa::TiledShape(shape, tileshape), csys, newName);

                img.setUnits(itsBunit);
                if (itsHaveBeam) {
                    img.setImageInfo(ii);
                }

            }

            if (itsCreateTaylorTerms) {

                tileshape(itsWCS->spec) = 1;
                ttshape = shape;
                ttshape(itsWCS->spec) = 1;
                ASKAPLOG_INFO_STR(logger, "Creating Taylor term images with form " << newName <<
                                  ".taylor.0-" << itsMaxTaylorTerm <<
                                  " with the shape " << ttshape <<
                                  " and tileshape " << tileshape);
                createTaylorTermImages(newName, csys, ttshape, tileshape, itsBunit, ii);

            }

        }

        if (saveData) {

            if (itsArray.size() > 0) {

                casa::IPosition location(itsDim, 0);
                if (useOffset) {
                    for (uint i = 0; i < itsDim; i++) {
                        location(i) = itsSourceSection.getStart(i);
                    }
                }

                if (itsWriteFullImage) {

                    casa::PagedImage<float> img(newName);

                    if (itsFlagWriteByChannel) {
                        shape(itsWCS->spec) = 1;
                        for (size_t z = 0; z < itsAxes[itsWCS->spec]; z++) {
                            size_t spatsize = itsAxes[itsWCS->lat] *
                                              itsAxes[itsWCS->lng];
                            Array<Float> arr(shape, itsArray.data() + z * spatsize, casa::SHARE);
                            img.putSlice(arr, location);
                            location(itsWCS->spec)++;

                        }
                    } else {
                        // make the casa::Array, sharing the memory
                        // storage so there is minimal additional
                        // impact
                        Array<Float> arr(shape, itsArray.data(), casa::SHARE);

                        casa::IPosition location(itsDim, 0);

                        if (useOffset) {
                            for (uint i = 0; i < itsDim; i++) {
                                location(i) = itsSourceSection.getStart(i);
                            }
                        }

                        ASKAPLOG_DEBUG_STR(logger, "shape = " << shape <<
                                           ", location = " << location);
                        ASKAPLOG_INFO_STR(logger, "Writing an array with the shape " <<
                                          arr.shape() << " into a CASA image " <<
                                          newName << " at location " << location);
                        img.putSlice(arr, location);
                    }
                }

                if (itsCreateTaylorTerms) {

                    location(itsWCS->spec) =
                        itsSourceSection.getStart(itsWCS->spec);
                    ASKAPLOG_INFO_STR(logger, "Writing to Taylor term images");
                    writeTaylorTermImages(newName, location);

                }
            } else {
                ASKAPLOG_WARN_STR(logger, "Cannot write array as it has not been allocated");
            }
        }

    }

}


double FITSfile::maxFreq()
{
    int spec = itsWCS->spec;
    return itsWCS->crval[spec] +
           (itsAxes[spec] / 2 + 0.5) * itsWCS->cdelt[spec];
}
double FITSfile::minFreq()
{
    int spec = itsWCS->spec;
    return itsWCS->crval[spec] -
           (itsAxes[spec] / 2 + 0.5) * itsWCS->cdelt[spec];
}


void FITSfile::createTaylorTermImages(std::string nameBase,
                                      casa::CoordinateSystem csys,
                                      casa::IPosition shape,
                                      casa::IPosition tileshape,
                                      casa::Unit bunit,
                                      casa::ImageInfo iinfo)
{


    for (size_t t = 0; t <= itsMaxTaylorTerm; t++) {

        std::stringstream outname;
        outname << nameBase << ".taylor." << t;

        casa::PagedImage<float> outimg(casa::TiledShape(shape, tileshape), csys, outname.str());

        outimg.setUnits(bunit);
        outimg.setImageInfo(iinfo);

    }

}


void FITSfile::defineTaylorTerms()
{

    if (itsArray.size() > 0) {

        ASKAPLOG_INFO_STR(logger, "Calculating taylor term arrays, for terms " <<
                          "up to and including .taylor." << itsMaxTaylorTerm);

        const size_t spec = itsWCS->spec;
        const unsigned int maxterm = 2;
        if (itsMaxTaylorTerm > maxterm) {
            ASKAPLOG_WARN_STR(logger, "A maximum taylor term of " <<
                              itsMaxTaylorTerm <<
                              " was requested. We will only fill terms up to .taylor." <<
                              maxterm);
        }

        casa::IPosition shape(itsDim);
        for (uint i = 0; i < itsDim; i++) {
            shape(i) = itsAxes[i];
        }
        shape(spec) = 1;
        for (size_t i = 0; i <= itsMaxTaylorTerm; i++) {
            itsTTmaps[i] = casa::Array<float>(shape, 0.);
        }
        const size_t ndata = itsAxes[itsWCS->spec];
        const size_t degree = itsMaxTaylorTerm + 3;
        double chisq;
        gsl_matrix *xdat, *cov;
        gsl_vector *ydat, *w, *c;
        xdat = gsl_matrix_alloc(ndata, degree);
        ydat = gsl_vector_alloc(ndata);
        w = gsl_vector_alloc(ndata);
        c = gsl_vector_alloc(degree);
        cov = gsl_matrix_alloc(degree, degree);

        for (size_t i = 0; i < ndata; i++) {
            // Set the frequency values, normalised by the reference frequency nuZero.
            // Note that the fitting is done in log-space (and **NOT** log10-space!!)
            double freq = itsWCS->crval[spec] +
                          (i - itsWCS->crpix[spec]) * itsWCS->cdelt[spec];
            float logfreq = log(freq / itsBaseFreq);
            float xval = 1.;
            for (size_t d = 0; d < degree; d++) {
                gsl_matrix_set(xdat, i, d, xval);
                xval *= logfreq;
            }
            gsl_vector_set(w, i, 1.);
        }

        const size_t xlen = itsAxes[itsWCS->lng];
        const size_t ylen = itsAxes[itsWCS->lat];
        casa::IPosition outpos(shape.size(), 0);
        for (size_t y = 0; y < ylen; y++) {
            outpos[1] = y;

            for (size_t x = 0; x < xlen; x++) {
                outpos[0] = x;

                size_t pos = x + y * xlen;

                if (pos % int(xlen * ylen * itsTTlogevery / 100.) == 0) {
                    ASKAPLOG_INFO_STR(logger, "Found Taylor terms for " << pos <<
                                      " spectra out of " << xlen * ylen <<
                                      " with x=" << x << " and y=" << y);
                }

                if (itsArray[pos] > 1.e-20) {
                    for (size_t i = 0; i < ndata; i++) {
                        gsl_vector_set(ydat, i, log(itsArray[pos + i * xlen * ylen]));
                    }
                    gsl_multifit_linear_workspace * work = gsl_multifit_linear_alloc(ndata,
                                                           degree);
                    gsl_multifit_wlinear(xdat, w, ydat, c, cov, &chisq, work);
                    gsl_multifit_linear_free(work);

                    float Izero = exp(gsl_vector_get(c, 0));
                    float alpha = gsl_vector_get(c, 1);
                    float beta = gsl_vector_get(c, 2);
                    itsTTmaps[0](outpos) = Izero;
                    if (itsMaxTaylorTerm >= 1) {
                        itsTTmaps[1](outpos) = Izero * alpha;
                    }
                    if (itsMaxTaylorTerm >= 2) {
                        itsTTmaps[2](outpos) = Izero * (0.5 * alpha * (alpha - 1) + beta);
                    }

                }
            }
        }

        gsl_matrix_free(cov);
        gsl_vector_free(c);
        gsl_vector_free(w);
        gsl_vector_free(ydat);
        gsl_matrix_free(xdat);
        
    }
}

void FITSfile::writeTaylorTermImages(std::string nameBase, casa::IPosition location)
{

    for (size_t t = 0; t <= itsMaxTaylorTerm; t++) {
        std::stringstream outname;
        outname << nameBase << ".taylor." << t;
        casa::PagedImage<float> outimg(outname.str());
        outimg.putSlice(itsTTmaps[t], location);
    }

}





}

}

}
