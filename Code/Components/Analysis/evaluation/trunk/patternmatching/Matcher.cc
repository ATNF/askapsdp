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
#include <patternmatching/Matcher.h>
#include <patternmatching/GrothTriangles.h>
#include <evaluationutilities/EvaluationUtilities.h>

#include <APS/ParameterSet.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <utility>
#include <string>
#include <math.h>

ASKAP_LOGGER(logger, ".matching");

namespace askap
{

  namespace evaluation
  {

    namespace matching
    {

      Matcher::Matcher(const LOFAR::ACC::APS::ParameterSet& parset)
      {
	/// @details The parameter file is examined for the relevant
	/// parameters to define the input and output files, the base
	/// positions for both lists, and the epsilon value. The input
	/// files are read to obtain the source and reference point
	/// lists.

	this->itsSrcFile = parset.getString("srcFile","");
	this->itsRefFile = parset.getString("refFile","");
	this->itsSrcPosRA  = parset.getString("srcPosRA");
	this->itsSrcPosDec = parset.getString("srcPosDec");
	this->itsRefPosRA  = parset.getString("refPosRA");
	this->itsRefPosDec = parset.getString("refPosDec");
	this->itsEpsilon = parset.getDouble("epsilon",defaultEpsilon);
	this->itsMeanDx = 0.;
	this->itsMeanDy = 0.;
	this->itsRmsDx = 0.;
	this->itsRmsDy = 0.;
	this->itsOutputBestFile = parset.getString("matchFile","matches.txt");
	this->itsOutputMissFile = parset.getString("missFile","misses.txt");

	bool filesOK = true;
	if(this->itsSrcFile == "" ){
	  ASKAPLOG_ERROR_STR(logger, "srcFile not defined. Cannot get pixel list!");
	  filesOK = false;
	}
	if(this->itsRefFile == "" ){
	  ASKAPLOG_ERROR_STR(logger, "refFile not defined. Cannot get pixel list!");
	  filesOK = false;
	}

	if(filesOK){
	  std::ifstream fsrc(this->itsSrcFile.c_str());
	  std::ifstream fref(this->itsRefFile.c_str());
	  
	  this->itsSrcPixList = getSrcPixList(fsrc, this->itsSrcPosRA, this->itsSrcPosDec);
	  ASKAPLOG_INFO_STR(logger, "Size of source pixel list = " << this->itsSrcPixList.size());
	  
	  this->itsRefPixList = getPixList(fref, this->itsRefPosRA, this->itsRefPosDec);
	  ASKAPLOG_INFO_STR(logger, "Size of reference pixel list = " << this->itsRefPixList.size());
	}
     }

      //**************************************************************//

      void Matcher::setTriangleLists()
      {

	/// @details The point lists are first shortened to the
	/// appropriate size by trimList(). The shortened lists are then
	/// converted into triangle lists, which are matched and
	/// trimmed.

	std::vector<Point> srclist = trimList(this->itsSrcPixList);
	ASKAPLOG_INFO_STR(logger, "Trimmed src list to " << srclist.size() << " points");
	std::vector<Point> reflist = trimList(this->itsRefPixList);
	ASKAPLOG_INFO_STR(logger, "Trimmed ref list to " << reflist.size() << " points");

	this->itsSrcTriList = getTriList(srclist);
	this->itsRefTriList = getTriList(reflist);


	this->itsMatchingTriList = matchLists(this->itsSrcTriList, this->itsRefTriList, this->itsEpsilon);

	trimTriList(this->itsMatchingTriList);
    
	ASKAPLOG_INFO_STR(logger, "Found " << this->itsMatchingTriList.size() << " matches\n");

      }

      //**************************************************************//

      void Matcher::findMatches()
      {

	/// @details Matching points are found via the Groth voting
	/// function vote(). The number of matches and their sense are
	/// recorded.
	
	this->itsMatchingPixList = vote(this->itsMatchingTriList);
	this->itsNumMatch1 = this->itsMatchingPixList.size();
	
	this->itsSenseMatch = (this->itsMatchingTriList[0].first.isClockwise() == 
			       this->itsMatchingTriList[0].second.isClockwise()    );

      }


      //**************************************************************//

      void Matcher::findOffsets()
      {

	/// @details The mean and rms offsets in the x- and
	/// y-directions are measured for the matching points.

	float *dx = new float[this->itsNumMatch1];
	float *dy = new float[this->itsNumMatch1];
	for(int i=0;i<this->itsNumMatch1;i++){
	  if(this->itsSenseMatch){
	    dx[i] = this->itsMatchingPixList[i].first.x()-this->itsMatchingPixList[i].second.x();
	    dy[i] = this->itsMatchingPixList[i].first.y()-this->itsMatchingPixList[i].second.y();
	  }
	  else{
	    dx[i] = this->itsMatchingPixList[i].first.x()-this->itsMatchingPixList[i].second.x();
	    dy[i] = this->itsMatchingPixList[i].first.y()+this->itsMatchingPixList[i].second.y();
	  }

	  std::cout.precision(3);
	  std::cout.setf(std::ios::fixed);
	  std::cout << "[" << this->itsMatchingPixList[i].first.ID() << "]\t"
		    << std::setw(10) << this->itsMatchingPixList[i].first.x()  << " "
		    << std::setw(10) << this->itsMatchingPixList[i].first.y()  << " "
		    << std::setw(10) << this->itsMatchingPixList[i].first.flux() << "\t"
		    << "[" << this->itsMatchingPixList[i].second.ID() << "]\t"
		    << std::setw(10) << this->itsMatchingPixList[i].second.x()  << " "
		    << std::setw(10) << this->itsMatchingPixList[i].second.y()  << " "
		    << std::setw(10) << this->itsMatchingPixList[i].second.flux() << "\t"
		    << "dx = " << std::setw(7) << dx[i] <<"\t" << "dy = " << std::setw(7) << dy[i] <<"\n";

	}


	for(int i=0;i<this->itsNumMatch1;i++){
	  this->itsMeanDx += dx[i];
	  this->itsMeanDy += dy[i];
	}
	this->itsMeanDx /= double(this->itsNumMatch1);
	this->itsMeanDy /= double(this->itsNumMatch1);
	for(int i=0;i<this->itsNumMatch1;i++){
	  this->itsRmsDx += (dx[i]-this->itsMeanDx)*(dx[i]-this->itsMeanDx);
	  this->itsRmsDy += (dy[i]-this->itsMeanDy)*(dy[i]-this->itsMeanDy);
	}
	this->itsRmsDx = sqrt( this->itsRmsDx / (double(this->itsNumMatch1-1)) );
	this->itsRmsDy = sqrt( this->itsRmsDy / (double(this->itsNumMatch1-1)) );
	ASKAPLOG_INFO_STR(logger, "Offsets between the two is dx=" << this->itsMeanDx << "+-" << this->itsRmsDx 
			  <<", dy=" << this->itsMeanDy << "+-" << this->itsRmsDy << "\n");
 

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

	const float matchRadius = 3.;
	std::vector<Point>::iterator src,ref;
	std::vector<std::pair<Point,Point> >::iterator match;
	for(src=this->itsSrcPixList.begin(); src<this->itsSrcPixList.end(); src++){
	  bool isMatch=false;
	  match = this->itsMatchingPixList.begin();
	  for(;match<this->itsMatchingPixList.end()&&!isMatch;match++){
	    isMatch = (src->ID() == match->first.ID());
	  }
	  if(!isMatch){
	    float minOffset;
	    int minRef=-1;
	    for(ref=this->itsRefPixList.begin(); ref<this->itsRefPixList.end(); ref++){
	      float offset = hypot(src->x()-ref->x()-this->itsMeanDx,
				   src->y()-ref->y()-this->itsMeanDy);
	      if(offset < matchRadius*this->itsEpsilon){
		if((minRef==-1)||(offset<minOffset)){
		  minOffset = offset;
		  minRef = int(ref-this->itsRefPixList.begin());
		}
	      }
	    }

	    if(minRef>=0){ // there was a match within errors
	      ref = this->itsRefPixList.begin() + minRef;
	      //	      ASKAPLOG_INFO_STR(logger, "New match: " << src->ID() << " <--> " << ref->ID() << " with offset = " << minOffset  << " cf. this->itsEpsilon=" << this->itsEpsilon);
	      std::pair<Point,Point> newMatch(*src, *ref);
	      this->itsMatchingPixList.push_back(newMatch);
	    }

	  }
	}
	
	this->itsNumMatch2 = this->itsMatchingPixList.size();

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
	fout.precision(3);
	fout.setf(std::ios::fixed);
	std::vector<std::pair<Point,Point> >::iterator match;
	int ct=0;
	char matchType;
	for(match=this->itsMatchingPixList.begin(); match<this->itsMatchingPixList.end(); match++){

	  if(ct++<this->itsNumMatch1) matchType = '1';
	  else matchType = '2';
	  fout << matchType << "\t"
	       << "[" << match->first.ID() << "]\t"
	       << std::setw(10) << match->first.x()  << " "
	       << std::setw(10) << match->first.y()  << " "
	       << std::setw(10) << match->first.flux() << "\t"
	       << "[" << match->second.ID() << "]\t"
	       << std::setw(10) << match->second.x()  << " "
	       << std::setw(10) << match->second.y()  << " "
	       << std::setw(10) << match->second.flux() << "\n";

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
	fout.precision(3);
	fout.setf(std::ios::fixed);
	std::vector<Point>::iterator pt;
	std::vector<std::pair<Point,Point> >::iterator match;
	for(pt=this->itsRefPixList.begin(); pt<this->itsRefPixList.end(); pt++){
	  bool isMatch=false;
	  match = this->itsMatchingPixList.begin();
	  for(;match<this->itsMatchingPixList.end()&&!isMatch;match++){
	    isMatch = (pt->ID() == match->second.ID());
	  }
	  if(!isMatch){
	    fout << "R\t[" << pt->ID() << "]\t"
		 << std::setw(10) << pt->x()  << " "
		 << std::setw(10) << pt->y() << " "
		 << std::setw(10) << pt->flux()  << "\n";
	  }
	}
	for(pt=this->itsSrcPixList.begin(); pt<this->itsSrcPixList.end(); pt++){
	  bool isMatch=false;
	  match = this->itsMatchingPixList.begin();
	  for(;match<this->itsMatchingPixList.end()&&!isMatch;match++){
	    isMatch = (pt->ID() == match->first.ID());
	  }
	  if(!isMatch){
	    fout << "S\t[" << pt->ID() << "]\t"
		 << std::setw(10) << pt->x()  << " "
		 << std::setw(10) << pt->y()  << " "
		 << std::setw(10) << pt->flux() << "\n";
	  }
	}


      }



    }

  }

}

