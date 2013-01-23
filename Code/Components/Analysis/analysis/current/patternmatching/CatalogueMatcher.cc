/// @file
///
/// XXX Notes on program XXX
///
/// @copyright (c) 2011 CSIRO
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
/// @author XXX XXX <XXX.XXX@csiro.au>
///
#include <askap_analysis.h>
#include <patternmatching/CatalogueMatcher.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <patternmatching/Triangle.h>
#include <patternmatching/Point.h>
#include <patternmatching/PointCatalogue.h>
#include <analysisutilities/MatchingUtilities.h>

#include <Common/ParameterSet.h>

#include <vector>

ASKAP_LOGGER(logger, ".cataloguematching");

namespace askap {

  namespace analysis { 

    namespace matching {

      CatalogueMatcher::CatalogueMatcher()
      {
      }
      
      CatalogueMatcher::CatalogueMatcher(const LOFAR::ParameterSet& parset)
      {
	this->itsEpsilon = parset.getDouble("epsilon", defaultEpsilon);
	LOFAR::ParameterSet subset=parset.makeSubset("source.");
	this->itsSrcCatalogue = PointCatalogue(subset);
	subset=parset.makeSubset("reference.");
	this->itsRefCatalogue = PointCatalogue(subset);
	this->itsMatchFile = parset.getString("matchFile", "matches.txt");
	this->itsMissFile = parset.getString("missFile", "misses.txt");
	this->itsMeanDx = this->itsMeanDy = 0.;
      }

      CatalogueMatcher::CatalogueMatcher(const CatalogueMatcher& other)
      {
	this->operator=(other);
      }

      CatalogueMatcher& CatalogueMatcher::operator= (const CatalogueMatcher& other)
      {
	if(this==&other) return *this;
	this->itsSrcCatalogue = other.itsSrcCatalogue;
	this->itsRefCatalogue = other.itsRefCatalogue;
	this->itsMatchingTriList = other.itsMatchingTriList;
	this->itsMatchingPixList = other.itsMatchingPixList;
	this->itsEpsilon = other.itsEpsilon;
	this->itsNumInitialMatches = other.itsNumInitialMatches;
	this->itsSenseMatch = other.itsSenseMatch;
	this->itsMeanDx = other.itsMeanDx;
	this->itsMeanDy = other.itsMeanDy;
	this->itsMatchFile = other.itsMatchFile;
	this->itsMissFile = other.itsMissFile;
	return *this;
      }


      bool CatalogueMatcher::read()
      {
	/// @details This function reads the source and reference
	/// pixel lists from the files provided. Checks are made
	/// for the validity of the files.

	bool filesOK = true;

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

	if(!this->itsRefCatalogue.crudeMatch(this->itsSrcCatalogue.fullPointList(),this->itsEpsilon))
	  ASKAPLOG_WARN_STR(logger, "Crude matching failed! Using full reference point list");

	return filesOK;
      }


      //**************************************************************//

      void CatalogueMatcher::findMatches()
      {
	/// @details Matching points are found via the Groth voting
	/// function vote(). The number of matches and their sense are
	/// recorded.
	this->itsMatchingTriList = matchLists(this->itsSrcCatalogue.triangleList(), this->itsRefCatalogue.triangleList(), this->itsEpsilon);
	trimTriList(this->itsMatchingTriList);
	ASKAPLOG_INFO_STR(logger, "Found " << this->itsMatchingTriList.size() << " matches");
	this->itsNumInitialMatches = 0;
	      
	if (this->itsMatchingTriList.size() > 0) {
	  this->itsMatchingPixList = vote(this->itsMatchingTriList);
	  this->itsNumInitialMatches = this->itsMatchingPixList.size();
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

      void CatalogueMatcher::findOffsets()
      {
	/// @details The mean and rms offsets in the x- and
	/// y-directions are measured for the matching points.
	// std::vector<double> dx(this->itsNumMatch1,0.),dy(this->itsNumMatch1,0.);
	std::vector<double> dx(this->itsMatchingPixList.size(),0.),dy(this->itsMatchingPixList.size(),0.);

	double sensescale=this->itsSenseMatch?-1.:1.;
	for (size_t i = 0; i < this->itsMatchingPixList.size(); i++) {
	  dx[i] = this->itsMatchingPixList[i].first.x() - this->itsMatchingPixList[i].second.x();
	  dy[i] = this->itsMatchingPixList[i].first.y() + sensescale*this->itsMatchingPixList[i].second.y();
	}

	this->itsMeanDx = this->itsMeanDy = 0.;
	for (size_t i = 0; i < this->itsMatchingPixList.size(); i++) {
	  this->itsMeanDx += dx[i];
	  this->itsMeanDy += dy[i];
	}
	this->itsMeanDx /= double(this->itsMatchingPixList.size());
	this->itsMeanDy /= double(this->itsMatchingPixList.size());

	// Don't need these elsewhere, so make local only
	double rmsDx=0.,rmsDy=0.;
	for (size_t i = 0; i < this->itsMatchingPixList.size(); i++) {
	  rmsDx += (dx[i] - this->itsMeanDx) * (dx[i] - this->itsMeanDx);
	  rmsDy += (dy[i] - this->itsMeanDy) * (dy[i] - this->itsMeanDy);
	}
	rmsDx = sqrt(rmsDx / (double(this->itsMatchingPixList.size() - 1)));
	rmsDy = sqrt(rmsDy / (double(this->itsMatchingPixList.size() - 1)));

	std::stringstream ss;
	ss << "Offsets between the two are dx = " << this->itsMeanDx << " +- " << rmsDx
	   << " dy = " << this->itsMeanDy << " +- " << rmsDy;
	ASKAPLOG_INFO_STR(logger, ss.str());
      }
 
      //**************************************************************//

      void CatalogueMatcher::addNewMatches()
      {
	/// @details The source point list is scanned for points that
	/// were not initially matched, but have a reference
	/// counterpart within a certain number of epsilon values
	/// (currently set at 3). These points are added to the
	/// itsMatchingPixList, and the new total number of matches is
	/// recorded.

	if (itsNumInitialMatches > 0) {

	  this->rejectMultipleMatches();
	  const float matchRadius = 3.;
	  std::vector<Point>::iterator src, ref;
	  std::vector<std::pair<Point, Point> >::iterator match;

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

	  ASKAPLOG_INFO_STR(logger, "Total number of matches = " << this->itsMatchingPixList.size());
	}
      }

      //**************************************************************//

      void CatalogueMatcher::rejectMultipleMatches()
      {
	/// @details Objects that appear twice in the match list are
	/// examined, and the one with the largest flux value is
	/// kept. All others are removed from the list.
	if (this->itsMatchingPixList.size() < 2) return;

	std::vector<std::pair<Point, Point> >::iterator alice, bob;
	alice = this->itsMatchingPixList.begin();

	while (alice < this->itsMatchingPixList.end() - 1) {
	  bool aliceGone = false;
	  bob = alice + 1;

	  while (bob < this->itsMatchingPixList.end() && !aliceGone) {
	    bool bobGone = false;
	    if (alice->second.ID() == bob->second.ID()) { // alice & bob have the same reference source
	      double df_alice = alice->first.flux() - alice->second.flux();
	      double df_bob   = bob->first.flux() - bob->second.flux();

	      if (fabs(df_alice) < fabs(df_bob)) {
		this->itsMatchingPixList.erase(bob);
		bobGone = true;
	      } else {
		this->itsMatchingPixList.erase(alice);
		aliceGone = true;
	      }
	    }
	    if (!bobGone) bob++;
	  }

	  if (!aliceGone) alice++;
	}
      }

      //**************************************************************//

      void CatalogueMatcher::outputMatches()
      {
	/// @details The list of matching points is written to the
	/// designated output file. The format is: type of match --
	/// source ID -- source X -- source Y -- source Flux -- ref ID
	/// -- ref X -- ref Y -- ref Flux. The "type of match" is
	/// either 1 for points matched with the Groth algorithm, or 2
	/// for those subsequently matched.
	std::ofstream fout(this->itsMatchFile.c_str());
	std::vector<std::pair<Point, Point> >::iterator match;
	int prec = 3;
	size_t width = 0;
	for (match = this->itsMatchingPixList.begin(); match < this->itsMatchingPixList.end(); match++) {
	  prec = std::max(prec, int(ceil(log10(1. / match->first.flux()))) + 1);
	  width = std::max(width, match->first.ID().size());
	  width = std::max(width, match->second.ID().size());
	}

	fout.setf(std::ios::fixed);
	unsigned int ct = 0;
	char matchType;

	for (match = this->itsMatchingPixList.begin(); match < this->itsMatchingPixList.end(); match++) {
	  if (ct++ < this->itsNumInitialMatches) matchType = '1';
	  else matchType = '2';

	  fout << std::setw(3) << matchType << " " 
	       << std::setw(width) << match->first.ID() << " " 
	       << std::setw(width) << match->second.ID() << " " 
	       << std::setw(8)  << std::setprecision(6) << match->first.sep(match->second) << "\n";

	}

	fout.close();
      }

      //**************************************************************//

      void CatalogueMatcher::outputMisses()
      {
	/// @details The points in the source and reference lists that
	/// were not matched are written to the designated output
	/// file. The format is: type of point -- ID -- X -- Y --
	/// Flux. The "type of point" is either R for reference point
	/// or S for source point.
	std::ofstream fout(this->itsMissFile.c_str());
	fout.setf(std::ios::fixed);
	std::vector<Point>::iterator pt;
	std::vector<std::pair<Point, Point> >::iterator match;

	size_t width = 0;
	for (pt = this->itsRefCatalogue.fullPointList().begin(); pt < this->itsRefCatalogue.fullPointList().end(); pt++)
	  width = std::max(width, pt->ID().size());

	for (pt = this->itsRefCatalogue.fullPointList().begin(); pt < this->itsRefCatalogue.fullPointList().end(); pt++) {
	  bool isMatch = false;

	  for (match = this->itsMatchingPixList.begin(); match < this->itsMatchingPixList.end() && !isMatch; match++)
	    isMatch = (pt->ID() == match->second.ID());

	  if (!isMatch) {
	    fout << "R " 
		 << std::setw(width) << pt->ID() << " "
		 << std::setw(10) << std::setprecision(3) << pt->x()  << " "
		 << std::setw(10) << std::setprecision(3) << pt->y() << " "
		 << std::setw(10) << std::setprecision(8) << pt->flux()  << "\n";
	  }
	}

	width = 0;
	for (pt = this->itsSrcCatalogue.fullPointList().begin(); pt < this->itsSrcCatalogue.fullPointList().end(); pt++)
	  width = std::max(width, pt->ID().size());

	for (pt = this->itsSrcCatalogue.fullPointList().begin(); pt < this->itsSrcCatalogue.fullPointList().end(); pt++) {
	  bool isMatch = false;

	  for (match = this->itsMatchingPixList.begin(); match < this->itsMatchingPixList.end() && !isMatch; match++) 
	    isMatch = (pt->ID() == match->first.ID());

	  if (!isMatch) {
	    fout << "S " 
		 << std::setw(width) << pt->ID() << " "
		 << std::setw(10) << std::setprecision(3) << pt->x()  << " "
		 << std::setw(10) << std::setprecision(3) << pt->y()  << " "
		 << std::setw(10) << std::setprecision(8) << pt->flux() << "\n";
	  }
	}

      }

      //**************************************************************//

      void CatalogueMatcher::outputSummary()
      {
	std::ofstream fout;
	std::vector<Point>::iterator pt;
	std::vector<std::pair<Point, Point> >::iterator mpair;
	std::vector<std::pair<Point, Point> >::iterator match;
	std::string matchID;

	size_t width = 0;
	for (match = this->itsMatchingPixList.begin(); match < this->itsMatchingPixList.end(); match++) {
	  width = std::max(width, match->first.ID().size());
	  width = std::max(width, match->second.ID().size());
	}

	fout.open("match-summary-sources.txt");
	for(pt=this->itsSrcCatalogue.fullPointList().begin(); pt<this->itsSrcCatalogue.fullPointList().end(); pt++){
	  bool isMatch=false;
	  for (mpair=this->itsMatchingPixList.begin(); mpair < this->itsMatchingPixList.end() && !isMatch; mpair++) {
	    isMatch = (pt->ID() == mpair->first.ID());
	    matchID = isMatch ? mpair->second.ID() : "---";
	  }
	  fout << std::setw(width) << pt->ID() << " " 
	       << std::setw(width) << matchID << " "
	       << std::setw(10) << std::setprecision(7) << pt->x()  << " "
	       << std::setw(10) << std::setprecision(7) << pt->y()  << " "
	       << std::setw(10) << std::setprecision(8) << pt->flux() << "\n";
	}
	fout.close();

	fout.open("match-summary-reference.txt");
	for(pt=this->itsRefCatalogue.fullPointList().begin(); pt<this->itsRefCatalogue.fullPointList().end(); pt++){
	  bool isMatch=false;
	  for (mpair=this->itsMatchingPixList.begin(); mpair < this->itsMatchingPixList.end() && !isMatch; mpair++) {
	    isMatch = (pt->ID() == mpair->second.ID());
	    matchID = isMatch ? mpair->first.ID() : "---";
	  }
	  fout << std::setw(width) << pt->ID() << " " 
	       << std::setw(width) << matchID << " "
	       << std::setw(10) << std::setprecision(7) << pt->x()  << " "
	       << std::setw(10) << std::setprecision(7) << pt->y() << " "
	       << std::setw(10) << std::setprecision(8) << pt->flux()  << "\n";
	}
	fout.close();

      }


    }
  }
}
