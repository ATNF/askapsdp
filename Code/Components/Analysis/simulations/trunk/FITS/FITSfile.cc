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
#include <simulationutilities/Continuum.h>
#include <simulationutilities/HIprofile.h>
#include <simulationutilities/HIprofileS3SEX.h>
#include <simulationutilities/HIprofileS3SAX.h>
#include <analysisutilities/AnalysisUtilities.h>
#include <analysisutilities/CasaImageUtil.h>

#include <Common/ParameterSet.h>

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

#include <wcslib/wcs.h>
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

namespace askap {

    namespace simulations {

        namespace FITS {

            FITSfile::FITSfile()
            {
                /// @details Default constructor does not allocate anything, and the arrayAllocated flag is set to false.
                this->itsArrayAllocated = false;
                this->itsWCSAllocated = false;
                this->itsWCSsourcesAllocated = false;
            }

//--------------------------------------------------------

            FITSfile::~FITSfile()
            {
                /// @details Destructor deletes the flux array if it has been allocated.
                if (this->itsArrayAllocated) delete [] this->itsArray;

                int nwcs = 1;

                if (this->itsWCSAllocated) wcsvfree(&nwcs, &this->itsWCS);

                if (this->itsWCSsourcesAllocated) wcsvfree(&nwcs, &this->itsWCSsources);

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
                this->itsSourceList = f.itsSourceList;
                this->itsSourceListType = f.itsSourceListType;
                this->itsDatabaseOrigin = f.itsDatabaseOrigin;
                this->itsPosType = f.itsPosType;
                this->itsMinMinorAxis = f.itsMinMinorAxis;
                this->itsPAunits = f.itsPAunits;
                this->itsSourceFluxUnits = f.itsSourceFluxUnits;
                this->itsAxisUnits = f.itsAxisUnits;
                this->itsNumPix = f.itsNumPix;

                if (this->itsArrayAllocated) {
                    this->itsArrayAllocated = false;
                    delete [] itsArray;
                }

                this->itsArrayAllocated = f.itsArrayAllocated;

                if (this->itsArrayAllocated) {
                    this->itsArray = new float[this->itsNumPix];

                    for (size_t i = 0; i < this->itsNumPix; i++) this->itsArray[i] = f.itsArray[i];
                }

                this->itsNoiseRMS = f.itsNoiseRMS;
                this->itsDim = f.itsDim;
                this->itsAxes = f.itsAxes;
                this->itsSourceSection = f.itsSourceSection;
                this->itsHaveBeam = f.itsHaveBeam;
                this->itsBeamInfo = f.itsBeamInfo;
                this->itsHaveSpectralInfo = f.itsHaveSpectralInfo;
                this->itsBaseFreq = f.itsBaseFreq;
                this->itsRestFreq = f.itsRestFreq;
                this->itsAddSources = f.itsAddSources;
                this->itsDoContinuum = f.itsDoContinuum;
                this->itsDoHI = f.itsDoHI;
                this->itsDryRun = f.itsDryRun;
                this->itsEquinox = f.itsEquinox;
                this->itsBunit = f.itsBunit;
                this->itsUnitScl = f.itsUnitScl;
                this->itsUnitOff = f.itsUnitOff;
                this->itsUnitPwr = f.itsUnitPwr;

                int nwcs = 1;

                if (this->itsWCSAllocated) wcsvfree(&nwcs, &this->itsWCS);

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

                if (this->itsWCSsourcesAllocated) wcsvfree(&nwcs, &this->itsWCSsources);

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
                this->itsOutputSourceList = f.itsOutputSourceList;

                return *this;

            }

//--------------------------------------------------------

            FITSfile::FITSfile(const LOFAR::ParameterSet& parset, bool allocateMemory)
            {
                /// @details Constructor that reads in the necessary
                /// definitions from the parameterset. All FITSfile members
                /// are read in. The conversion factors for the source fluxes
                /// are also defined using the WCSLIB wcsunits function (using
                /// the sourceFluxUnits parameter: if this is not specified,
                /// the fluxes are assumed to be the same units as those of
                /// BUNIT). The pixel array is allocated here.

                ASKAPLOG_DEBUG_STR(logger, "Defining the FITSfile");
                this->itsFileName = parset.getString("filename", "");
                this->itsFITSOutput = parset.getBool("fitsOutput", true);
                this->itsCasaOutput = parset.getBool("casaOutput", false);
                this->itsBunit = casa::Unit(parset.getString("bunit", "Jy/Beam"));
                this->itsSourceList = parset.getString("sourcelist", "");
                std::ifstream file;
                file.open(this->itsSourceList.c_str(), std::ifstream::in);
                file.close();

                if (file.fail()) {
                    ASKAPTHROW(AskapError, "Source list " << this->itsSourceList << " could not be opened. Exiting.");
                }

                this->itsSourceListType = parset.getString("sourcelisttype", "continuum");

                if (this->itsSourceListType != "continuum" && this->itsSourceListType != "spectralline") {
                    this->itsSourceListType = "continuum";
                    ASKAPLOG_WARN_STR(logger, "Input parameter sourcelisttype needs to be *either* 'continuum' or 'spectralline'. Setting to continuum.");
                }

                this->itsDatabaseOrigin = parset.getString("database", "S3SAX"); // only used for spectralline case

                if (this->itsDatabaseOrigin != "S3SAX" && this->itsDatabaseOrigin != "S3SEX") {
                    this->itsDatabaseOrigin = "S3SAX";
                    ASKAPLOG_WARN_STR(logger, "Input parameter databaseorigin needs to be *either* 'S3SEX' or 'S3SAX'. Setting to S3SAX.");
                }

                this->itsPosType = parset.getString("posType", "dms");
                this->itsMinMinorAxis = parset.getFloat("minMinorAxis", 0.);
                this->itsPAunits = casa::Unit(parset.getString("PAunits", "rad"));

                if (this->itsPAunits != "rad" && this->itsPAunits != "deg") {
                    ASKAPLOG_WARN_STR(logger, "Input parameter PAunits needs to be *either* 'rad' *or* 'deg'. Setting to rad.");
                    this->itsPAunits = "rad";
                }

                this->itsAxisUnits = casa::Unit(parset.getString("axisUnits", "arcsec"));
                this->itsSourceFluxUnits = casa::Unit(parset.getString("sourceFluxUnits", ""));

                if (this->itsSourceFluxUnits != "") {
                    char *base = (char *)this->itsBunit.getName().c_str();
                    wcsutrn(0, base);
                    char *src = (char *)this->itsSourceFluxUnits.getName().c_str();
                    wcsutrn(0, src);
                    int status = wcsunits(src, base, &this->itsUnitScl, &this->itsUnitOff, &this->itsUnitPwr);

                    if (status) ASKAPTHROW(AskapError, "The parameters bunit (\"" << base
                                                   << "\") and sourceFluxUnits (\"" << src
                                                   << "\") are not interconvertible.");

                    ASKAPLOG_INFO_STR(logger, "Converting from " << src << " to " << base
                                          << ": " << this->itsUnitScl << "," << this->itsUnitOff << "," << this->itsUnitPwr);
                } else {
                    this->itsSourceFluxUnits = this->itsBunit;
                    this->itsUnitScl = 1.;
                    this->itsUnitOff = 0.;
                    this->itsUnitPwr = 1.;
                }

                this->itsNoiseRMS = parset.getFloat("noiserms", 0.001);

                this->itsDim = parset.getUint16("dim", 2);
                this->itsAxes = parset.getUint32Vector("axes");
                std::string sectionString = parset.getString("subsection", duchamp::nullSection(this->itsDim));
                this->itsSourceSection.setSection(sectionString);
                std::vector<int> axes(this->itsDim);

                for (uint i = 0; i < this->itsDim; i++) axes[i] = this->itsAxes[i];

                this->itsSourceSection.parse(axes);

                if (this->itsAxes.size() != this->itsDim)
                    ASKAPTHROW(AskapError, "Dimension mismatch: dim = " << this->itsDim
                                   << ", but axes has " << this->itsAxes.size() << " dimensions.");

                for (uint i = 0; i < this->itsDim; i++)
                    this->itsAxes[i] = this->itsSourceSection.getDim(i);

                std::stringstream ss;
                this->itsNumPix = this->itsAxes[0];
                ss << this->itsAxes[0];

                for (uint i = 1; i < this->itsDim; i++) {
                    this->itsNumPix *= this->itsAxes[i];
                    ss << "x" << this->itsAxes[i];
                }

                this->itsHaveBeam = parset.isDefined("beam");

                if (this->itsHaveBeam) this->itsBeamInfo = parset.getFloatVector("beam");

                this->itsEquinox = parset.getFloat("equinox", 2000.);
                LOFAR::ParameterSet subset(parset.makeSubset("WCSimage."));
                this->itsWCSAllocated = false;
                this->setWCS(true, subset);
                this->itsFlagPrecess = parset.getBool("WCSsources", false);
                this->itsWCSsourcesAllocated = false;

                if (this->itsFlagPrecess) {
                    LOFAR::ParameterSet subset(parset.makeSubset("WCSsources."));
                    this->setWCS(false, subset);
                }

                ASKAPLOG_DEBUG_STR(logger, "wcs->lat=" << this->itsWCS->lat << " wcs->lng=" << this->itsWCS->lng << " wcs->spec=" << this->itsWCS->spec);

                this->itsHaveSpectralInfo = parset.getBool("flagSpectralInfo", false);
                this->itsBaseFreq = parset.getFloat("baseFreq", this->itsWCS->crval[this->itsWCS->spec]);
                this->itsRestFreq = parset.getFloat("restFreq", nu0_HI);

                if (!this->itsHaveSpectralInfo) this->itsBaseFreq = this->itsWCS->crval[this->itsWCS->spec];

                this->itsAddSources = parset.getBool("addSources", true);
                this->itsDoContinuum = parset.getBool("doContinuum", true);
                this->itsDoHI = parset.getBool("doHI", false);
                this->itsDryRun = parset.getBool("dryRun", false);

                if (this->itsDryRun) {
                    this->itsFITSOutput = false;
                    this->itsCasaOutput = false;
                    ASKAPLOG_INFO_STR(logger, "Just a DRY RUN - no sources being added or images created.");
                }

                this->itsFlagOutputList = parset.getBool("outputList", false);

                if (this->itsSourceList.size() == 0) this->itsFlagOutputList = false;

                this->itsOutputSourceList = parset.getString("outputSourceList", "");

                if (allocateMemory && !this->itsDryRun) {
                    ASKAPLOG_DEBUG_STR(logger, "Allocating array of dimensions " << ss.str() << " with " << this->itsNumPix << " pixels, each of size " << sizeof(float) << " bytes");
                    this->itsArray = new float[this->itsNumPix];
                    this->itsArrayAllocated = true;
                    ASKAPLOG_DEBUG_STR(logger, "Allocation done.");

                    for (size_t i = 0; i < this->itsNumPix; i++) this->itsArray[i] = 0.;
                }

                ASKAPLOG_DEBUG_STR(logger, "FITSfile defined.");
            }

//--------------------------------------------------------

            void FITSfile::setWCS(bool isImage, const LOFAR::ParameterSet& parset)
            {
                /// @details Defines a world coordinate system from an
                /// input parameter set. This looks for parameters that
                /// define the various FITS header keywords for each
                /// axis (ctype, cunit, crval, cdelt, crpix, crota), as
                /// well as the equinox, then defines a WCSLIB wcsprm
                /// structure and assigns it to either FITSfile::itsWCS
                /// or FITSfile::itsWCSsources depending on the isImage
                /// parameter.
                /// @param isImage If true, the FITSfile::itsWCS
                /// structure is defined, else it is the
                /// FITSfile::itsWCSsources.
                /// @param parset The input parset to be examined.

                struct wcsprm *wcs = (struct wcsprm *)calloc(1, sizeof(struct wcsprm));
                wcs->flag = -1;
                wcsini(true , this->itsDim, wcs);
                wcs->flag = 0;
                std::vector<std::string> ctype, cunit;
                std::vector<float> crval, crpix, cdelt, crota;
                ctype = parset.getStringVector("ctype");

                if (ctype.size() != this->itsDim)
                    ASKAPTHROW(AskapError, "Dimension mismatch: dim = " << this->itsDim <<
                               ", but ctype has " << ctype.size() << " dimensions.");

                cunit = parset.getStringVector("cunit");

                if (cunit.size() != this->itsDim)
                    ASKAPTHROW(AskapError, "Dimension mismatch: dim = " << this->itsDim <<
                               ", but cunit has " << cunit.size() << " dimensions.");

                crval = parset.getFloatVector("crval");

                if (crval.size() != this->itsDim)
                    ASKAPTHROW(AskapError, "Dimension mismatch: dim = " << this->itsDim <<
                               ", but crval has " << crval.size() << " dimensions.");

                crpix = parset.getFloatVector("crpix");

                if (crpix.size() != this->itsDim)
                    ASKAPTHROW(AskapError, "Dimension mismatch: dim = " << this->itsDim <<
                               ", but crpix has " << crpix.size() << " dimensions.");

                cdelt = parset.getFloatVector("cdelt");

                if (cdelt.size() != this->itsDim)
                    ASKAPTHROW(AskapError, "Dimension mismatch: dim = " << this->itsDim <<
                               ", but cdelt has " << cdelt.size() << " dimensions.");

                crota = parset.getFloatVector("crota");

                if (crota.size() != this->itsDim)
                    ASKAPTHROW(AskapError, "Dimension mismatch: dim = " << this->itsDim <<
                               ", but crota has " << crota.size() << " dimensions.");

                for (uint i = 0; i < this->itsDim; i++) {
                    wcs->crpix[i] = crpix[i] - this->itsSourceSection.getStart(i);
                    wcs->cdelt[i] = cdelt[i];
                    wcs->crval[i] = crval[i];
                    wcs->crota[i] = crota[i];
                    strcpy(wcs->cunit[i], cunit[i].c_str());
                    strcpy(wcs->ctype[i], ctype[i].c_str());
                }

                wcs->equinox = this->itsEquinox;
                wcsset(wcs);

                int stat[NWCSFIX];
                int axes[this->itsAxes.size()];

                for (uint i = 0; i < this->itsAxes.size(); i++) axes[i] = this->itsAxes[i];

                int nwcs = 1;

                if (isImage) {
                    if (this->itsWCSAllocated) wcsvfree(&nwcs, &this->itsWCS);

                    this->itsWCS = (struct wcsprm *)calloc(1, sizeof(struct wcsprm));
                    this->itsWCSAllocated = true;
                    this->itsWCS->flag = -1;
                    wcsini(true, wcs->naxis, this->itsWCS);
                    wcsfix(1, (const int*)axes, wcs, stat);
                    wcscopy(true, wcs, this->itsWCS);
                    wcsset(this->itsWCS);
                } else {
                    if (this->itsWCSsourcesAllocated)  wcsvfree(&nwcs, &this->itsWCSsources);

                    this->itsWCSsources = (struct wcsprm *)calloc(1, sizeof(struct wcsprm));
                    this->itsWCSsourcesAllocated = true;
                    this->itsWCSsources->flag = -1;
                    wcsini(true, wcs->naxis, this->itsWCSsources);
                    wcsfix(1, (const int*)axes, wcs, stat);
                    wcscopy(true, wcs, this->itsWCSsources);
                    wcsset(this->itsWCSsources);
                }

                wcsvfree(&nwcs, &wcs);

            }

//--------------------------------------------------------

            void FITSfile::makeNoiseArray()
            {
                /// @details Fills the pixel array with fluxes sampled from a
                /// normal distribution ~ N(0,itsNoiseRMS) (i.e. the mean of
                /// the distribution is zero). Note that this overwrites the array.
                if (this->itsArrayAllocated) {
                    ASKAPLOG_DEBUG_STR(logger, "Making the noise array");

                    for (size_t i = 0; i < this->itsNumPix; i++) {
                        this->itsArray[i] = normalRandomVariable(0., this->itsNoiseRMS);
                    }
                }
            }

//--------------------------------------------------------

            void FITSfile::addNoise()
            {
                /// @details Adds noise to the array. Noise values are
                /// distributed as N(0,itsNoiseRMS) (i.e. with mean zero).
                if (this->itsArrayAllocated) {
                    ASKAPLOG_DEBUG_STR(logger, "Adding noise");

                    for (size_t i = 0; i < this->itsNumPix; i++) {
                        this->itsArray[i] += normalRandomVariable(0., this->itsNoiseRMS);
                    }
                }
            }

//--------------------------------------------------------

            void FITSfile::processSources()
            {
                /// @details Adds sources to the array. If the source list
                /// file has been defined, it is read one line at a time, and
                /// each source is added to the array. If it is a point source
                /// (i.e. major_axis = 0) then its flux is added to the
                /// relevant pixel, assuming it lies within the boundaries of
                /// the array. If it is a Gaussian source (major_axis>0), then
                /// the function addGaussian is used. The WCSLIB functions are
                /// used to convert the ra/dec positions to pixel positions.
                if (this->itsSourceList.size() > 0) { // if the source list is defined.
                    ASKAPLOG_DEBUG_STR(logger, "Adding sources from file " << this->itsSourceList);
                    std::ifstream srclist(this->itsSourceList.c_str());
                    std::string line, ra, dec;
                    int sourceType = 4;
                    double *wld = new double[3];
                    double *pix = new double[3];
                    double *newwld = new double[3];
                    std::ofstream outfile;

                    int countGauss = 0, countPoint = 0, countMiss=0, countDud=0;

		    Continuum cont;
		    HIprofileS3SEX profSEX;
		    HIprofileS3SAX profSAX;
		    HIprofile &prof = profSAX;
		    Spectrum &src = cont;

		    FluxGenerator fluxGen;
		    casa::Gaussian2D<casa::Double> gauss;
		    const float arcsecToPixel = 3600. * sqrt(fabs(this->itsWCS->cdelt[0] * this->itsWCS->cdelt[1]));

		    double maxFreq = this->maxFreq();
		    double minFreq = this->minFreq();
		    ASKAPLOG_DEBUG_STR(logger, "Maximum & minimum frequencies are " << maxFreq << " and " << minFreq);

                    if (this->itsFlagOutputList) outfile.open(this->itsOutputSourceList.c_str());

                    while (getline(srclist, line),
                            !srclist.eof()) {
// 		      ASKAPLOG_DEBUG_STR(logger, "input = " << line);

                        if (line[0] != '#') {  // ignore commented lines

                            if (this->itsHaveSpectralInfo) {
//                                 cont = Continuum(line);
                                cont.define(line);
                                cont.setNuZero(this->itsBaseFreq);
                                src = cont;
                            } else {
//                                 cont = Spectrum(line);
                                cont.define(line);
                                cont.setNuZero(this->itsBaseFreq);
                                src = cont;
                            }

                            if (this->itsSourceListType == "spectralline") {
                                if (this->itsDatabaseOrigin == "S3SEX") {
//                                     profSEX = HIprofileS3SEX(line);
                                    profSEX.define(line);
                                    sourceType = profSEX.type();
                                    src = profSEX;
                                    prof = profSEX;
//                                     std::cerr << profSEX << "\n";
                                } else if (this->itsDatabaseOrigin == "S3SAX") {
//                                     profSAX = HIprofileS3SAX(line);
                                    profSAX.define(line);
                                    src = profSAX;
                                    prof = profSAX;
//                                     std::cerr << profSAX << "\n";
                                } else
                                    ASKAPTHROW(AskapError, "'database' parameter has incompatible value '"
                                                   << this->itsDatabaseOrigin << "' - needs to be 'S3SEX' or 'S3SAX'");
                            }

                            // convert fluxes to correct units according to the image BUNIT keyword
                            src.setFluxZero(casa::Quantity(src.fluxZero(), this->itsSourceFluxUnits).getValue(this->itsBunit));

                            // convert sky position to pixels
                            if (this->itsPosType == "dms") {
                                wld[0] = analysis::dmsToDec(src.ra()) * 15.;
                                wld[1] = analysis::dmsToDec(src.dec());
                            } else if (this->itsPosType == "deg") {
                                wld[0] = atof(src.ra().c_str());
                                wld[1] = atof(src.dec().c_str());
                            } else ASKAPLOG_ERROR_STR(logger, "Incorrect position type: " << this->itsPosType);

                            wld[2] = this->itsBaseFreq;

                            if (this->itsFlagPrecess) wcsToPixSingle(this->itsWCSsources, wld, pix);
                            else                      wcsToPixSingle(this->itsWCS, wld, pix);

                            if (this->itsFlagOutputList) {
                                pixToWCSSingle(this->itsWCS, pix, newwld);
                                outfile.setf(std::ios::fixed);
                                outfile << std::setw(10) << std::setprecision(6) << newwld[0] << " "
					<< std::setw(10) << std::setprecision(6) << newwld[1] << " ";

				if(this->itsSourceListType == "spectralline" && this->itsDatabaseOrigin == "S3SAX")
				  outfile << std::setw(20) << std::setprecision(16) << profSAX.intFlux() << " ";
				else
				  outfile << std::setw(20) << std::setprecision(16) << src.fluxZero() << " ";

                                if (this->itsSourceListType == "spectralline" || this->itsHaveSpectralInfo)
                                    outfile << std::setw(10) << std::setprecision(6) << cont.alpha() << " "
                                        << std::setw(10) << std::setprecision(6) << cont.beta() << " ";

                                outfile << std::setw(10) << std::setprecision(6) << src.maj() << " "
                                    << std::setw(10) << std::setprecision(6) << src.min() << " "
                                    << std::setw(10) << std::setprecision(6) << src.pa() << " ";

                                if (this->itsSourceListType == "spectralline")
                                    outfile << std::setw(10) << std::setprecision(6) << prof.redshift() << " "
                                        << std::setw(10) << std::setprecision(6) << prof.mHI() << " "
                                        << std::setw(5) << sourceType << " ";

                                outfile << "\n";
                            }

			    bool lookAtSource = (this->itsArrayAllocated && this->itsAddSources) || this->itsDryRun;

			    if( this->itsSourceListType == "spectralline" && this->itsDatabaseOrigin == "S3SAX"){
			      // check the frequency limits for this source to see whether we need to look at it.
			      std::pair<double,double> freqLims = profSAX.freqLimits();
			      bool isGood = (freqLims.first < maxFreq) && (freqLims.second > minFreq);
			      if(isGood) ASKAPLOG_DEBUG_STR(logger, "Source (" << freqLims.second<<"-"<<freqLims.first<<") lies within freq limits");
			      else ASKAPLOG_DEBUG_STR(logger, "Outside freq limits! (" << freqLims.second<<"-"<<freqLims.first<<")");
			      lookAtSource = lookAtSource && isGood;
			    }

                            if (lookAtSource) {

                                if (this->itsWCS->spec > 0) fluxGen.setNumChan(this->itsAxes[this->itsWCS->spec]);
                                else fluxGen.setNumChan(1);

                                if (this->itsDoContinuum)
                                    fluxGen.addSpectrum(cont, pix[0], pix[1], this->itsWCS);

                                if (this->itsDoHI) {
                                    if (this->itsDatabaseOrigin == "S3SEX")
                                        fluxGen.addSpectrumInt(profSEX, pix[0], pix[1], this->itsWCS);
                                    else if (this->itsDatabaseOrigin == "S3SAX")
                                        fluxGen.addSpectrumInt(profSAX, pix[0], pix[1], this->itsWCS);
                                }

                                if (src.maj() > 0) {
                                    // convert widths from arcsec to pixels
                                    src.setMaj(casa::Quantity(src.maj(), this->itsAxisUnits).getValue("arcsec") / arcsecToPixel);

                                    if (src.maj() > 0 && !(src.min() > this->itsMinMinorAxis)) {
                                        ASKAPLOG_DEBUG_STR(logger, "Changing minor axis: " << src.min() << " --> " << this->itsMinMinorAxis);
                                        src.setMin(casa::Quantity(this->itsMinMinorAxis, this->itsAxisUnits).getValue("arcsec") / arcsecToPixel);
                                    } else src.setMin(casa::Quantity(src.min(), this->itsAxisUnits).getValue("arcsec") / arcsecToPixel);

                                    if (src.fluxZero() == 0.) src.setFluxZero(1.e-3);

//                                     casa::Gaussian2D<casa::Double> gauss(src.fluxZero(), pix[0], pix[1], src.maj(), src.min() / src.maj(),
//                                                                          casa::Quantity(src.pa(), this->itsPAunits).getValue("rad"));
                                    gauss.setXcenter(pix[0]);
                                    gauss.setYcenter(pix[1]);
                                    gauss.setMinorAxis(std::min(gauss.majorAxis(),src.maj()));  // need this so that we never have the minor axis > major axis
                                    gauss.setMajorAxis(src.maj());
                                    gauss.setMinorAxis(src.min());
                                    gauss.setPA(casa::Quantity(src.pa(), this->itsPAunits).getValue("rad"));
                                    gauss.setFlux(src.fluxZero());

                                    if (!this->itsDryRun) addGaussian(this->itsArray, this->itsAxes, gauss, fluxGen);
                                    else if (doAddGaussian(this->itsAxes, gauss))
                                        countGauss++;
				    else countMiss++;
                                } else {
                                    if (!this->itsDryRun) addPointSource(this->itsArray, this->itsAxes, pix, fluxGen);
                                    else if (doAddPointSource(this->itsAxes, pix))
                                        countPoint++;
				    else countMiss++;
                                }
                            }
			    else if(this->itsDryRun) countDud++;

                        } else {
                            // Write all commented lines directly into the output file
                            if (this->itsFlagOutputList) outfile << line << "\n";
                        }
                    }

                    if (this->itsFlagOutputList) outfile.close();

                    srclist.close();

                    if (this->itsDryRun)
		        ASKAPLOG_INFO_STR(logger, "Would add " << countPoint << " point sources and " << countGauss << " Gaussians, with " << countMiss<<" misses and " << countDud << " duds");


                    ASKAPLOG_DEBUG_STR(logger, "Finished adding sources");

                    delete [] wld;
                    delete [] pix;
                    delete [] newwld;
                }
            }


//--------------------------------------------------------

            void FITSfile::convolveWithBeam()
            {
                /// @brief The array is convolved with the Gaussian beam
                /// specified in itsBeamInfo. The GaussSmooth2D class from the
                /// Duchamp library is used. Note that this is only done if
                /// itsHaveBeam is set true.
                if (!this->itsHaveBeam) {
                    ASKAPLOG_WARN_STR(logger, "Cannot convolve with beam as the beam was not specified in the parset.");
                } else {
                    ASKAPLOG_DEBUG_STR(logger, "Convolving with the beam");
                    float maj = this->itsBeamInfo[0] / fabs(this->itsWCS->cdelt[0]);
                    float min = this->itsBeamInfo[1] / fabs(this->itsWCS->cdelt[1]);
                    float pa = this->itsBeamInfo[2];
                    GaussSmooth2D<float> smoother(maj, min, pa);
                    ASKAPLOG_DEBUG_STR(logger, "Defined the smoother, now to do the smoothing");
                    float *newArray = smoother.smooth(this->itsArray, this->itsAxes[0], this->itsAxes[1]);
                    ASKAPLOG_DEBUG_STR(logger, "Smoothing done.");

                    for (size_t i = 0; i < this->itsNumPix; i++) this->itsArray[i] = newArray[i];

                    ASKAPLOG_DEBUG_STR(logger, "Copying done.");

                    delete [] newArray;
                }
            }


//--------------------------------------------------------

            char *numerateKeyword(std::string key, int num)
            {
                /// @details A utility function to combine a keyword and a
                /// value, to produce a relevant FITS keyword for a given
                /// axis. For example numerateKeyword(CRPIX,1) returns CRPIX1.
                std::stringstream ss;
                ss << key << num;
                return (char *)ss.str().c_str();
            }

//--------------------------------------------------------

            void FITSfile::writeFITSimage(bool createFile, bool saveData)
            {
                /// @details Creates a FITS file with the appropriate headers
                /// and saves the flux array into it. Uses the CFITSIO library
                /// to do so.

                if (this->itsFITSOutput) {

                    ASKAPLOG_INFO_STR(logger, "Saving the FITS file to " << this->itsFileName);


                    int status = 0;

                    fitsfile *fptr;

                    if (createFile) {
                        ASKAPLOG_INFO_STR(logger, "Creating the FITS file");

                        if (fits_create_file(&fptr, this->itsFileName.c_str(), &status)) {
                            ASKAPLOG_ERROR_STR(logger, "Error opening FITS file:");
                            fits_report_error(stderr, status);
                            ASKAPTHROW(AskapError, "Error opening FITS file.");
                        }

                        status = 0;
                        long *dim = new long[this->itsDim];

                        for (uint i = 0; i < this->itsDim; i++) dim[i] = this->itsAxes[i];

                        if (fits_create_img(fptr, FLOAT_IMG, this->itsDim, dim, &status)) {
                            ASKAPLOG_ERROR_STR(logger, "Error creating the FITS image:");
                            fits_report_error(stderr, status);
                        }

                        delete [] dim;

                        status = 0;

                        std::string header = "EQUINOX";

                        if (fits_update_key(fptr, TFLOAT, (char *)header.c_str(), &(this->itsEquinox), NULL, &status))
                            fits_report_error(stderr, status);

                        if (this->itsHaveBeam) {
                            status = 0;

                            header = "BMAJ";

                            if (fits_update_key(fptr, TFLOAT, (char *)header.c_str(), &(this->itsBeamInfo[0]), NULL, &status))
                                fits_report_error(stderr, status);

                            status = 0;

                            header = "BMIN";

                            if (fits_update_key(fptr, TFLOAT, (char *)header.c_str(), &(this->itsBeamInfo[1]), NULL, &status))
                                fits_report_error(stderr, status);

                            status = 0;

                            header = "BPA";

                            if (fits_update_key(fptr, TFLOAT, (char *)header.c_str(), &(this->itsBeamInfo[2]), NULL, &status))
                                fits_report_error(stderr, status);
                        }

                        status = 0;

                        char *unit = (char *)this->itsBunit.getName().c_str();

                        header = "BUNIT";

                        if (fits_update_key(fptr, TSTRING, (char *)header.c_str(), unit,  NULL, &status))
                            fits_report_error(stderr, status);

                        if (this->itsSourceListType == "spectralline") {
                            status = 0;

                            header = "RESTFREQ";

                            if (fits_update_key(fptr, TFLOAT, (char *)header.c_str(), &(this->itsRestFreq), NULL, &status))
                                fits_report_error(stderr, status);
                        }

                        float val;

                        for (uint d = 0; d < this->itsDim; d++) {
                            status = 0;

                            if (fits_update_key(fptr, TSTRING, numerateKeyword("CTYPE", d + 1), this->itsWCS->ctype[d],  NULL, &status))
                                fits_report_error(stderr, status);

                            status = 0;

                            if (fits_update_key(fptr, TSTRING, numerateKeyword("CUNIT", d + 1), this->itsWCS->cunit[d],  NULL, &status))
                                fits_report_error(stderr, status);

                            status = 0;
                            val = this->itsWCS->crval[d];

                            if (fits_update_key(fptr, TFLOAT, numerateKeyword("CRVAL", d + 1), &val, NULL, &status))
                                fits_report_error(stderr, status);

                            val = this->itsWCS->cdelt[d];
                            status = 0;

                            if (fits_update_key(fptr, TFLOAT, numerateKeyword("CDELT", d + 1), &val, NULL, &status))
                                fits_report_error(stderr, status);

                            val = this->itsWCS->crpix[d];
                            status = 0;

                            if (fits_update_key(fptr, TFLOAT, numerateKeyword("CRPIX", d + 1), &val, NULL, &status))
                                fits_report_error(stderr, status);

                            val = this->itsWCS->crota[d];
                            status = 0;

                            if (fits_update_key(fptr, TFLOAT, numerateKeyword("CROTA", d + 1), &val, NULL, &status))
                                fits_report_error(stderr, status);
                        }

                    }

                    if (saveData) {

                        ASKAPLOG_INFO_STR(logger, "Saving the data to the FITS file");

                        if (!createFile) {
                            status = 0;
                            std::string filename = this->itsFileName;

                            if (filename[0] == '!') filename = filename.substr(1);

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
                        ASKAPLOG_DEBUG_STR(logger, "Image dimensions are " << axes[0] << "x" << axes[1] << "x" << axes[2] << "x" << axes[3]);

                        ASKAPLOG_INFO_STR(logger, "Opened the FITS file, preparing to write data");

                        long *fpixel = new long[this->itsDim];
                        long *lpixel = new long[this->itsDim];

                        for (uint i = 0; i < this->itsDim; i++) {
                            fpixel[i] = this->itsSourceSection.getStart(i) + 1;
                            lpixel[i] = this->itsSourceSection.getEnd(i) + 1;
                        }

                        status = 0;

                        if (fits_write_subset(fptr, TFLOAT, fpixel, lpixel, this->itsArray, &status))
                            fits_report_error(stderr, status);

                        delete [] fpixel;
                        delete [] lpixel;

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
                /// @details Takes the name of a fits file and produces
                /// the equivalent CASA image name. This simply involves
                /// removing the ".fits" extension if it exists, or, if it
                /// doesn't, adding a ".casa" extension.
                /// @param fitsName The name of the fits file
                /// @return The name of the casa image.

                std::string casaname;
                size_t pos = fitsName.rfind(".fits");

                if (pos == std::string::npos) { // imageName doesn't have a .fits extension
                    casaname = fitsName + ".casa";
                } else { // just remove the .fits extension
                    casaname = fitsName.substr(0, pos);
                }

                if (casaname[0] == '!') casaname = casaname.substr(1);

                return casaname;
            }


//--------------------------------------------------------

            void FITSfile::writeCASAimage(bool createFile, bool saveData)
            {
                /// @details Writes the data to a casa image. The WCS is
                /// converted to a casa-format coordinate system using
                /// the analysis package function, the brightness units
                /// and restoring beam are saved to the image, and the
                /// data array is written using a casa::Array class. No
                /// additional memory allocation is done in saving the
                /// data array (the casa::SHARE flag is used in the
                /// array constructor). The name of the casa image is
                /// determined by the casafy() function.

                if (this->itsCasaOutput) {

                    std::string newName = casafy(this->itsFileName);
                    casa::IPosition shape(this->itsDim);

                    for (uint i = 0; i < this->itsDim; i++) shape(i) = this->itsAxes[i];

                    if (createFile) {
                        casa::CoordinateSystem csys = analysis::wcsToCASAcoord(this->itsWCS);

                        ASKAPLOG_INFO_STR(logger, "Creating a new CASA image " << newName << " with the shape " << shape);
                        casa::PagedImage<float> img(casa::TiledShape(shape), csys, newName);

                        img.setUnits(this->itsBunit);

                        if (this->itsHaveBeam) {
                            casa::ImageInfo ii = img.imageInfo();
                            ii.setRestoringBeam(casa::Quantity(this->itsBeamInfo[0], "deg"),
                                                casa::Quantity(this->itsBeamInfo[1], "deg"),
                                                casa::Quantity(this->itsBeamInfo[2], "deg"));
                            img.setImageInfo(ii);
                        }
                    }

                    if (saveData) {

                        casa::PagedImage<float> img(newName);

                        // make the casa::Array, sharing the memory storage so there is minimal additional impact
                        Array<Float> arr(shape, this->itsArray, casa::SHARE);

                        casa::IPosition location(this->itsDim);

                        for (uint i = 0; i < this->itsDim; i++) location(i) = this->itsSourceSection.getStart(i);

                        ASKAPLOG_DEBUG_STR(logger, "shape = " << shape << ", location = " << location);
                        ASKAPLOG_INFO_STR(logger, "Writing an array with the shape " << arr.shape() << " into a CASA image " << newName << " at location " << location);
                        img.putSlice(arr, location);
                    }

                }

            }


	  double FITSfile::maxFreq()
	  {
	    int spec=this->itsWCS->spec;
	    return this->itsWCS->crval[spec] + (this->itsAxes[spec]/2+0.5)*this->itsWCS->cdelt[spec];
	  }
	  double FITSfile::minFreq()
	  {
	    int spec=this->itsWCS->spec;
	    return this->itsWCS->crval[spec] - (this->itsAxes[spec]/2+0.5)*this->itsWCS->cdelt[spec];
	  }
	 

        }

    }

}
