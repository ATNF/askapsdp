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

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

#include <duchamp/fitsHeader.hh>
#include <duchamp/Utils/Statistics.hh>
#include <duchamp/Utils/Section.hh>
#include <duchamp/param.hh>

#define WCSLIB_GETWCSTAB // define this so that we don't try and redefine 
//  wtbarr (this is a problem when using gcc v.4+)
#include <fitsio.h>

///@brief Where the log messages go.
ASKAP_LOGGER(logger, ".analysisutilities");

namespace askap {
    namespace analysis {

        std::vector<long> getFITSdimensions(std::string filename)
        {
            /// @details A simple function to open a FITS file and read the
            /// axis dimensions, returning the array of values.
            int numAxes, status = 0;  /* MUST initialize status */
            fitsfile *fptr;
            std::vector<long> dim;
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

                long *dimAxes = new long[numAxes];

                for (int i = 0; i < numAxes; i++) dimAxes[i] = 1;

                status = 0;

                if (fits_get_img_size(fptr, numAxes, dimAxes, &status)) {
                    fits_report_error(stderr, status);
                }

                // Close the FITS file -- not needed any more in this function.
                status = 0;
                fits_close_file(fptr, &status);

                if (status) {
                    fits_report_error(stderr, status);
                }

                dim = std::vector<long>(numAxes);

                for (int i = 0; i < numAxes; i++) dim[i] = dimAxes[i];

                delete [] dimAxes;

            }

            return dim;
        }

        float chisqProb(float ndof, float chisq)
        {
            /// @details Returns the probability of exceeding the given
            /// value of chisq by chance. If it comes from a fit, this
            /// probability is assuming the fit is valid.
            ///
            /// Typical use: say you have a fit with ndof=5 degrees of
            /// freedom that gives a chisq value of 12. You call this
            /// function via chisqProb(5,12.), which will return
            /// 0.0347878. If your confidence limit is 95% (ie. you can
            /// tolerate a 1-in-20 chance that a valid fit will produce a
            /// chisq value that high), you would reject that fit (since
            /// 0.0347878 < 0.05), but if it is 99%, you would accept it
            /// (since 0.0347878 > 0.01).
            return gsl_sf_gamma_inc(ndof / 2., chisq / 2.) / gsl_sf_gamma(ndof / 2.);
        }

        duchamp::Param parseParset(const LOFAR::ParameterSet& parset)
        {
            /// @details
            /// Takes a ParameterSet and reads in the necessary Duchamp
            /// parameters. It checks many of the duchamp::param parameters,
            /// and if they are not present, a default value, defined
            /// herein, is set (note that this is not necessarily the
            /// standard Duchamp default value).
            ///
            /// The excpetions are the image names, as these will in general
            /// depend on the node and on whether the current node is a
            /// master or worker. These should be set by the calling
            /// function.
            duchamp::Param par;

	    if(parset.isDefined("image"))
	      par.setImageFile(parset.getString("image"));
	    else if(parset.isDefined("imageFile"))
	      par.setImageFile(parset.getString("imageFile"));
	    else
	      ASKAPLOG_ERROR_STR(logger, "No image defined - use either 'imageFile' or 'image' parameters (the former is for consistency with Duchamp parameters)");
            par.setFlagSubsection(parset.getBool("flagSubsection", false));
            if (par.getFlagSubsection())
                par.setSubsection(parset.getString("subsection", ""));
	    // flagReconExists
	    // reconFile
	    // flagSmoothExists
	    // smoothFile
	    // usePrevious
	    // objectList

            par.setFlagLog(parset.getBool("flagLog",true)); // different from Duchamp default
	    // logfile - this is defined in DuchampParallel, as it depends on the worker/master number.
            std::string outputfile;
            outputfile = parset.getString("outfile", "");
            if (outputfile == "") outputfile =  parset.getString("resultsFile", "");
            if (outputfile != "") par.setOutFile(outputfile);
	    par.setFlagSeparateHeader(parset.getBool("flagSeparateHeader",par.getFlagSeparateHeader()));
	    par.setHeaderFile(parset.getString("headerFile",par.getHeaderFile()));
	    // spectraFile
	    // flagTextSpectra
	    // spectraTextFile --> can't do as this code is in outputSpectra.cc which is disabled due to no pgplot
	    // flagOutputMomentMap
	    // fileOutputMomentMap
	    // flagOutputMask
	    // fileOutputMask
	    // flagMaskWithObjectNum
	    // flagOutputSmooth
	    // fileOutputSmooth
	    // flagOutputRecon
	    // fileOutputRecon
	    // flagOutputResid
	    // fileOutputResid
	    // flagVOT
	    // votfile
            par.setFlagKarma(parset.getBool("flagKarma", true)); // different from Duchamp default
	    par.setKarmaFile(parset.getString("karmaFile",par.getKarmaFile()));
	    par.setFlagMaps(false); // flagMaps
	    // detectMap
	    // momentMap
	    // flagXOutput - not using X
	    // newFluxUnits - not using - caused confusion...
	    par.setPrecFlux(parset.getInt16("precFlux",par.getPrecFlux()));
	    par.setPrecVel(parset.getInt16("precVel",par.getPrecVel()));
	    par.setPrecSNR(parset.getInt16("precSNR",par.getPrecSNR()));

	    //

	    // flagTrim
	    par.setFlagMW(parset.getBool("flagMW",par.getFlagMW()));
	    par.setMinMW(parset.getInt16("minMW",par.getMinMW()));
	    par.setMaxMW(parset.getInt16("maxMW",par.getMaxMW()));
	    // flagBaseline

	    //

            par.setFlagStatSec(parset.getBool("flagStatSec", par.getFlagStatSec));
            par.setStatSec(parset.getString("statsec", par.getStatSec()));
            par.setFlagRobustStats(parset.getBool("flagRobust", par.getFlagRobust()));
	    par.setFlagNegative(parset.getBool("flagNegative",par.getFlagNegative()));
            par.setCut(parset.getFloat("snrCut", par.getCut()));
            if (parset.isDefined("threshold")) {
                par.setFlagUserThreshold(true);
                par.setThreshold(parset.getFloat("threshold",par.getThreshold()));
            } else {
                par.setFlagUserThreshold(false);
            }
            par.setFlagGrowth(parset.getBool("flagGrowth", par.getFlagGrowth()));
            par.setGrowthCut(parset.getFloat("growthCut", par.getGrowthCut()));
            if (parset.isDefined("growthThreshold")) {
	      par.setGrowthThreshold(parset.getFloat("growthThreshold",par.getGrowthThreshold()));
	      par.setFlagUserGrowthThreshold(true);
            }
            if (parset.isDefined("beamSize")) {
	      par.setBeamSize(parset.getFloat("beamSize"));
	      ASKAPLOG_WARN_STR(logger, "Parset has beamSize parameter. This is deprecated from Duchamp 1.1.9 onwards - use beamArea instead. Setting beamArea=" << par.getBeamSize());
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
            par.setFilterCode(parset.getInt16("filterCode", par.getFilterCode()));
            par.filter().define(par.getFilterCode());

	    //

            if (par.getFlagATrous()) par.setFlagSmooth(false);
            else par.setFlagSmooth(parset.getBool("flagSmooth", false));
            par.setSmoothType(parset.getString("smoothType", par.getSmoothType()));
            par.setHanningWidth(parset.getInt16("hanningWidth", par.getHanningWidth()));
            par.setKernMaj(parset.getFloat("kernMaj", par.getKernMaj()));
            par.setKernMin(parset.getFloat("kernMin", par.getKernMin()));
            par.setKernPA(parset.getFloat("kernPA", par.getKernPA()));

	    // flagFDR? How to deal with distributed case?
	    // alphaFDR ?
	    // FDRnumCorChan ?

            par.setFlagAdjacent(parset.getBool("flagAdjacent", par.getFlagAdjacent()));
            par.setThreshS(parset.getFloat("threshSpatial", par.getThreshS()));
            par.setThreshV(parset.getFloat("threshVelocity", par.getThreshV()));
            par.setMinPix(parset.getInt16("minPix", par.getMinPix()));
            par.setMinChannels(parset.getInt16("minChannels", par.getMinChannels()));
	    par.setMinVoxels(parset.getInt16("minVoxels", par.getMinVoxels()));
	    par.setFlagRejectBeforeMerge(parset.getBool("flagRejectBeforeMerge",par.getFlagRejectBeforeMerge()));
	    par.setFlagTwoStageMerging(parset.getBool("flagTwoStageMerging",par.getFlagTwoStageMerging()));

	    //

            par.setVerbosity(parset.getBool("verbose", false));
	    // No drawBorders
	    // No drawBlankEdges
            par.setPixelCentre(parset.getString("pixelCentre", "centroid"));
	    par.setSpectralMethod(parset.getString("spectralMethod",par.getSpectralMethod()));
	    par.setSpectralUnits(parset.getString("spectralUnits",par.getSpectralUnits()));
	    par.setSortingParam(parset.getString("sortingParam",par.getSortingParam()));


            par.checkPars();
            return par;
        }

        double findSpread(bool robust, double middle, int size, float *array)
        {
            /// @details
            /// Finds the "spread" (ie. the rms or standard deviation) of an
            /// array of values using a given mean value. The option exists
            /// to use the standard deviation, or, by setting robust=true,
            /// the median absolute deviation from the median. In the latter
            /// case, the middle value given is assumed to be the median,
            /// and the returned value is the median absolute difference of
            /// the data values from the median.
            double spread = 0.;

            if (robust) {
                float *arrayCopy = new float[size];

                for (int i = 0; i < size; i++) arrayCopy[i] = fabs(array[i] - middle);

                bool isEven = ((size / 2) == 0);
                std::nth_element(arrayCopy, arrayCopy + size / 2, arrayCopy + size);
                spread = arrayCopy[size/2];

                if (isEven) {
                    std::nth_element(arrayCopy, arrayCopy + size / 2 - 1, arrayCopy + size);
                    spread += arrayCopy[size/2-1];
                    spread /= 2.;
                }

                delete [] arrayCopy;
                spread = Statistics::madfmToSigma(spread);
            } else {
                for (int i = 0; i < size; i++) spread += (array[i] - middle) * (array[i] - middle);

                spread = sqrt(spread / double(size - 1));
            }

            return spread;
        }


        double findSpread(bool robust, double middle, int size, float *array, bool *mask)
        {
            /// @details
            /// Finds the "spread" (ie. the rms or standard deviation) of an
            /// array of values using a given mean value. The option exists
            /// to use the standard deviation, or, by setting robust=true,
            /// the median absolute deviation from the median. In the latter
            /// case, the middle value given is assumed to be the median,
            /// and the returned value is the median absolute difference of
            /// the data values from the median.
            int goodSize = 0;

            for (int i = 0; i < size; i++) if (mask[i]) goodSize++;

            double spread = 0.;

            if (robust) {
                float *arrayCopy = new float[goodSize];
                int j = 0;

                for (int i = 0; i < size; i++) if (mask[i]) arrayCopy[j++] = fabs(array[i] - middle);

                bool isEven = ((goodSize / 2) == 0);
                std::nth_element(arrayCopy, arrayCopy + goodSize / 2, arrayCopy + goodSize);
                spread = arrayCopy[goodSize/2];

                if (isEven) {
                    std::nth_element(arrayCopy, arrayCopy + goodSize / 2 - 1, arrayCopy + goodSize);
                    spread += arrayCopy[goodSize/2-1];
                    spread /= 2.;
                }

                delete [] arrayCopy;
                spread = Statistics::madfmToSigma(spread);
            } else {
                for (int i = 0; i < size; i++) if (mask[i]) spread += (array[i] - middle) * (array[i] - middle);

                spread = sqrt(spread / double(goodSize - 1));
            }

            return spread;
        }


        std::string removeLeadingBlanks(std::string s)
        {
            /// @brief Remove blank spaces from the beginning of a string
            /// @details
            /// All blank spaces from the start of the string to the first
            /// non-blank-space character are deleted.
            ///
            int i = 0;

            while (s[i] == ' ') {
                i++;
            }

            std::string newstring;

            for (unsigned int j = i; j < s.size(); j++) newstring += s[j];

            return newstring;
        }

        double dmsToDec(std::string input)
        {
            /// @details
            ///  Assumes the angle given is in degrees, so if passing RA as
            ///  the argument, need to multiply by 15 to get the result in
            ///  degrees rather than hours.  The sign of the angle is
            ///  preserved, if present.
            ///
            std::string dms = removeLeadingBlanks(input);
            bool isNeg = false;

            if (dms[0] == '-') isNeg = true;

            for (unsigned int i = 0; i < dms.size(); i++) if (dms[i] == ':') dms[i] = ' ';

            std::stringstream ss;
            ss.str(dms);
            double d, m, s;
            ss >> d >> m >> s;
            double dec = fabs(d) + m / 60. + s / 3600.;

            if (isNeg) dec = dec * -1.;

            return dec;
        }

        std::string decToDMS(const double input, std::string type, int secondPrecision, std::string separator)
        {
            /// @details
            ///  This is the general form, where one can specify the degree of
            ///  precision of the seconds, and the separating character. The format reflects the axis type:
            ///  @li RA   (right ascension):     hh:mm:ss.ss, with dec modulo 360. (24hrs)
            ///  @li DEC  (declination):        sdd:mm:ss.ss  (with sign, either + or -)
            ///  @li GLON (galactic longitude): ddd:mm:ss.ss, with dec made modulo 360.
            ///  @li GLAT (galactic latitude):  sdd:mm:ss.ss  (with sign, either + or -)
            ///    Any other type defaults to RA, and prints warning.
            /// @param input Angle in decimal degrees.
            /// @param type Axis type to be used
            /// @param secondPrecision How many decimal places to quote the seconds to.
            /// @param separator The character (or string) to use as a
            /// separator between hh and mm, and mm and ss.sss.
            ///
            double normalisedInput = input;
            int degSize = 2; // number of figures in the degrees part of the output.
            std::string sign = "";

            if ((type == "RA") || (type == "GLON")) {
                if (type == "GLON")  degSize = 3; // longitude has three figures in degrees.

                // Make these modulo 360.;
                while (normalisedInput < 0.) { normalisedInput += 360.; }

                while (normalisedInput > 360.) { normalisedInput -= 360.; }

                if (type == "RA") normalisedInput /= 15.;  // Convert to hours.
            } else if ((type == "DEC") || (type == "GLAT")) {
                if (normalisedInput < 0.) sign = "-";
                else sign = "+";
            } else { // UNKNOWN TYPE -- DEFAULT TO RA.
                std::cerr << "WARNING <decToDMS> : Unknown axis type ("
                              << type << "). Defaulting to using RA.\n";

                while (normalisedInput < 0.) { normalisedInput += 360.; }

                while (normalisedInput > 360.) { normalisedInput -= 360.; }

                normalisedInput /= 15.;
            }

            normalisedInput = fabs(normalisedInput);

            int secondWidth = 2;

            if (secondPrecision > 0) secondWidth += 1 + secondPrecision;

            double dec_abs = normalisedInput;
            int hourOrDeg = int(dec_abs);
            int min = int(fmod(dec_abs, 1.) * 60.);
            const double onemin = 1. / 60.;
            double sec = fmod(dec_abs, onemin) * 3600.;

            if (fabs(sec - 60.) < 1.e-10) { // to prevent rounding errors stuffing things up
                sec = 0.;
                min++;
            } else if (sec > 60.) {
                sec -= 60.;
                min++;
            }

            if (min == 60) {
                min = 0;
                hourOrDeg++;
            }

            if (type == "RA") hourOrDeg = hourOrDeg % 24;
            else if (type == "GLON") hourOrDeg = hourOrDeg % 360;
            else if (type == "GLAT" || type == "DEC") hourOrDeg = ((hourOrDeg + 90) % 180) - 90;

            std::stringstream output;
            output.setf(std::ios::fixed);
            output << sign
                << std::setw(degSize) << std::setfill('0') << std::setprecision(0)
                << fabs(hourOrDeg) << separator
                << std::setw(2) << std::setfill('0') << std::setprecision(0)
                << min  << separator ;
            output << std::setw(secondWidth) << std::setfill('0')
                << std::setprecision(secondPrecision) << sec;
            return output.str();
        }


        double angularSeparation(const std::string ra1, const std::string dec1,
                                 const std::string ra2, const std::string dec2)
        {
            /// @details
            /// Calculates the angular separation between two sky positions,
            /// given as strings for RA and DEC. Uses the function
            /// angularSeparation(double,double,double,double).
            /// @param ra1 The right ascension for the first position.
            /// @param dec1 The declination for the first position.
            /// @param ra2 The right ascension for the second position.
            /// @param dec2 The declination for the second position.
            /// @return The angular separation in degrees.
            ///
            if ((ra1 == ra2) && (dec1 == dec2))
                return 0.;
            else {
                double sep = angularSeparation(
                                 dmsToDec(ra1) * 15.,
                                 dmsToDec(dec1),
                                 dmsToDec(ra2) * 15.,
                                 dmsToDec(dec2)
                             );
                return sep;
            }
        }

        double angularSeparation(double ra1, double dec1, double ra2, double dec2)
        {
            /// @details
            /// Calculates the angular separation between two sky positions,
            /// where RA and DEC are given in decimal degrees.
            /// @param ra1 The right ascension for the first position.
            /// @param dec1 The declination for the first position.
            /// @param ra2 The right ascension for the second position.
            /// @param dec2 The declination for the second position.
            /// @return The angular separation in degrees.
            ///
            double r1, d1, r2, d2;
            r1 = ra1  * M_PI / 180.;
            d1 = dec1 * M_PI / 180.;
            r2 = ra2  * M_PI / 180.;
            d2 = dec2 * M_PI / 180.;
            long double angsep = cos(r1 - r2) * cos(d1) * cos(d2) + sin(d1) * sin(d2);
            return acosl(angsep)*180. / M_PI;
        }


        void equatorialToGalactic(double ra, double dec, double &gl, double &gb)
        {
            /// @details
            /// Converts an equatorial (ra,dec) position to galactic
            /// coordinates. The equatorial position is assumed to be J2000.0.
            ///
            /// @param ra Right Ascension, J2000.0
            /// @param dec Declination, J2000.0
            /// @param gl Galactic Longitude. Returned value.
            /// @param gb Galactic Latitude. Returned value.
            ///
            const double NGP_RA = 192.859508 * M_PI / 180.;
            const double NGP_DEC = 27.128336 * M_PI / 180.;
            const double ASC_NODE = 32.932;
            double deltaRA = ra * M_PI / 180. - NGP_RA;
            double d = dec     * M_PI / 180.;
            double sinb = cos(d) * cos(NGP_DEC) * cos(deltaRA) + sin(d) * sin(NGP_DEC);
            gb = asin(sinb);
            // The l in sinl and cosl here is really gl-ASC_NODE
            double sinl = (sin(d) * cos(NGP_DEC) - cos(d) * cos(deltaRA) * sin(NGP_DEC)) / cos(gb);
            double cosl = cos(d) * sin(deltaRA) / cos(gb);
            gl = atan(fabs(sinl / cosl));

            // atan of the abs.value of the ratio returns a value between 0 and 90 degrees.
            // Need to correct the value of l according to the correct quandrant it is in.
            // This is worked out using the signs of sinl and cosl
            if (sinl > 0) {
                if (cosl > 0) gl = gl;
                else       gl = M_PI - gl;
            } else {
                if (cosl > 0) gl = 2.*M_PI - gl;
                else       gl = M_PI + gl;
            }

            // Find the correct values of the lat & lon in degrees.
            gb = asin(sinb) * 180. / M_PI;
            gl = gl * 180. / M_PI + ASC_NODE;
        }




    }
}
