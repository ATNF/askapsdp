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
#include <patternmatching/Triangle.h>
#include <patternmatching/Point.h>
#include <patternmatching/Matcher.h>

#include <coordutils/PositionUtilities.h>

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
            double peakflux, iflux1, iflux2, pflux1, pflux2, maj, min, pa, majD, minD, paD, chisq, rmsfit, noise, alpha, beta;
            int nfree, ndof, npixfit, npixobj, guess;

            // Convert the base position
	    double ra,dec,zworld;
            ra = analysisutilities::dmsToDec(raBaseStr) * 15.;
            dec = analysisutilities::dmsToDec(decBaseStr);
            zworld = header.specToVel(0.);
            double xBase, yBase,zBase,x,y,z;
	    header.wcsToPix(ra,dec,zworld,xBase,yBase,zBase);

	    std::string line;

            ASKAPLOG_DEBUG_STR(logger, "About to read source pixel list");

            // now at start of object list
            while (getline(fin, line),
		   !fin.eof()) {

	      if(line[0] != '#'){

	      std::stringstream inputline(line);

	      inputline >> id >> name >> raS >> decS >> iflux1 >> pflux1 >> iflux2 >> pflux2 >> maj >> min >> pa >> majD >> minD >> paD >> alpha >> beta >> chisq >> noise >> rmsfit >> nfree >> ndof >> npixfit >> npixobj >> guess;
	      
	      if (fluxUseFit == "no") {
                    peakflux = pflux1;
                } else if (fluxUseFit == "yes") {
                    peakflux = pflux2;
                } else {
                    //NOTE: was if(fluxUseFit=="best") but taking "best" to be the default

                    if (pflux2 > 0) peakflux = pflux2;
                    else peakflux = pflux1;
                }

		//                id += "_" + raS + "_" + decS;
		id += "_" + name;
                // ASKAPLOG_DEBUG_STR(logger, id << " " << peakflux);

                std::stringstream ss;

		//		ASKAPLOG_DEBUG_STR(logger, "Read RA, Dec = " << raS << " " << decS);
                if (posType == "dms") {
                    ra = analysisutilities::dmsToDec(raS) * 15.;
                    dec = analysisutilities::dmsToDec(decS);
                } else if (posType == "deg") {
                    ra = atof(raS.c_str());
                    dec = atof(decS.c_str());
                } else
                    ASKAPTHROW(AskapError, "Unknown position type in getSrcPixList: " << posType);

		//		wcsprt(header.getWCS());
		//		ASKAPLOG_DEBUG_STR(logger, "Converting world coords ("<<ra<<","<<dec<<")");
                if (header.wcsToPix(ra,dec,zworld,x,y,z)) {
                    ASKAPLOG_ERROR_STR(logger, "getSrcPixList: Conversion error... source ID=" << id << ": " 
				       << std::setprecision(6) << ra << " --> " << x << " and " 
				       << std::setprecision(6) << dec << " --> " << y);
                }
		//		ASKAPLOG_DEBUG_STR(logger, "... to pixel coords ("<<x<<","<<y<<")");
		    

                if (radius < 0 || (radius > 0 && hypot(x - xBase, y - yBase) < radius*60.)) {
                    // matching::Point pt(pix[0], pix[1], peakflux, id, maj, min, pa,alpha,beta);
		    matching::Point pt(xBase, yBase, peakflux, id);
                    pixlist.push_back(pt);
                }

	      }

            }

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

            // Convert the base position
	    double ra,dec,zworld;
            ra = analysisutilities::dmsToDec(raBaseStr) * 15.;
            dec = analysisutilities::dmsToDec(decBaseStr);
	    zworld = header.specToVel(0.);
	    //	    ASKAPLOG_DEBUG_STR(logger, "Converting position ("<<raBaseStr<<","<<decBaseStr<<") or world coords ("<<ra<<","<<dec<<")");
	    double xBase,yBase,zBase,x,y,z;
            header.wcsToPix(ra,dec,zworld,xBase,yBase,zBase);
	    //	    ASKAPLOG_DEBUG_STR(logger, "Got position ("<<xBase<<","<<yBase<<")");

            while (getline(fin, line),
                    !fin.eof()) {
                if (line[0] != '#') {
                    std::stringstream ss(line);
                    ss >> raS >> decS >> flux >> alpha >> beta >> maj >> min >> pa;

                    if (posType == "dms") {
                        ra = analysisutilities::dmsToDec(raS) * 15.;
                        dec = analysisutilities::dmsToDec(decS);
                    } else if (posType == "deg") {
                        ra = atof(raS.c_str());
                        dec = atof(decS.c_str());
                    } else
                        ASKAPTHROW(AskapError, "Unknown position type in getRefPixList: " << posType);

                    std::stringstream idString;
                    idString << ct++ << "_" << analysisutilities::decToDMS(ra, "RA") << "_" << analysisutilities::decToDMS(dec, "DEC");

		    //		    wcsprt(header.getWCS());
		    //		    ASKAPLOG_DEBUG_STR(logger, "Converting world coords ("<<ra<<","<<dec<<")");
                    if (header.wcsToPix(ra,dec,zworld,x,y,z)) {
                        ASKAPLOG_ERROR_STR(logger, "getPixList: Conversion error... source ID=" << idString.str()
                                               << ", wld=(" << std::setprecision(6) << ra << "," << std::setprecision(6) << dec << "), line = " << line);
                    }
		    //		    ASKAPLOG_DEBUG_STR(logger, "... to pixel coords ("<<x<<","<<y<<")");

                    if (radius < 0 || (radius > 0 && hypot(x - xBase, y - yBase) < radius*60.)) {
		      //                        matching::Point pt(pix[0], pix[1], flux, idString.str(), maj, min, pa,alpha,beta);
                        matching::Point pt(x, y, flux, idString.str());
                        pixlist.push_back(pt);
                    }
                }
            }

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
            double raBase = analysisutilities::dmsToDec(raBaseStr) * 15.;
            double decBase = analysisutilities::dmsToDec(decBaseStr);
            double xpt, ypt, ra, dec, peakflux, iflux1, iflux2, pflux1, pflux2, maj, min, pa, majD, minD, paD, chisq, rmsfit, noise, alpha, beta;
            int nfree, ndof, npixfit, npixobj,guess;
	    std::string line;

            ASKAPLOG_DEBUG_STR(logger, "About to read source pixel list");

            // now at start of object list
            while (getline(fin, line),
		   !fin.eof()) {

	      if(line[0] != '#'){
		
		std::stringstream inputline(line);

		inputline >> id >> name >> raS >> decS >> iflux1 >> pflux1 >> iflux2 >> pflux2 >> maj >> min >> pa >> majD >> minD >> paD >> alpha >> beta >> chisq >> noise >> rmsfit >> nfree >> ndof >> npixfit >> npixobj >> guess;
		  
		  if (fluxUseFit == "no") {
                    peakflux = pflux1;
		  } else if (fluxUseFit == "yes") {
                    peakflux = pflux2;
		  } else {
                    //NOTE: was if(fluxUseFit=="best") but taking "best" to be the default
                    if (pflux2 > 0) peakflux = pflux2;
                    else peakflux = pflux1;
		  }

		//                id += "_" + raS + "_" + decS;
                id += "_" + name;
                ASKAPLOG_DEBUG_STR(logger, id << " " << peakflux);

                std::stringstream ss;
		
                if (posType == "dms") {
		  ra = analysisutilities::dmsToDec(raS) * 15.;
		  dec = analysisutilities::dmsToDec(decS);
                } else if (posType == "deg") {
		  ra = atof(raS.c_str());
		  dec = atof(decS.c_str());
                } else
		  ASKAPTHROW(AskapError, "Unknown position type in getSrcPixList: " << posType);



                xpt = analysisutilities::angularSeparation(ra, decBase, raBase, decBase) * 3600.;

                if (ra > raBase) xpt *= -1.;

                ypt = (dec - decBase) * 3600.;

                if (radius < 0 || (radius > 0 && hypot(xpt, ypt) < radius*60.)) {
		  //                    matching::Point pix(xpt, ypt, peakflux, id, maj, min, pa,alpha,beta);
                    matching::Point pix(xpt, ypt, peakflux, id);
                    pixlist.push_back(pix);
                }
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
            double raBase = analysisutilities::dmsToDec(raBaseStr) * 15.;
            double decBase = analysisutilities::dmsToDec(decBaseStr);
            double ra, dec, xpt, ypt, flux, maj, min, pa, alpha, beta;
            int ct = 1;

            while (getline(fin, line),
                    !fin.eof()) {
                if (line[0] != '#') {
                    std::stringstream ss(line);
                    ss >> raS >> decS >> flux >> alpha >> beta >> maj >> min >> pa;

                    if (posType == "dms") {
                        ra = analysisutilities::dmsToDec(raS) * 15.;
                        dec = analysisutilities::dmsToDec(decS);
                    } else if (posType == "deg") {
                        ra = atof(raS.c_str());
                        dec = atof(decS.c_str());
                    } else
                        ASKAPTHROW(AskapError, "Unknown position type in getRefPixList: " << posType);

                    std::stringstream idString;
                    idString << ct++ << "_" << analysisutilities::decToDMS(ra, "RA") << "_" << analysisutilities::decToDMS(dec, "DEC");
                    xpt = analysisutilities::angularSeparation(ra, decBase, raBase, decBase) * 3600.;

                    if (ra > raBase) xpt *= -1.;

                    ypt = (dec - decBase) * 3600.;

                    if (radius < 0 || (radius > 0 && hypot(xpt, ypt) < radius*60.)) {
		      //                        matching::Point pix(xpt, ypt, flux, idString.str(), maj, min, pa,alpha,beta);
                        matching::Point pix(xpt, ypt, flux, idString.str());
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
