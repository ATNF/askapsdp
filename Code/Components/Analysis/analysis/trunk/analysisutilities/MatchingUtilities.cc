/// @file
///
/// Provides base class for handling the matching of lists of points
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

#include <analysisutilities/MatchingUtilities.h>
#include <patternmatching/GrothTriangles.h>
#include <patternmatching/Matcher.h>
#include <analysisutilities/AnalysisUtilities.h>

#include <duchamp/fitsHeader.hh>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <string>
#include <math.h>

///@brief Where the log messages go.
ASKAP_LOGGER(logger, ".matching");

namespace askap {

    namespace analysis {


      std::vector<matching::Point> getSrcPixList(std::ifstream &fin, duchamp::FitsHeader &header, std::string posType, double radius, std::string fluxMethod, std::string fluxUseFit)
        {
            /// @details Read in a list of points from a duchamp-Summary.txt
            /// file (that is, a summary file produced by cduchamp). The
            /// base positions are used to convert each point's position
            /// into an offset in arcsec. The ID of each point is generated
            /// from the object number in the list, plus the ra and dec,
            /// e.g. 2_12:34:56.78_-45:34:23.12
            /// @param fin The file stream to read from
            /// @param raBaseStr The base right ascension, in string form, e.g. 12:23:34.5
            /// @param decBaseStr The base right ascension, in string form, e.g. -12:23:34.57
            std::vector<matching::Point> pixlist;
            std::string raS, decS, sdud, id;
            double flux, peakflux, iflux1, iflux2, pflux1, pflux2, maj, min, pa, chisq, rms, noise;
            int nfree, ndof, npixfit, npixobj;

	    double *wcs = new double[3];
	    double *pix = new double[3];
	    wcs[2] = header.specToVel(0.);

            char line[501];
            fin.getline(line, 500);
            fin.getline(line, 500);

	    ASKAPLOG_DEBUG_STR(logger, "About to read source pixel list");

            // now at start of object list
            while (fin >> id >> raS >> decS >> iflux1 >> pflux1 >> iflux2 >> pflux2 >> maj >> min >> pa >> chisq >> noise >> rms >> nfree >> ndof >> npixfit >> npixobj,
                    !fin.eof()) {
                if (fluxUseFit == "no") {
                    flux = iflux1;
                    peakflux = pflux1;
                } else if (fluxUseFit == "yes") {
                    flux = iflux2;
                    peakflux = pflux2;
                } else {
                    //NOTE: was if(fluxUseFit=="best") but taking "best" to be the default
                    if (iflux2 > 0) flux = iflux2;
                    else flux = iflux1;

                    if (pflux2 > 0) peakflux = pflux2;
                    else peakflux = pflux1;
                }

                id += "_" + raS + "_" + decS;
		ASKAPLOG_DEBUG_STR(logger, id << " " << peakflux);

                std::stringstream ss;
		if(posType == "dms"){
		  wcs[0] = analysis::dmsToDec(raS) * 15.;
		  wcs[1] = analysis::dmsToDec(decS);
		}
		else if(posType == "deg"){
		  wcs[0] = atof(raS.c_str());
		  wcs[1] = atof(decS.c_str());
		}
		else
		  ASKAPTHROW(AskapError, "Unknown position type in getSrcPixList: " << posType);

		if(header.wcsToPix(wcs,pix)){
		  ASKAPLOG_ERROR_STR(logger, "getSrcPixList: Conversion error... source ID=" << id 
				     <<", wcs=("<< std::setprecision(6) << wcs[0]<<","<< std::setprecision(6) << wcs[1]<<")");
		}
		
//                 if (radius < 0 || (radius > 0 && hypot(pix[0],pix[1]) < radius*60.)) {
                if (radius < 0 || (radius > 0 && hypot(pix[0],pix[1]) < radius*60.)) {
		  matching::Point pt(pix[0], pix[1], peakflux, id, maj, min, pa);
                    pt.setStuff(chisq, noise, rms, nfree, ndof, npixfit, npixobj, flux);
                    pixlist.push_back(pt);
                }
            }

	    delete [] wcs;
	    delete [] pix;

            stable_sort(pixlist.begin(), pixlist.end());
            reverse(pixlist.begin(), pixlist.end());
            return pixlist;
        }

      std::vector<matching::Point> getPixList(std::ifstream &fin, duchamp::FitsHeader &header, std::string posType, double radius)
        {
            /// @details Reads in a list of points from a file, to serve as
            /// a reference list. The file should have six columns: ra, dec,
            /// flux, major axis, minor axis, position angle. The ra and dec
            /// should be in string form: 12:23:34.43 etc.The base positions
            /// are used to convert each point's position into an offset in
            /// arcsec. The ID of each point is generated from the object
            /// number in the list, plus the ra and dec,
            /// e.g. 2_12:34:56.78_-45:34:23.12 @param fin The file stream
            /// to read from @param raBaseStr The base right ascension, in
            /// string form, e.g. 12:23:34.5 @param decBaseStr The base
            /// right ascension, in string form, e.g. -12:23:34.57
            std::vector<matching::Point> pixlist;
            std::string raS, decS, id;
//             double raBase = analysis::dmsToDec(raBaseStr) * 15.;
//             double decBase = analysis::dmsToDec(decBaseStr);
            double flux, maj, min, pa;
            int ct = 1;

	    double *wcs = new double[3];
	    double *pix = new double[3];
	    wcs[2] = header.specToVel(0.);

            while (fin >> raS >> decS >> flux >> maj >> min >> pa,
                    !fin.eof()) {
                std::stringstream ss;
                ss << ct++;
		if(posType == "dms"){
		  wcs[0] = analysis::dmsToDec(raS) * 15.;
		  wcs[1] = analysis::dmsToDec(decS);
		}
		else if(posType == "deg"){
		  wcs[0] = atof(raS.c_str());
		  wcs[1] = atof(decS.c_str());
		}
		else
		  ASKAPTHROW(AskapError, "Unknown position type in getRefPixList: " << posType);
 //                id = ss.str() + "_" + analysis::degToDMS(wcs[0],"RA") + "_" + analysis::degToDMS(wcs[1],"DEC");
               id = ss.str() + "_" + analysis::decToDMS(wcs[0],"RA") + "_" + analysis::decToDMS(wcs[1],"DEC");

		if(header.wcsToPix(wcs,pix)){
		  ASKAPLOG_ERROR_STR(logger, "getPixList: Conversion error... source ID=" << id 
				     <<", wcs=("<< std::setprecision(6) << wcs[0]<<","<< std::setprecision(6) << wcs[1]<<")");
		}

//                 if (radius < 0 || (radius > 0 && hypot(xpt, ypt) < radius*60.)) {
                if (radius < 0 || (radius > 0 && hypot(pix[0], pix[1]) < radius*60.)) {
		  matching::Point pt(pix[0],pix[1], flux, id, maj, min, pa);
                    pixlist.push_back(pt);
                }
            }

	    delete [] wcs;
	    delete [] pix;

            stable_sort(pixlist.begin(), pixlist.end());
            reverse(pixlist.begin(), pixlist.end());
            return pixlist;
        }

      std::vector<matching::Point> getSrcPixList(std::ifstream &fin, std::string raBaseStr, std::string decBaseStr, std::string posType, double radius, std::string fluxMethod, std::string fluxUseFit)
        {
            /// @details Read in a list of points from a duchamp-Summary.txt
            /// file (that is, a summary file produced by cduchamp). The
            /// base positions are used to convert each point's position
            /// into an offset in arcsec. The ID of each point is generated
            /// from the object number in the list, plus the ra and dec,
            /// e.g. 2_12:34:56.78_-45:34:23.12
            /// @param fin The file stream to read from
            /// @param raBaseStr The base right ascension, in string form, e.g. 12:23:34.5
            /// @param decBaseStr The base right ascension, in string form, e.g. -12:23:34.57
            std::vector<matching::Point> pixlist;
            std::string raS, decS, sdud, id;
            double raBase = analysis::dmsToDec(raBaseStr) * 15.;
            double decBase = analysis::dmsToDec(decBaseStr);
            double xpt, ypt, ra, dec, flux, peakflux, iflux1, iflux2, pflux1, pflux2, maj, min, pa, chisq, rms, noise;
            int nfree, ndof, npixfit, npixobj;
            char line[501];
            fin.getline(line, 500);
            fin.getline(line, 500);

	    ASKAPLOG_DEBUG_STR(logger, "About to read source pixel list");

            // now at start of object list
            while (fin >> id >> raS >> decS >> iflux1 >> pflux1 >> iflux2 >> pflux2 >> maj >> min >> pa >> chisq >> noise >> rms >> nfree >> ndof >> npixfit >> npixobj,
                    !fin.eof()) {
                if (fluxUseFit == "no") {
                    flux = iflux1;
                    peakflux = pflux1;
                } else if (fluxUseFit == "yes") {
                    flux = iflux2;
                    peakflux = pflux2;
                } else {
                    //NOTE: was if(fluxUseFit=="best") but taking "best" to be the default
                    if (iflux2 > 0) flux = iflux2;
                    else flux = iflux1;

                    if (pflux2 > 0) peakflux = pflux2;
                    else peakflux = pflux1;
                }

                id += "_" + raS + "_" + decS;
		ASKAPLOG_DEBUG_STR(logger, id << " " << peakflux);

                std::stringstream ss;
		if(posType == "dms"){
		  ra = analysis::dmsToDec(raS) * 15.;
		  dec = analysis::dmsToDec(decS);
		}
		else if(posType == "deg"){
		  ra = atof(raS.c_str());
		  dec = atof(decS.c_str());
		}
		else
		  ASKAPTHROW(AskapError, "Unknown position type in getSrcPixList: " << posType);

		
		
                xpt = analysis::angularSeparation(ra, decBase, raBase, decBase) * 3600.;

                if (ra > raBase) xpt *= -1.;

                //    ypt = angularSeparation(raBase,dec, raBase,decBase) * 3600.;
                ypt = (dec - decBase) * 3600.;

                if (radius < 0 || (radius > 0 && hypot(xpt, ypt) < radius*60.)) {
                    matching::Point pix(xpt, ypt, peakflux, id, maj, min, pa);
                    pix.setStuff(chisq, noise, rms, nfree, ndof, npixfit, npixobj, flux);
                    pixlist.push_back(pix);
                }
            }

            stable_sort(pixlist.begin(), pixlist.end());
            reverse(pixlist.begin(), pixlist.end());
            return pixlist;
        }

      std::vector<matching::Point> getPixList(std::ifstream &fin, std::string raBaseStr, std::string decBaseStr, std::string posType, double radius)
        {
            /// @details Reads in a list of points from a file, to serve as
            /// a reference list. The file should have six columns: ra, dec,
            /// flux, major axis, minor axis, position angle. The ra and dec
            /// should be in string form: 12:23:34.43 etc.The base positions
            /// are used to convert each point's position into an offset in
            /// arcsec. The ID of each point is generated from the object
            /// number in the list, plus the ra and dec,
            /// e.g. 2_12:34:56.78_-45:34:23.12 @param fin The file stream
            /// to read from @param raBaseStr The base right ascension, in
            /// string form, e.g. 12:23:34.5 @param decBaseStr The base
            /// right ascension, in string form, e.g. -12:23:34.57
            std::vector<matching::Point> pixlist;
            std::string raS, decS, id;
            double raBase = analysis::dmsToDec(raBaseStr) * 15.;
            double decBase = analysis::dmsToDec(decBaseStr);
            double ra, dec, xpt, ypt, flux, maj, min, pa;
            int ct = 1;

            while (fin >> raS >> decS >> flux >> maj >> min >> pa,
                    !fin.eof()) {
                std::stringstream ss;
                ss << ct++;
		if(posType == "dms"){
		  ra = analysis::dmsToDec(raS) * 15.;
		  dec = analysis::dmsToDec(decS);
		}
		else if(posType == "deg"){
		  ra = atof(raS.c_str());
		  dec = atof(decS.c_str());
		}
		else
		  ASKAPTHROW(AskapError, "Unknown position type in getRefPixList: " << posType);
		//                id = ss.str() + "_" + analysis::degToDMS(ra,"RA") + "_" + analysis::degToDMS(dec,"DEC");
                id = ss.str() + "_" + analysis::decToDMS(ra,"RA") + "_" + analysis::decToDMS(dec,"DEC");
                xpt = analysis::angularSeparation(ra, decBase, raBase, decBase) * 3600.;

                if (ra > raBase) xpt *= -1.;

                //    ypt = angularSeparation(raBase,dec, raBase,decBase) * 3600.;
                ypt = (dec - decBase) * 3600.;

                if (radius < 0 || (radius > 0 && hypot(xpt, ypt) < radius*60.)) {
                    matching::Point pix(xpt, ypt, flux, id, maj, min, pa);
                    pixlist.push_back(pix);
                }
            }

            stable_sort(pixlist.begin(), pixlist.end());
            reverse(pixlist.begin(), pixlist.end());
            return pixlist;
        }


        std::vector<matching::Point> trimList(std::vector<matching::Point> &inputList, const unsigned int maxSize)
        {
            /// @details The list of points is sorted by flux, and only the
            /// maxSize highest-flux points are returned.
            /// @param inputList List of points to be trimmed
            /// @param maxSize Number of points to be returned. Defaults to matching::maxSizePointList
            /// @return The maxSize highest-flux points, in flux order.
            std::vector<matching::Point> outList = inputList;
            std::sort(outList.begin(), outList.end()); // sort by flux, ascending order
            std::reverse(outList.begin(), outList.end());

//       std::vector<matching::Point>::reverse_iterator pt;
//       for(pt=outList.rbegin(); pt!=outList.rend() && outList.size()<maxSize; pt++)
//  outList.push_back(*pt);
            if (outList.size() > maxSize) {
                std::vector<matching::Point>::iterator pt = outList.begin() + maxSize;

                while (pt != outList.end()) outList.erase(pt);
            }

            return outList;
        }


    }

}
