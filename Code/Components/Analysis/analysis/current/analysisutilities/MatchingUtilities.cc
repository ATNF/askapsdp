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
#include <duchamp/Utils/utils.hh>

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


        std::vector<matching::Point> getSrcPixList(std::ifstream &fin, duchamp::FitsHeader &header, std::string raBaseStr, std::string decBaseStr, std::string posType, double radius, std::string fluxMethod, std::string fluxUseFit)
        {
            /// @details Read in a list of points from a duchamp-Summary.txt
            /// file (that is, a summary file produced by cduchamp). The
            /// base positions are used to convert each point's position
            /// into an offset in arcsec. The ID of each point is generated
            /// from the object number in the list, plus the ra and dec,
            /// e.g. 2_12:34:56.78_-45:34:23.12
            /// @param fin The file stream to read from
            /// @param header The FitsHeader object defining the WCS transformations
            /// @param raBaseStr The base right ascension, in string form, e.g. 12:23:34.5
            /// @param decBaseStr The base right ascension, in string form, e.g. -12:23:34.57
            /// @param posType The type of position given in the stream: "dms" (12:23:45) or "deg" (12.3958333)
            /// @param radius The maximum radius from the base position within which to keep objects. If negative, everything is kept.
            /// @param fluxMethod Which flux value to use: either "peak" or "integrated". Not used at the moment.
            /// @param fluxUseFit Whether to use the fitted value of the flux. Can be either "yes", "no", or "best". If "best", we use the fitted flux whenever that value is >0 (taken to mean a fit was made successfully).
            /// @return A list of sources from the file
            std::vector<matching::Point> pixlist;
            std::string raS, decS, sdud, id,name;
            double flux, peakflux, iflux1, iflux2, pflux1, pflux2, maj, min, pa, majD, minD, paD, chisq, rmsfit, noise, alpha, beta;
            int nfree, ndof, npixfit, npixobj, guess;

            double *wld = new double[3];
            double *pix = new double[3];
            wld[2] = header.specToVel(0.);

            // Convert the base position
            wld[0] = analysis::dmsToDec(raBaseStr) * 15.;
            wld[1] = analysis::dmsToDec(decBaseStr);
	    //	    ASKAPLOG_DEBUG_STR(logger, "Converting position ("<<raBaseStr<<","<<decBaseStr<<") or world coords ("<<wld[0]<<","<<wld[1]<<")");
            header.wcsToPix(wld, pix);
            double xBase = pix[0];
            double yBase = pix[1];
	    //	    ASKAPLOG_DEBUG_STR(logger, "Got position ("<<pix[0]<<","<<pix[1]<<")");

            char line[501];
            fin.getline(line, 500);
            fin.getline(line, 500);

            ASKAPLOG_DEBUG_STR(logger, "About to read source pixel list");

            // now at start of object list
            while (fin >> id >> name >> raS >> decS >> iflux1 >> pflux1 >> iflux2 >> pflux2 >> maj >> min >> pa >> majD >> minD >> paD >> alpha >> beta >> chisq >> noise >> rmsfit >> nfree >> ndof >> npixfit >> npixobj >> guess,
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

		//                id += "_" + raS + "_" + decS;
		id += "_" + name;
                // ASKAPLOG_DEBUG_STR(logger, id << " " << peakflux);

                std::stringstream ss;

                if (posType == "dms") {
                    wld[0] = analysis::dmsToDec(raS) * 15.;
                    wld[1] = analysis::dmsToDec(decS);
                } else if (posType == "deg") {
                    wld[0] = atof(raS.c_str());
                    wld[1] = atof(decS.c_str());
                } else
                    ASKAPTHROW(AskapError, "Unknown position type in getSrcPixList: " << posType);

		//		wcsprt(header.getWCS());
		//		ASKAPLOG_DEBUG_STR(logger, "Converting world coords ("<<wld[0]<<","<<wld[1]<<")");
                if (header.wcsToPix(wld, pix)) {
                    ASKAPLOG_ERROR_STR(logger, "getSrcPixList: Conversion error... source ID=" << id << ": " 
				       << std::setprecision(6) << wld[0] << " --> " << pix[0] << " and " 
				       << std::setprecision(6) << wld[1] << " --> " << pix[1]);
                }
		//		ASKAPLOG_DEBUG_STR(logger, "... to pixel coords ("<<pix[0]<<","<<pix[1]<<")");
		    

                if (radius < 0 || (radius > 0 && hypot(pix[0] - xBase, pix[1] - yBase) < radius*60.)) {
                    matching::Point pt(pix[0], pix[1], peakflux, id, maj, min, pa,alpha,beta);
                    pt.setStuff(chisq, noise, rmsfit, nfree, ndof, npixfit, npixobj, flux);
                    pixlist.push_back(pt);
                }
            }

            delete [] wld;
            delete [] pix;

            stable_sort(pixlist.begin(), pixlist.end());
            reverse(pixlist.begin(), pixlist.end());
            return pixlist;
        }

        std::vector<matching::Point> getPixList(std::ifstream &fin, duchamp::FitsHeader &header, std::string raBaseStr, std::string decBaseStr, std::string posType, double radius)
        {
            /// @details Reads in a list of points from a file, to serve as
            /// a reference list. The file should have six columns: ra, dec,
            /// flux, major axis, minor axis, position angle. The ra and dec
            /// should be in string form: 12:23:34.43 etc.The base positions
            /// are used to convert each point's position into an offset in
            /// arcsec. The ID of each point is generated from the object
            /// number in the list, plus the ra and dec,
            /// e.g. 2_12:34:56.78_-45:34:23.12
            ///
            /// @param fin The file stream to read from
            /// @param header The FitsHeader object containing information on the WCS system (for conversion from WCS to pixel positions)
            /// @param raBaseStr The base right ascension, in string form, e.g. 12:23:34.5
            /// @param decBaseStr The base right ascension, in string form, e.g. -12:23:34.57
            /// @param posType The type of position, either "dms" (12:34:56 format) or "deg" (decimal degrees)
            /// @param radius The search radius within which points are kept. Objects
            /// @return A list of sources from the file
            std::vector<matching::Point> pixlist;
            std::string line, raS, decS, id;
            double flux, maj, min, pa, alpha, beta;
            int ct = 1;

            double *wld = new double[3];
            double *pix = new double[3];
            wld[2] = header.specToVel(0.);

            // Convert the base position
            wld[0] = analysis::dmsToDec(raBaseStr) * 15.;
            wld[1] = analysis::dmsToDec(decBaseStr);
	    //	    ASKAPLOG_DEBUG_STR(logger, "Converting position ("<<raBaseStr<<","<<decBaseStr<<") or world coords ("<<wld[0]<<","<<wld[1]<<")");
            header.wcsToPix(wld, pix);
            double xBase = pix[0];
            double yBase = pix[1];
	    //	    ASKAPLOG_DEBUG_STR(logger, "Got position ("<<pix[0]<<","<<pix[1]<<")");

            while (getline(fin, line),
                    !fin.eof()) {
                if (line[0] != '#') {
                    std::stringstream ss(line);
                    ss >> raS >> decS >> flux >> alpha >> beta >> maj >> min >> pa;

                    if (posType == "dms") {
                        wld[0] = analysis::dmsToDec(raS) * 15.;
                        wld[1] = analysis::dmsToDec(decS);
                    } else if (posType == "deg") {
                        wld[0] = atof(raS.c_str());
                        wld[1] = atof(decS.c_str());
                    } else
                        ASKAPTHROW(AskapError, "Unknown position type in getRefPixList: " << posType);

                    std::stringstream idString;
                    idString << ct++ << "_" << analysis::decToDMS(wld[0], "RA") << "_" << analysis::decToDMS(wld[1], "DEC");

		    //		    wcsprt(header.getWCS());
		    //		    ASKAPLOG_DEBUG_STR(logger, "Converting world coords ("<<wld[0]<<","<<wld[1]<<")");
                    if (header.wcsToPix(wld, pix)) {
                        ASKAPLOG_ERROR_STR(logger, "getPixList: Conversion error... source ID=" << idString.str()
                                               << ", wld=(" << std::setprecision(6) << wld[0] << "," << std::setprecision(6) << wld[1] << "), line = " << line);
                    }
		    //		    ASKAPLOG_DEBUG_STR(logger, "... to pixel coords ("<<pix[0]<<","<<pix[1]<<")");

                    if (radius < 0 || (radius > 0 && hypot(pix[0] - xBase, pix[1] - yBase) < radius*60.)) {
                        matching::Point pt(pix[0], pix[1], flux, idString.str(), maj, min, pa,alpha,beta);
                        pixlist.push_back(pt);
                    }
                }
            }

            delete [] wld;
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
            /// @param posType The type of position given in the stream: "dms" (12:23:45) or "deg" (12.3958333)
            /// @param radius The maximum radius from the base position within which to keep objects. If negative, everything is kept.
            /// @param fluxMethod Which flux value to use: either "peak" or "integrated". Not used at the moment.
            /// @param fluxUseFit Whether to use the fitted value of the flux. Can be either "yes", "no", or "best". If "best", we use the fitted flux whenever that value is >0 (taken to mean a fit was made successfully).
            /// @return A list of sources from the file
            std::vector<matching::Point> pixlist;
            std::string raS, decS, sdud, id, name;
            double raBase = analysis::dmsToDec(raBaseStr) * 15.;
            double decBase = analysis::dmsToDec(decBaseStr);
            double xpt, ypt, ra, dec, flux, peakflux, iflux1, iflux2, pflux1, pflux2, maj, min, pa, majD, minD, paD, chisq, rmsfit, noise, alpha, beta;
            int nfree, ndof, npixfit, npixobj,guess;
            char line[501];
            fin.getline(line, 500);
            fin.getline(line, 500);

            ASKAPLOG_DEBUG_STR(logger, "About to read source pixel list");

            // now at start of object list
            while (fin >> id >> name >> raS >> decS >> iflux1 >> pflux1 >> iflux2 >> pflux2 >> maj >> min >> pa >> majD >> minD >> paD >> alpha >> beta >> chisq >> noise >> rmsfit >> nfree >> ndof >> npixfit >> npixobj >> guess,
		   //            while (fin >> id >> raS >> decS >> iflux1 >> pflux1 >> iflux2 >> pflux2 >> maj >> min >> pa >> alpha >> beta >> chisq >> noise >> rms >> nfree >> ndof >> npixfit >> npixobj,
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

		//                id += "_" + raS + "_" + decS;
                id += "_" + name;
                ASKAPLOG_DEBUG_STR(logger, id << " " << peakflux);

                std::stringstream ss;

                if (posType == "dms") {
                    ra = analysis::dmsToDec(raS) * 15.;
                    dec = analysis::dmsToDec(decS);
                } else if (posType == "deg") {
                    ra = atof(raS.c_str());
                    dec = atof(decS.c_str());
                } else
                    ASKAPTHROW(AskapError, "Unknown position type in getSrcPixList: " << posType);



                xpt = analysis::angularSeparation(ra, decBase, raBase, decBase) * 3600.;

                if (ra > raBase) xpt *= -1.;

                ypt = (dec - decBase) * 3600.;

                if (radius < 0 || (radius > 0 && hypot(xpt, ypt) < radius*60.)) {
                    matching::Point pix(xpt, ypt, peakflux, id, maj, min, pa,alpha,beta);
                    pix.setStuff(chisq, noise, rmsfit, nfree, ndof, npixfit, npixobj, flux);
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
            /// e.g. 2_12:34:56.78_-45:34:23.12
            ///
            /// @param fin The file stream to read from
            /// @param raBaseStr The base right ascension, in string form, e.g. 12:23:34.5
            /// @param decBaseStr The base right ascension, in string form, e.g. -12:23:34.57
            /// @param posType The type of position, either "dms" (12:34:56 format) or "deg" (decimal degrees)
            /// @param radius The search radius within which points are kept. Objects
            /// @return A list of sources from the file

            std::vector<matching::Point> pixlist;
            std::string line, raS, decS, id;
            double raBase = analysis::dmsToDec(raBaseStr) * 15.;
            double decBase = analysis::dmsToDec(decBaseStr);
            double ra, dec, xpt, ypt, flux, maj, min, pa, alpha, beta;
            int ct = 1;

            while (getline(fin, line),
                    !fin.eof()) {
                if (line[0] != '#') {
                    std::stringstream ss(line);
                    ss >> raS >> decS >> flux >> alpha >> beta >> maj >> min >> pa;

                    if (posType == "dms") {
                        ra = analysis::dmsToDec(raS) * 15.;
                        dec = analysis::dmsToDec(decS);
                    } else if (posType == "deg") {
                        ra = atof(raS.c_str());
                        dec = atof(decS.c_str());
                    } else
                        ASKAPTHROW(AskapError, "Unknown position type in getRefPixList: " << posType);

                    std::stringstream idString;
                    idString << ct++ << "_" << analysis::decToDMS(ra, "RA") << "_" << analysis::decToDMS(dec, "DEC");
                    xpt = analysis::angularSeparation(ra, decBase, raBase, decBase) * 3600.;

                    if (ra > raBase) xpt *= -1.;

                    ypt = (dec - decBase) * 3600.;

                    if (radius < 0 || (radius > 0 && hypot(xpt, ypt) < radius*60.)) {
                        matching::Point pix(xpt, ypt, flux, idString.str(), maj, min, pa,alpha,beta);
                        pixlist.push_back(pix);
                    }
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

            if (outList.size() > maxSize) {
                std::vector<matching::Point>::iterator pt = outList.begin() + maxSize;

                while (pt != outList.end()) outList.erase(pt);
            }

            return outList;
        }


      std::vector<matching::Point> crudeMatchList(std::vector<matching::Point> &reflist, std::vector<matching::Point> &srclist, float maxOffset)
      {
	std::vector<matching::Point>::iterator ref,src;
	std::vector<matching::Point> newreflist;
	for(src=srclist.begin();src<srclist.end();src++){
	  
	  for(ref=reflist.begin();ref<reflist.end();ref++){
	    if(src->sep(*ref)<maxOffset) newreflist.push_back(*ref);
	  }

	}

	return newreflist;

      }


    }

}
