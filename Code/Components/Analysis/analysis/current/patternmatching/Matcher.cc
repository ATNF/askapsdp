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

#include <patternmatching/Matcher.h>
#include <patternmatching/Triangle.h>
#include <patternmatching/Point.h>
#include <patternmatching/PointCatalogue.h>
#include <analysisutilities/MatchingUtilities.h>

#include <Common/ParameterSet.h>

#include <duchamp/fitsHeader.hh>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <utility>
#include <string>
#include <math.h>

///@brief Where the log messages go.
ASKAP_LOGGER(logger, ".matching");

using namespace askap;

namespace askap {

    namespace analysis {

        namespace matching {

            Matcher::Matcher()
            {
                this->itsMeanDx = 0.;
                this->itsMeanDy = 0.;
                this->itsRmsDx = 0.;
                this->itsRmsDy = 0.;
            }

            Matcher::~Matcher()
            {
            }

            Matcher::Matcher(const Matcher &m)
            {
                operator=(m);
            }

            Matcher& Matcher::operator=(const Matcher &m)
            {
                if (this == &m) return *this;

                this->itsFITShead = m.itsFITShead;
                this->itsSrcFile = m.itsSrcFile;
                this->itsRefFile = m.itsRefFile;
                this->itsRA = m.itsRA;
                this->itsDec = m.itsDec;
                this->itsSrcPosType = m.itsSrcPosType;
                this->itsRefPosType = m.itsRefPosType;
                this->itsRadius = m.itsRadius;
                this->itsFluxMethod = m.itsFluxMethod;
                this->itsFluxUseFit = m.itsFluxUseFit;
                this->itsSrcPixList = m.itsSrcPixList;
                this->itsRefPixList = m.itsRefPixList;
                this->itsSrcTriList = m.itsSrcTriList;
                this->itsRefTriList = m.itsRefTriList;
                this->itsMatchingTriList = m.itsMatchingTriList;
                this->itsMatchingPixList = m.itsMatchingPixList;
                this->itsEpsilon = m.itsEpsilon;
		this->itsTrimSize = m.itsTrimSize;
                this->itsMeanDx = m.itsMeanDx;
                this->itsMeanDy = m.itsMeanDy;
                this->itsRmsDx = m.itsRmsDx;
                this->itsRmsDy = m.itsRmsDy;
                this->itsNumMatch1 = m.itsNumMatch1;
                this->itsNumMatch2 = m.itsNumMatch2;
                this->itsSenseMatch = m.itsSenseMatch;
                this->itsOutputBestFile = m.itsOutputBestFile;
                this->itsOutputMissFile = m.itsOutputMissFile;
		this->itsSrcCatalogue = m.itsSrcCatalogue;
		this->itsRefCatalogue = m.itsRefCatalogue;
                return *this;
            }

            Matcher::Matcher(const LOFAR::ParameterSet& parset)
            {
                /// @details The parameter file is examined for the relevant
                /// parameters to define the input and output files, the base
                /// positions for both lists, and the epsilon value. The input
                /// files are read to obtain the source and reference point
                /// lists.
                this->itsSrcFile = parset.getString("srcFile", "");
                this->itsRefFile = parset.getString("refFile", "");
                this->itsFluxMethod = parset.getString("fluxMethod", "peak");
                this->itsFluxUseFit = parset.getString("fluxUseFit", "best");
                this->itsRA  = parset.getString("RA", "00:00:00");
                this->itsDec = parset.getString("Dec", "00:00:00");
                this->itsSrcPosType = parset.getString("srcPosType", "deg");
                this->itsRefPosType = parset.getString("refPosType", "deg");
                this->itsRadius = parset.getDouble("radius", -1.);
                this->itsEpsilon = parset.getDouble("epsilon", defaultEpsilon);
		this->itsTrimSize = parset.getInt16("trimsize",matching::maxSizePointList);
                this->itsMeanDx = 0.;
                this->itsMeanDy = 0.;
                this->itsRmsDx = 0.;
                this->itsRmsDy = 0.;
                this->itsOutputBestFile = parset.getString("matchFile", "matches.txt");
                this->itsOutputMissFile = parset.getString("missFile", "misses.txt");

		LOFAR::ParameterSet subset=parset.makeSubset("source.");
		this->itsSrcCatalogue = PointCatalogue(subset);
		subset=parset.makeSubset("reference.");
		this->itsRefCatalogue = PointCatalogue(subset);
            }

            //**************************************************************//

            void Matcher::setHeader(duchamp::FitsHeader &head)
            {
                /// @details This function takes a duchamp FitsHeader
                /// representation of the FITS header information and
                /// stores it for doing the coordinate conversions
                this->itsFITShead = head;
            }

            //**************************************************************//

            bool Matcher::readLists()
            {
                /// @details This function reads the source and reference
                /// pixel lists from the files provided. Checks are made
                /// for the validity of the files.

                bool filesOK = true;

                // if (this->itsSrcFile == "") {
                //     ASKAPTHROW(AskapError, "srcFile not defined. Cannot get pixel list!");
                //     filesOK = false;
                // }

                // if (this->itsRefFile == "") {
                //     ASKAPTHROW(AskapError, "refFile not defined. Cannot get pixel list!");
                //     filesOK = false;
                // }

                // if (filesOK) {
                //     std::ifstream fsrc(this->itsSrcFile.c_str());

                //     if (!fsrc.is_open())
                //         ASKAPTHROW(AskapError, "srcFile (" << this->itsSrcFile << ") not valid. Error opening file.");

                //     std::ifstream fref(this->itsRefFile.c_str());

                //     if (!fref.is_open())
                //         ASKAPTHROW(AskapError, "refFile (" << this->itsRefFile << ") not valid. Error opening file.");

                //     this->itsSrcPixList = getSrcPixList(fsrc, this->itsFITShead, this->itsRA, this->itsDec, this->itsSrcPosType, this->itsRadius, this->itsFluxMethod, this->itsFluxUseFit);
                //     ASKAPLOG_INFO_STR(logger, "Size of source pixel list = " << this->itsSrcPixList.size());

                //     this->itsRefPixList = getPixList(fref, this->itsFITShead, this->itsRA, this->itsDec, this->itsRefPosType, this->itsRadius);
                //     ASKAPLOG_INFO_STR(logger, "Size of reference pixel list = " << this->itsRefPixList.size());
                // } else {
                //     ASKAPLOG_WARN_STR(logger, "Not reading any pixel lists!");
                // }

		this->itsSrcCatalogue.read();
		this->itsRefCatalogue.read();
		filesOK = this->itsSrcCatalogue.pointList().size()>0 && this->itsRefCatalogue.pointList().size()>0;
		if(this->itsSrcCatalogue.pointList().size()==0)
		   ASKAPLOG_ERROR_STR(logger, "Could not read source catalogue from " << this->itsSrcCatalogue.filename());
		if(this->itsRefCatalogue.pointList().size()==0)
		   ASKAPLOG_ERROR_STR(logger, "Could not read source catalogue from " << this->itsRefCatalogue.filename());
		if(filesOK){
		  ASKAPLOG_INFO_STR(logger, "Size of source pixel list = " << this->itsSrcCatalogue.pointList().size());
		  ASKAPLOG_INFO_STR(logger, "Size of reference pixel list = " << this->itsRefCatalogue.pointList().size());
		}
		return filesOK;
            }

            //**************************************************************//

            void Matcher::fixRefList(std::vector<float> beam)
            {
                /// @details This function takes a reference list and
                /// convolves the sizes of the sources with a given beam. The
                /// relationships discussed in Wild (1970), AustJPhys 23, 113
                /// are used to combine a gaussian source with a gaussian beam.
                ///
                /// @param beam A vector containing the beam major axis, beam
                /// minor axis and beam position angle, all in degrees.
                ///
                /// @todo This treatment only deals with Gaussian
                /// sources. What if we have discs as well?
                // ASKAPLOG_INFO_STR(logger, "Beam info being used: maj=" << beam[0]*3600.
                //                       << ", min=" << beam[1]*3600. << ", pa=" << beam[2]);
                // float a1 = std::max(beam[0] * 3600., beam[1] * 3600.);
                // float b1 = std::min(beam[0] * 3600., beam[1] * 3600.);
                // float pa1 = beam[2];
                // float d1 = a1 * a1 - b1 * b1;
                // std::vector<Point>::iterator pix = this->itsRefPixList.begin();

                // for (; pix < this->itsRefPixList.end(); pix++) {
                //     double a2 = std::max(pix->majorAxis(), pix->minorAxis());
                //     double b2 = std::min(pix->majorAxis(), pix->minorAxis());
                //     double pa2 = pix->PA();
                //     double d2 = a2 * a2 - b2 * b2;
                //     double d0sq = d1 * d1 + d2 * d2 + 2. * d1 * d2 * cos(2.*(pa1 - pa2));
                //     double d0 = sqrt(d0sq);
                //     double a0sq = 0.5 * (a1 * a1 + b1 * b1 + a2 * a2 + b2 * b2 + d0);
                //     double b0sq = 0.5 * (a1 * a1 + b1 * b1 + a2 * a2 + b2 * b2 - d0);
                //     pix->setMajorAxis(sqrt(a0sq));
                //     pix->setMinorAxis(sqrt(b0sq));

                //     if (d0sq > 0) {
                //         // leave out normalisation by d0, since we will take ratios to get tan2pa0
                //         double sin2pa0 = (d1 * sin(2.*pa1) + d2 * sin(2.*pa2));
                //         double cos2pa0 = (d1 * cos(2.*pa1) + d2 * cos(2.*pa2));
                //         double pa0 = atan(fabs(sin2pa0 / cos2pa0));

                //         // atan of the absolute value of the ratio returns a value between 0 and 90 degrees.
                //         // Need to correct the value of l according to the correct quandrant it is in.
                //         // This is worked out using the signs of sinl and cosl
                //         if (sin2pa0 > 0) {
                //             if (cos2pa0 > 0) pa0 = pa0;
                //             else          pa0 = M_PI - pa0;
                //         } else {
                //             if (cos2pa0 > 0) pa0 = 2.*M_PI - pa0;
                //             else          pa0 = M_PI + pa0;
                //         }

                //         pix->setPA(pa0 / 2.);
                //     } else  pix->setPA(0.);

                // }
            }


            //**************************************************************//

            void Matcher::setTriangleLists()
            {
                /// @details The point lists are first shortened to the
                /// appropriate size by trimList(). The shortened lists are then
                /// converted into triangle lists, which are matched and
                /// trimmed.

	      this->itsSrcTriList = this->itsSrcCatalogue.triangleList();
	      if(!this->itsRefCatalogue.crudeMatch(this->itsSrcCatalogue.fullPointList(),this->itsEpsilon))
		ASKAPLOG_WARN_STR(logger, "Crude matching failed! Using full reference point list");
	      this->itsRefTriList = this->itsRefCatalogue.triangleList();
	      
	      // std::vector<Point> srclist = trimList(this->itsSrcPixList, this->itsTrimSize);
              //   ASKAPLOG_INFO_STR(logger, "Trimmed src list to " << srclist.size() << " points");
              //   // std::vector<Point> reflist = trimList(this->itsRefPixList, this->itsTrimSize);
              //   // ASKAPLOG_INFO_STR(logger, "Trimmed ref list to " << reflist.size() << " points");
              //   this->itsSrcTriList = getTriList(srclist);
	      // 	ASKAPLOG_INFO_STR(logger, "Performing crude match on reference list");
	      // 	std::vector<Point> newreflist = crudeMatchList(this->itsRefPixList, this->itsSrcPixList,5);
	      // 	// ASKAPLOG_INFO_STR(logger, "Performing crude match on trimmed reference list");
	      // 	// std::vector<Point> newreflist = crudeMatchList(this->itsRefPixList, srclist,5);
	      // 	ASKAPLOG_INFO_STR(logger, "Now have reference list of size " << newreflist.size() << " points");
	      // 	newreflist = trimList(newreflist,this->itsTrimSize);
	      // 	ASKAPLOG_INFO_STR(logger, "Reference list trimmed to " << newreflist.size() << " points");
	      // 	//                this->itsRefTriList = getTriList(reflist);
              //   this->itsRefTriList = getTriList(newreflist);

                this->itsMatchingTriList = matchLists(this->itsSrcTriList, this->itsRefTriList, this->itsEpsilon);
                trimTriList(this->itsMatchingTriList);
                ASKAPLOG_INFO_STR(logger, "Found " << this->itsMatchingTriList.size() << " matches");
            }

            //**************************************************************//

            void Matcher::findMatches()
            {
                /// @details Matching points are found via the Groth voting
                /// function vote(). The number of matches and their sense are
                /// recorded.
                this->itsNumMatch1 = 0;

                if (this->itsMatchingTriList.size() > 0) {
                    this->itsMatchingPixList = vote(this->itsMatchingTriList);
                    this->itsNumMatch1 = this->itsMatchingPixList.size();
                    ASKAPLOG_INFO_STR(logger, "After voting, have found " << this->itsMatchingPixList.size() << " matching points");
                    this->itsSenseMatch = (this->itsMatchingTriList[0].first.isClockwise() ==
                                           this->itsMatchingTriList[0].second.isClockwise());

                    if (this->itsSenseMatch)
                        ASKAPLOG_INFO_STR(logger, "The two lists have the same sense.");
                    else
                        ASKAPLOG_INFO_STR(logger, "The two lists have the opposite sense.");
                }
            }


            //**************************************************************//

            void Matcher::findOffsets()
            {
                /// @details The mean and rms offsets in the x- and
                /// y-directions are measured for the matching points.
	      // std::vector<double> dx(this->itsNumMatch1,0.),dy(this->itsNumMatch1,0.);
	      std::vector<double> dx(this->itsMatchingPixList.size(),0.),dy(this->itsMatchingPixList.size(),0.);

                // for (int i = 0; i < this->itsNumMatch1; i++) {
	      for (size_t i = 0; i < this->itsMatchingPixList.size(); i++) {
                    if (this->itsSenseMatch) {
                        dx[i] = this->itsMatchingPixList[i].first.x() - this->itsMatchingPixList[i].second.x();
                        dy[i] = this->itsMatchingPixList[i].first.y() - this->itsMatchingPixList[i].second.y();
                    } else {
                        dx[i] = this->itsMatchingPixList[i].first.x() - this->itsMatchingPixList[i].second.x();
                        dy[i] = this->itsMatchingPixList[i].first.y() + this->itsMatchingPixList[i].second.y();
                    }

                }

                this->itsMeanDx = this->itsMeanDy = 0.;

                // for (int i = 0; i < this->itsNumMatch1; i++) {
                for (size_t i = 0; i < this->itsMatchingPixList.size(); i++) {
                    this->itsMeanDx += dx[i];
                    this->itsMeanDy += dy[i];
                }

                // this->itsMeanDx /= double(this->itsNumMatch1);
                // this->itsMeanDy /= double(this->itsNumMatch1);
                this->itsMeanDx /= double(this->itsMatchingPixList.size());
                this->itsMeanDy /= double(this->itsMatchingPixList.size());

                this->itsRmsDx = this->itsRmsDy = 0.;

                // for (int i = 0; i < this->itsNumMatch1; i++) {
                for (size_t i = 0; i < this->itsMatchingPixList.size(); i++) {
                    this->itsRmsDx += (dx[i] - this->itsMeanDx) * (dx[i] - this->itsMeanDx);
                    this->itsRmsDy += (dy[i] - this->itsMeanDy) * (dy[i] - this->itsMeanDy);
                }

                // this->itsRmsDx = sqrt(this->itsRmsDx / (double(this->itsNumMatch1 - 1)));
                // this->itsRmsDy = sqrt(this->itsRmsDy / (double(this->itsNumMatch1 - 1)));
                this->itsRmsDx = sqrt(this->itsRmsDx / (double(this->itsMatchingPixList.size() - 1)));
                this->itsRmsDy = sqrt(this->itsRmsDy / (double(this->itsMatchingPixList.size() - 1)));
                std::stringstream ss;
                ss.setf(std::ios::fixed);
                ss << "Offsets between the two are dx = " << this->itsMeanDx << " +- " << this->itsRmsDx
                    << " dy = " << this->itsMeanDy << " +- " << this->itsRmsDy;
                ASKAPLOG_INFO_STR(logger, ss.str());
            }

            //**************************************************************//

            void Matcher::addNewMatches()
            {
                /// @details The source point list is scanned for points that
                /// were not initially matched, but have a reference
                /// counterpart within a certain number of epsilon values
                /// (currently set at 3). These points are added to the
                /// itsMatchingPixList, and the new total number of matches is
                /// recorded.

                if (itsNumMatch1 > 0) {

                    this->rejectMultipleMatches();
                    const float matchRadius = 3.;
                    std::vector<Point>::iterator src, ref;
                    std::vector<std::pair<Point, Point> >::iterator match;

                    // for (src = this->itsSrcPixList.begin(); src < this->itsSrcPixList.end(); src++) {
                    for (src = this->itsSrcCatalogue.fullPointList().begin(); src < this->itsSrcCatalogue.fullPointList().end(); src++) {
                        bool isMatch = false;
                        match = this->itsMatchingPixList.begin();

                        for (; match < this->itsMatchingPixList.end() && !isMatch; match++) {
                            isMatch = (src->ID() == match->first.ID());
                        }

                        if (!isMatch) {
                            float minOffset = 0.;
                            int minRef = -1;

                            for (ref = this->itsRefCatalogue.fullPointList().begin(); ref < this->itsRefCatalogue.fullPointList().end(); ref++) {
                                float offset = hypot(src->x() - ref->x() - this->itsMeanDx,
                                                     src->y() - ref->y() - this->itsMeanDy);

                                if (offset < matchRadius*this->itsEpsilon) {
                                    if ((minRef == -1) || (offset < minOffset)) {
                                        minOffset = offset;
                                        minRef = int(ref - this->itsRefCatalogue.fullPointList().begin());
                                    }
                                }
                            }

                            if (minRef >= 0) { // there was a match within errors
			      ref = this->itsRefCatalogue.fullPointList().begin() + minRef;
                                std::pair<Point, Point> newMatch(*src, *ref);
                                this->itsMatchingPixList.push_back(newMatch);
                            }
                        }
                    }

                    this->rejectMultipleMatches();
                    this->itsNumMatch2 = this->itsMatchingPixList.size();

		    ASKAPLOG_INFO_STR(logger, "Total number of matches = " << this->itsMatchingPixList.size());
                }
            }

            //**************************************************************//

            void Matcher::rejectMultipleMatches()
            {
                /// @details Objects that appear twice in the match list are
                /// examined, and the one with the largest flux value is
                /// kept. All others are removed from the list.
                if (this->itsMatchingPixList.size() < 2) return;

                std::vector<std::pair<Point, Point> >::iterator alice, bob;
                alice = this->itsMatchingPixList.begin();

                while (alice < this->itsMatchingPixList.end() - 1) {
                    bool bobGone = false;
                    bool aliceGone = false;
                    bob = alice + 1;

                    while (bob < this->itsMatchingPixList.end() && !aliceGone) {
                        if (alice->second.ID() == bob->second.ID()) { // alice & bob have the same reference source
                            float df_alice, df_bob;

                            // if (this->itsFluxMethod == "integrated") {
                            //     df_alice = alice->first.stuff().flux() - alice->second.flux();
                            //     df_bob   = bob->first.stuff().flux() - bob->second.flux();
                            // } else {
                                df_alice = alice->first.flux() - alice->second.flux();
                                df_bob   = bob->first.flux() - bob->second.flux();
                            // }

                            if (fabs(df_alice) < fabs(df_bob)) {
                                // delete bob
                                this->itsMatchingPixList.erase(bob);
                                bobGone = true;
                            } else {
                                // delete alice
                                this->itsMatchingPixList.erase(alice);
                                aliceGone = true;
                            }
                        }

                        if (!bobGone) bob++;
                        else bobGone = false;
                    }

                    if (!aliceGone) alice++;
                }
            }



            //**************************************************************//

            void Matcher::outputMatches()
            {
                /// @details The list of matching points is written to the
                /// designated output file. The format is: type of match --
                /// source ID -- source X -- source Y -- source Flux -- ref ID
                /// -- ref X -- ref Y -- ref Flux. The "type of match" is
                /// either 1 for points matched with the Groth algorithm, or 2
                /// for those subsequently matched.
                std::ofstream fout(this->itsOutputBestFile.c_str());
                std::vector<std::pair<Point, Point> >::iterator match;
                int prec = 3;

                for (match = this->itsMatchingPixList.begin(); match < this->itsMatchingPixList.end(); match++) {
                    // if (this->itsFluxMethod == "integrated") { // need to swap around since we have initially stored peak flux in object.
                    //     float tmpflux;
                    //     tmpflux = match->first.stuff().flux();
                    //     match->first.stuff().setFlux(match->first.flux());
                    //     match->first.setFlux(tmpflux);
                    // }

                    int newprec = int(ceil(log10(1. / match->first.flux()))) + 1;
                    prec = std::max(prec, newprec);
                    newprec = int(ceil(log10(1. / match->first.flux()))) + 1;
                    prec = std::max(prec, newprec);
                }

                fout.setf(std::ios::fixed);
                int ct = 0;
                char matchType;

                for (match = this->itsMatchingPixList.begin(); match < this->itsMatchingPixList.end(); match++) {
                    if (ct++ < this->itsNumMatch1) matchType = '1';
                    else matchType = '2';

		    fout << matchType << "\t" << match->first.ID() << " " << match->second.ID() << " " << 
		      std::setw(8)  << std::setprecision(6) << match->first.sep(match->second) << "\n";

                    // fout << matchType << "\t"
                    //     << "[" << match->first.ID() << "]\t"
                    //     << std::setw(10) << std::setprecision(3) << match->first.x()  << " "
                    //     << std::setw(10) << std::setprecision(3) << match->first.y()  << " "
                    //     << std::setw(10) << std::setprecision(8) << match->first.flux() << " "
                    //     << std::setw(10) << std::setprecision(3) << match->first.majorAxis() << " "
                    //     << std::setw(10) << std::setprecision(3) << match->first.minorAxis() << " "
                    //     << std::setw(10) << std::setprecision(3) << match->first.PA()  << " "
		    // 	 << std::setw(10) << std::setprecision(3) << match->first.alpha() << " "
		    // 	 << std::setw(10) << std::setprecision(3) << match->first.beta() << " "
                    //     << std::setw(10) << match->first.stuff() << "\t"
                    //     << "[" << match->second.ID() << "]\t"
                    //     << std::setw(10) << std::setprecision(3) << match->second.x()  << " "
                    //     << std::setw(10) << std::setprecision(3) << match->second.y()  << " "
                    //     << std::setw(10) << std::setprecision(8) << match->second.flux() << " "
                    //     << std::setw(10) << std::setprecision(3) << match->second.majorAxis() << " "
                    //     << std::setw(10) << std::setprecision(3) << match->second.minorAxis() << " "
                    //     << std::setw(10) << std::setprecision(3) << match->second.PA() << "\t"
		    // 	 << std::setw(10) << std::setprecision(3) << match->second.alpha() << " "
		    // 	 << std::setw(10) << std::setprecision(3) << match->second.beta() << " "
                    //     << std::setw(8)  << std::setprecision(3) << match->first.sep(match->second) << " "
                    //     << std::setw(8)  << std::setprecision(8) << match->first.flux() - match->second.flux() << " "
                    //     << std::setw(8)  << std::setprecision(6) << (match->first.flux() - match->second.flux()) / match->second.flux() << "\n";
                }

                fout.close();
            }

            //**************************************************************//

            void Matcher::outputMisses()
            {
                /// @details The points in the source and reference lists that
                /// were not matched are written to the designated output
                /// file. The format is: type of point -- ID -- X -- Y --
                /// Flux. The "type of point" is either R for reference point
                /// or S for source point.
                std::ofstream fout(this->itsOutputMissFile.c_str());
                fout.setf(std::ios::fixed);
                std::vector<Point>::iterator pt;
                std::vector<std::pair<Point, Point> >::iterator match;
		//                Stuff nullstuff(0., 0., 0., 0, 0, 0, 0, 0.);

                for (pt = this->itsRefCatalogue.fullPointList().begin(); pt < this->itsRefCatalogue.fullPointList().end(); pt++) {
                    bool isMatch = false;
                    match = this->itsMatchingPixList.begin();

                    for (; match < this->itsMatchingPixList.end() && !isMatch; match++) {
                        isMatch = (pt->ID() == match->second.ID());
                    }

                    if (!isMatch) {
                        fout << "R\t" << pt->ID() << "\t"
                            << std::setw(10) << std::setprecision(3) << pt->x()  << " "
                            << std::setw(10) << std::setprecision(3) << pt->y() << " "
                            << std::setw(10) << std::setprecision(8) << pt->flux()  << " "
                            // << std::setw(10) << std::setprecision(3) << pt->majorAxis() << " "
                            // << std::setw(10) << std::setprecision(3) << pt->minorAxis() << " "
                            // << std::setw(10) << std::setprecision(3) << pt->PA()  << " "
                            // << std::setw(10) << nullstuff 
			     << "\n";
                    }
                }

                for (pt = this->itsSrcCatalogue.fullPointList().begin(); pt < this->itsSrcCatalogue.fullPointList().end(); pt++) {
                    bool isMatch = false;
                    match = this->itsMatchingPixList.begin();

                    for (; match < this->itsMatchingPixList.end() && !isMatch; match++) {
                        isMatch = (pt->ID() == match->first.ID());
                    }

                    if (!isMatch) {
                        fout << "S\t" << pt->ID() << "\t"
                            << std::setw(10) << std::setprecision(3) << pt->x()  << " "
                            << std::setw(10) << std::setprecision(3) << pt->y()  << " "
                            << std::setw(10) << std::setprecision(8) << pt->flux() << " "
                            // << std::setw(10) << std::setprecision(3) << pt->majorAxis() << " "
                            // << std::setw(10) << std::setprecision(3) << pt->minorAxis() << " "
                            // << std::setw(10) << std::setprecision(3) << pt->PA()  << " "
                            // << std::setw(10) << pt->stuff() 
			     << "\n";
                    }
                }

            }


	  void Matcher::outputSummary()
	  {
	    std::ofstream fout;
	    //                Stuff nullstuff(0., 0., 0., 0, 0, 0, 0, 0.);

	    std::vector<Point>::iterator pt;
	    std::vector<std::pair<Point, Point> >::iterator mpair;
	    Point match;

	    fout.open("match-summary-sources.txt");
	    for(pt=this->itsSrcCatalogue.fullPointList().begin(); pt<this->itsSrcCatalogue.fullPointList().end(); pt++){
	      bool isMatch=false;
	      for (mpair=this->itsMatchingPixList.begin(); mpair < this->itsMatchingPixList.end() && !isMatch; mpair++) {
		isMatch = (pt->ID() == mpair->first.ID());
		if(isMatch) match = mpair->second;
	      }
	      std::string matchID = isMatch ? match.ID() : "---";
	      fout << pt->ID() << " " << matchID << "\t"
                            << std::setw(10) << std::setprecision(7) << pt->x()  << " "
                            << std::setw(10) << std::setprecision(7) << pt->y()  << " "
                            << std::setw(10) << std::setprecision(8) << pt->flux() << " "
                            // << std::setw(10) << std::setprecision(3) << pt->majorAxis() << " "
                            // << std::setw(10) << std::setprecision(3) << pt->minorAxis() << " "
                            // << std::setw(10) << std::setprecision(3) << pt->PA()  << " "
                            // << std::setw(10) << pt->stuff() 
		   << "\n";
	    }
	    fout.close();

	    fout.open("match-summary-reference.txt");
	    for(pt=this->itsRefCatalogue.fullPointList().begin(); pt<this->itsRefCatalogue.fullPointList().end(); pt++){
	      bool isMatch=false;
	      for (mpair=this->itsMatchingPixList.begin(); mpair < this->itsMatchingPixList.end() && !isMatch; mpair++) {
		isMatch = (pt->ID() == mpair->second.ID());
		if(isMatch) match = mpair->first;
	      }
	      std::string matchID = isMatch ? match.ID() : "---";
	      fout << pt->ID() << " " << matchID << "\t"
		   << std::setw(10) << std::setprecision(7) << pt->x()  << " "
		   << std::setw(10) << std::setprecision(7) << pt->y() << " "
		   << std::setw(10) << std::setprecision(8) << pt->flux()  << " "
                            // << std::setw(10) << std::setprecision(3) << pt->majorAxis() << " "
                            // << std::setw(10) << std::setprecision(3) << pt->minorAxis() << " "
                            // << std::setw(10) << std::setprecision(3) << pt->PA()  << " "
                            // << std::setw(10) << nullstuff
		   << "\n";
	    }
	    fout.close();

	  }



        }

    }

}

