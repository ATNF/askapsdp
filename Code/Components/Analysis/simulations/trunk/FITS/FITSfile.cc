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
#include <analysisutilities/AnalysisUtilities.h>

#include <Common/ParameterSet.h>

#include <duchamp/Utils/Section.hh>

#include <scimath/Functionals/Gaussian1D.h>
#include <scimath/Functionals/Gaussian2D.h>
#include <scimath/Functionals/Gaussian3D.h>
#include <casa/namespace.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/Unit.h>

#include <wcslib/wcs.h>
#include <wcslib/wcsunits.h>
#include <wcslib/wcsfix.h>
#define WCSLIB_GETWCSTAB // define this so that we don't try to redefine wtbarr
// (this is a problem when using wcslib-4.2)
#include <fitsio.h>
#include <duchamp/duchamp.hh>
#include <duchamp/Utils/utils.hh>
#include <duchamp/Utils/GaussSmooth.hh>

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
		
		if(this->itsWCSAllocated) wcsvfree(&nwcs,&this->itsWCS);
		if(this->itsWCSsourcesAllocated) wcsvfree(&nwcs,&this->itsWCSsources);

            }


	  FITSfile::FITSfile(const FITSfile &f)
	  {
	    operator=(f);
	  }

	  FITSfile& FITSfile::operator=(const FITSfile &f)
	  {
	    if (this == &f) return *this;

	    this->itsFileName = f.itsFileName;
	    this->itsSourceList = f.itsSourceList;
	    this->itsSourceListType = f.itsSourceListType;
	    this->itsPosType = f.itsPosType;
	    this->itsMinMinorAxis = f.itsMinMinorAxis;
	    this->itsPAunits = f.itsPAunits;
	    this->itsSourceFluxUnits = f.itsSourceFluxUnits;
	    this->itsAxisUnits = f.itsAxisUnits;
	    this->itsNumPix = f.itsNumPix;
	    if(this->itsArrayAllocated){
	      this->itsArrayAllocated=false;
	      delete [] itsArray;
	    }
	    this->itsArrayAllocated = f.itsArrayAllocated;
	    if(this->itsArrayAllocated){
	      this->itsArray = new float[this->itsNumPix];
	      for(int i=0;i<this->itsNumPix;i++) this->itsArray[i] = f.itsArray[i];
	    }
	    this->itsNoiseRMS = f.itsNoiseRMS;
	    this->itsDim = f.itsDim;
	    this->itsAxes = f.itsAxes;
	    this->itsSourceSection = f.itsSourceSection;
	    this->itsHaveBeam = f.itsHaveBeam;
	    this->itsBeamInfo = f.itsBeamInfo;
	    this->itsHaveSpectralInfo = f.itsHaveSpectralInfo;
	    this->itsBaseFreq = f.itsBaseFreq;
	    this->itsDoContinuum = f.itsDoContinuum;
	    this->itsDoHI = f.itsDoHI;
	    this->itsEquinox = f.itsEquinox;
	    this->itsBunit = f.itsBunit;
            this->itsUnitScl = f.itsUnitScl;
            this->itsUnitOff = f.itsUnitOff;
            this->itsUnitPwr = f.itsUnitPwr;

	    int nwcs = 1;
	    if(this->itsWCSAllocated) wcsvfree(&nwcs,&this->itsWCS);
	    this->itsWCSAllocated = f.itsWCSAllocated;
	    if(this->itsWCSAllocated){
	      this->itsWCS = (struct wcsprm *)calloc(1,sizeof(struct wcsprm));
	      this->itsWCSAllocated = true;
	      this->itsWCS->flag     = -1;
	      wcsini(true, f.itsWCS->naxis, this->itsWCS); 
	      wcscopy(true, f.itsWCS, this->itsWCS); 
	      wcsset(this->itsWCS);
	    }
	    
	    this->itsFlagPrecess = f.itsFlagPrecess;
	    if(this->itsWCSsourcesAllocated) wcsvfree(&nwcs,&this->itsWCSsources);
	    this->itsWCSsourcesAllocated = f.itsWCSsourcesAllocated;
	    if(this->itsFlagPrecess){
	      if(this->itsWCSsourcesAllocated){
		this->itsWCSsources = (struct wcsprm *)calloc(1,sizeof(struct wcsprm));
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

            FITSfile::FITSfile(const LOFAR::ParameterSet& parset)
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
                this->itsBunit = casa::Unit(parset.getString("bunit", "Jy/Beam"));
                this->itsSourceList = parset.getString("sourcelist", "");
		std::ifstream file;
		file.open(this->itsSourceList.c_str(), std::ifstream::in);
		file.close();
		if(file.fail()){
		  ASKAPTHROW(AskapError,"Source list " << this->itsSourceList << " could not be opened. Exiting.");
		}
		this->itsSourceListType = parset.getString("sourcelisttype","continuum");
		if(this->itsSourceListType!="continuum" && this->itsSourceListType!="spectralline"){
		  this->itsSourceListType = "continuum";
		  ASKAPLOG_WARN_STR(logger, "Input parameter sourcelisttype needs to be *either* 'continuum' or 'spectralline'. Setting to continuum.");
		}
// 		this->itsDatabaseOrigin = parset.getString("database","S3SAX"); // only used for spectralline case
		
                this->itsPosType = parset.getString("posType", "dms");
		this->itsMinMinorAxis = parset.getFloat("minMinorAxis", 0.);
                this->itsPAunits = casa::Unit(parset.getString("PAunits", "rad"));
		if(this->itsPAunits != "rad" && this->itsPAunits != "deg"){
		  ASKAPLOG_WARN_STR(logger, "Input parameter PAunits needs to be *either* 'rad' *or* 'deg'. Setting to rad.");
		  this->itsPAunits = "rad";
		}
		this->itsAxisUnits = casa::Unit(parset.getString("axisUnits","arcsec"));
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

                this->itsDim = parset.getInt32("dim", 2);
                this->itsAxes = parset.getInt32Vector("axes");
		this->itsSourceSection.setSection(duchamp::nullSection(this->itsDim));
		this->itsSourceSection.parse(this->itsAxes);

                if (this->itsAxes.size() != this->itsDim)
                    ASKAPTHROW(AskapError, "Dimension mismatch: dim = " << this->itsDim
                                   << ", but axes has " << this->itsAxes.size() << " dimensions.");

                this->itsNumPix = this->itsAxes[0];

                for (uint i = 1; i < this->itsDim; i++) this->itsNumPix *= this->itsAxes[i];

                this->itsArray = new float[this->itsNumPix];
                this->itsArrayAllocated = true;
                for (int i = 0; i < this->itsNumPix; i++) this->itsArray[i] = 0.;

                this->itsHaveBeam = parset.isDefined("beam");

                if (this->itsHaveBeam) this->itsBeamInfo = parset.getFloatVector("beam");

                this->itsEquinox = parset.getFloat("equinox", 2000.);
                LOFAR::ParameterSet subset(parset.makeSubset("WCSimage."));
		this->itsWCSAllocated = false;
                this->setWCS(true, subset);
                this->itsFlagPrecess = parset.getBool("WCSsources", false);

                if (this->itsFlagPrecess) {
                    LOFAR::ParameterSet subset(parset.makeSubset("WCSsources."));
		    this->itsWCSsourcesAllocated = false;
                    this->setWCS(false, subset);
                }

                this->itsHaveSpectralInfo = parset.getBool("flagSpectralInfo", false);
		this->itsBaseFreq = parset.getFloat("baseFreq", this->itsWCS->crval[this->itsWCS->spec]);
		if(!this->itsHaveSpectralInfo) this->itsBaseFreq = this->itsWCS->crval[this->itsWCS->spec];

		this->itsDoContinuum = parset.getBool("doContinuum", true);
		this->itsDoHI = parset.getBool("doHI",false);

                this->itsFlagOutputList = parset.getBool("outputList", false);

                if (this->itsSourceList.size() == 0) this->itsFlagOutputList = false;

                this->itsOutputSourceList = parset.getString("outputSourceList", "");

		ASKAPLOG_DEBUG_STR(logger, "FITSfile defined.");
            }

//--------------------------------------------------------

            void FITSfile::setWCS(bool isImage, const LOFAR::ParameterSet& parset)
            {
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
                    wcs->crpix[i] = crpix[i];
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
		for(uint i=0;i<this->itsAxes.size();i++) axes[i] = this->itsAxes[i];

		int nwcs = 1;
                if (isImage) {
		  if(this->itsWCSAllocated) wcsvfree(&nwcs, &this->itsWCS);
                    this->itsWCS = (struct wcsprm *)calloc(1, sizeof(struct wcsprm));
		    this->itsWCSAllocated = true;
                    this->itsWCS->flag = -1;
                    wcsini(true, wcs->naxis, this->itsWCS);
		    wcsfix(1, (const int*)axes, wcs, stat);
                    wcscopy(true, wcs, this->itsWCS);
                    wcsset(this->itsWCS);
                } else {
		  if(this->itsWCSsourcesAllocated)  wcsvfree(&nwcs, &this->itsWCSsources);
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
                ASKAPLOG_DEBUG_STR(logger, "Making the noise array");

                for (int i = 0; i < this->itsNumPix; i++) {
                    this->itsArray[i] = normalRandomVariable(0., this->itsNoiseRMS);
                }
            }

//--------------------------------------------------------

            void FITSfile::addNoise()
            {
                /// @details Adds noise to the array. Noise values are
                /// distributed as N(0,itsNoiseRMS) (i.e. with mean zero).
                ASKAPLOG_DEBUG_STR(logger, "Adding noise");

                for (int i = 0; i < this->itsNumPix; i++) {
                    this->itsArray[i] += normalRandomVariable(0., this->itsNoiseRMS);
                }
            }

//--------------------------------------------------------

            void FITSfile::addSources()
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
		    casa::Double flux, maj, min, pa,redshift,m_HI,alpha,beta;
		    int sourceType;
                    double *wld = new double[3];
                    double *pix = new double[3];
                    double *newwld = new double[3];
                    std::ofstream outfile;

                    if (this->itsFlagOutputList) outfile.open(this->itsOutputSourceList.c_str());
		    
                    while (getline(srclist, line),
                            !srclist.eof()) {

                        if (line[0] != '#') {  // ignore commented lines

			    alpha = 0.;
			    beta = 0.;
			    redshift = 0.;
			    m_HI = 0.;
// 			    if(this->itsSourceListType=="continuum"){
// 			      if (this->itsHaveSpectralInfo) {
// 				line >> ra >> dec >> flux >> alpha >> beta >> maj >> min >> pa; 
// 			      }
// 			      else {
// 				line >> ra >> dec >> flux >> maj >> min >> pa; 
// 			      }
// 			    }
// 			    else if(this->itsSourceListType=="spectralline"){
// 			      line >> ra >> dec >> flux >> alpha >> beta >> maj >> min >> pa >> redshift >> m_HI >> sourceType;
// 			    }
// 			    else
// 			      ASKAPTHROW(AskapError, "sourcelisttype has incompatible value '"
// 					 <<this->itsSourceListType<<"' - needs to be continuum or spectralline");
			    Continuum cont;
			    HIprofileS3SEX prof;
			    Spectrum &src=cont;
// 			    if(this->itsSourceListType=="continuum"){
			      if (this->itsHaveSpectralInfo) {
				cont = Continuum(line);
				cont.setNuZero(this->itsBaseFreq);
				src = cont;
			      }
			      else {
				cont = Spectrum(line);
				cont.setNuZero(this->itsBaseFreq);
				src = cont;
			      }
// 			    }
// 			    else if(this->itsSourceListType=="spectralline"){
			    if(this->itsSourceListType=="spectralline"){
			      prof = HIprofileS3SEX(line);
			      src = prof;
 			      std::cerr << prof << "\n";
			    }
			    else
			      ASKAPTHROW(AskapError, "sourcelisttype has incompatible value '"
					 <<this->itsSourceListType<<"' - needs to be continuum or spectralline");

                            // convert fluxes to correct units according to the image BUNIT keyword
// 			    flux = casa::Quantity(flux,this->itsSourceFluxUnits).getValue(this->itsBunit);
			    src.setFluxZero(casa::Quantity(src.fluxZero(),this->itsSourceFluxUnits).getValue(this->itsBunit));

			    FluxGenerator fluxGen;
			    if(this->itsWCS->spec > 0) fluxGen.setNumChan(this->itsAxes[this->itsWCS->spec]);
			    else fluxGen.setNumChan(1);
			    ASKAPLOG_DEBUG_STR(logger, "Have defined a flux generator with " << fluxGen.nChan() << " channels.");

// 			    Continuum cont(alpha,beta,this->itsBaseFreq,flux);
// 			    HIprofileS3SEX prof;
// 			    if(this->itsDoHI){
// 			      prof = HIprofileS3SEX(GALTYPE(sourceType),redshift,m_HI,maj,min);
// 			      std::cerr << prof << "\n";
// 			    }

                            // convert sky position to pixels
                            if (this->itsPosType == "dms") {
//                                 wld[0] = analysis::dmsToDec(ra) * 15.;
//                                 wld[1] = analysis::dmsToDec(dec);
			      wld[0] = analysis::dmsToDec(src.ra()) * 15.;
			      wld[1] = analysis::dmsToDec(src.dec());
                            } else if (this->itsPosType == "deg") {
//                                 wld[0] = atof(ra.c_str());
//                                 wld[1] = atof(dec.c_str());
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
					<< std::setw(10) << std::setprecision(6) << newwld[1] << " "
					<< std::setw(20) << std::setprecision(16) << flux << " "
					<< std::setw(10) << std::setprecision(6) << alpha << " "
					<< std::setw(10) << std::setprecision(6) << beta << " "
					<< std::setw(10) << std::setprecision(6) << maj << " "
					<< std::setw(10) << std::setprecision(6) << min << " "
					<< std::setw(10) << std::setprecision(6) << pa << " "
					<< std::setw(10) << std::setprecision(6) << redshift << " "
					<< std::setw(10) << std::setprecision(6) << m_HI << " "
					<< std::setw(5) << sourceType << "\n";
                            }

			    if(this->itsDoContinuum) fluxGen.addSpectrum(cont,pix[0],pix[1],this->itsWCS);
			    if(this->itsDoHI)        fluxGen.addSpectrumInt(prof,pix[0],pix[1],this->itsWCS);

//                             if (maj > 0) {
                            if (src.maj() > 0) {
                                // convert widths from arcsec to pixels
                                float arcsecToPixel = 3600. * sqrt(fabs(this->itsWCS->cdelt[0] * this->itsWCS->cdelt[1]));
//                                 maj = casa::Quantity(maj,this->itsAxisUnits).getValue("arcsec") / arcsecToPixel;
// 				if(maj>0 && !(min>this->itsMinMinorAxis)){
// 				  ASKAPLOG_DEBUG_STR(logger, "Changing minor axis: " << min << " --> " << this->itsMinMinorAxis);
// 				  min = casa::Quantity(this->itsMinMinorAxis,this->itsAxisUnits).getValue("arcsec") / arcsecToPixel;
// 				}
//                                 else min = casa::Quantity(min,this->itsAxisUnits).getValue("arcsec") / arcsecToPixel;
                                src.setMaj(casa::Quantity(src.maj(),this->itsAxisUnits).getValue("arcsec") / arcsecToPixel);
				if(src.maj()>0 && !(src.min()>this->itsMinMinorAxis)){
				  ASKAPLOG_DEBUG_STR(logger, "Changing minor axis: " << src.min() << " --> " << this->itsMinMinorAxis);
				  src.setMin(casa::Quantity(this->itsMinMinorAxis,this->itsAxisUnits).getValue("arcsec") / arcsecToPixel);
				}
                                else src.setMin(casa::Quantity(src.min(),this->itsAxisUnits).getValue("arcsec") / arcsecToPixel);

// 				if (src.flux == 0.) flux = 1.e-3;
				if (src.fluxZero() == 0.) src.setFluxZero(1.e-3);

// 				ASKAPLOG_DEBUG_STR(logger, "Defining a Gaussian of flux " << flux << " at [" << pix[0] << ","<<pix[1]<<"]");
//                                 casa::Gaussian2D<casa::Double> gauss(flux, pix[0], pix[1], maj, min/maj,
// 								     casa::Quantity(pa,this->itsPAunits).getValue("rad"));
// 				gauss.setFlux(flux);
				ASKAPLOG_DEBUG_STR(logger, "Defining a Gaussian of flux " << src.fluxZero() << " at [" << pix[0] << ","<<pix[1]<<"]");
                                casa::Gaussian2D<casa::Double> gauss(src.fluxZero(), pix[0], pix[1], src.maj(), src.min() / src.maj(), 
								     casa::Quantity(src.pa(),this->itsPAunits).getValue("rad"));
				gauss.setFlux(src.fluxZero());

                                addGaussian(this->itsArray, this->itsSourceSection, this->itsAxes, gauss, fluxGen);
                            } else {
			      addPointSource(this->itsArray, this->itsSourceSection, this->itsAxes, pix, fluxGen);
                            }
                        }
			else{
			  // Write all commented lines directly into the output file
			  if(this->itsFlagOutputList) outfile << line << "\n";
			}
                    }

                    if (this->itsFlagOutputList) outfile.close();
		    srclist.close();

                    delete [] wld;
                    delete [] pix;
                    delete [] newwld;
                }
            }


//--------------------------------------------------------

            void FITSfile::convolveWithBeam()
            {
                /// @brief The array is convolved with the Gaussian beam
                /// specified in itsBeamInfo. The GaussSmooth class from the
                /// Duchamp library is used. Note that this is only done if
                /// itsHaveBeam is set true.
                if (!this->itsHaveBeam) {
                    ASKAPLOG_WARN_STR(logger, "Cannot convolve with beam as the beam was not specified in the parset.");
                } else {
                    ASKAPLOG_DEBUG_STR(logger, "Convolving with the beam");
                    float maj = this->itsBeamInfo[0] / fabs(this->itsWCS->cdelt[0]);
                    float min = this->itsBeamInfo[1] / fabs(this->itsWCS->cdelt[1]);
                    float pa = this->itsBeamInfo[2];
                    GaussSmooth<float> smoother(maj, min, pa);
                    float *newArray = smoother.smooth(this->itsArray, this->itsAxes[0], this->itsAxes[1]);

                    for (int i = 0; i < this->itsNumPix; i++) this->itsArray[i] = newArray[i];

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

            void FITSfile::saveFile()
            {
                /// @details Creates a FITS file with the appropriate headers
                /// and saves the flux array into it. Uses the CFITSIO library
                /// to do so.

//                 ASKAPLOG_DEBUG_STR(logger, "Saving the FITS file to " << this->itsFileName);
                int status = 0;
                long *fpixel = new long[this->itsDim];

                for (uint i = 0; i < this->itsDim; i++) fpixel[i] = 1;

                fitsfile *fptr;

                if (fits_create_file(&fptr, this->itsFileName.c_str(), &status))
                    fits_report_error(stderr, status);

		if(status){
		  ASKAPLOG_ERROR_STR(logger, "Error opening FITS file:");
		  fits_report_error(stderr, status);
		  ASKAPTHROW(AskapError,"Error opening FITS file.");
		}

                status = 0;
                long *dim = new long[this->itsDim];

                for (uint i = 0; i < this->itsDim; i++) dim[i] = this->itsAxes[i];

                if (fits_create_img(fptr, FLOAT_IMG, this->itsDim, dim, &status))
                    fits_report_error(stderr, status);

		delete [] dim;

                status = 0;

                if (fits_update_key(fptr, TFLOAT, "EQUINOX", &(this->itsEquinox), NULL, &status))
                    fits_report_error(stderr, status);

                if (this->itsHaveBeam) {
                    status = 0;

                    if (fits_update_key(fptr, TFLOAT, "BMAJ", &(this->itsBeamInfo[0]), NULL, &status))
                        fits_report_error(stderr, status);

                    status = 0;

                    if (fits_update_key(fptr, TFLOAT, "BMIN", &(this->itsBeamInfo[1]), NULL, &status))
                        fits_report_error(stderr, status);

                    status = 0;

                    if (fits_update_key(fptr, TFLOAT, "BPA", &(this->itsBeamInfo[2]), NULL, &status))
                        fits_report_error(stderr, status);
                }

                status = 0;

		char *unit = (char *)this->itsBunit.getName().c_str();
//                 if (fits_update_key(fptr, TSTRING, "BUNIT", (char *)this->itsBunit.getName().c_str(),  NULL, &status))
                if (fits_update_key(fptr, TSTRING, "BUNIT", unit,  NULL, &status))
                    fits_report_error(stderr, status);

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

                status = 0;

                if (fits_write_pix(fptr, TFLOAT, fpixel, this->itsNumPix, this->itsArray, &status))
                    fits_report_error(stderr, status);

                status = 0;
                fits_close_file(fptr, &status);

                if (status) {
                    std::cerr << "Error closing file: ";
                    fits_report_error(stderr, status);
                }

		delete [] fpixel;

            }


        }

    }

}
