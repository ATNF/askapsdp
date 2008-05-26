/// @file
///
/// Provides base class for handling the matching of lists of points
///
/// (c) 2007 ASKAP, All Rights Reserved.
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

      Matcher::Matcher(int argc, const char** argv, const LOFAR::ACC::APS::ParameterSet& parset)
      {

	this->itsSrcFile = parset.getString("srcFile");
	this->itsRefFile = parset.getString("refFile");
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
	
	std::ifstream fsrc(this->itsSrcFile.c_str());
	std::ifstream fref(this->itsRefFile.c_str());

	this->itsSrcPixList = getSrcPixList(fsrc, this->itsSrcPosRA, this->itsSrcPosDec);
	ASKAPLOG_INFO_STR(logger, "Size of source pixel list = " << this->itsSrcPixList.size());

	this->itsRefPixList = getPixList(fref, this->itsRefPosRA, this->itsRefPosDec);
	ASKAPLOG_INFO_STR(logger, "Size of reference pixel list = " << this->itsRefPixList.size());

     }

      void Matcher::setTriangleLists()
      {

	this->itsSrcTriList = getTriList(this->itsSrcPixList);
	this->itsRefTriList = getTriList(this->itsRefPixList);

	this->itsMatchingTriList = matchLists(this->itsSrcTriList, this->itsRefTriList, this->itsEpsilon);

	trimTriList(this->itsMatchingTriList);
    
	ASKAPLOG_INFO_STR(logger, "Found " << this->itsMatchingTriList.size() << " matches\n");

      }

      void Matcher::findMatches()
      {

	this->itsMatchingPixList = vote(this->itsMatchingTriList);
	
	this->itsSenseMatch = (this->itsMatchingTriList[0].first.isClockwise() == 
			       this->itsMatchingTriList[0].second.isClockwise()    );

      }


      void Matcher::findOffsets()
      {

	int size = this->itsMatchingPixList.size();
	float *dx = new float[size];
	float *dy = new float[size];
	for(int i=0;i<size;i++){
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
		    << std::setw(10) << this->itsMatchingPixList[i].first.y()  << "\t"
		    << "[" << this->itsMatchingPixList[i].second.ID() << "]\t"
		    << std::setw(10) << this->itsMatchingPixList[i].second.x()  << " "
		    << std::setw(10) << this->itsMatchingPixList[i].second.y()  << "\t"
		    << "dx = " << std::setw(7) << dx[i] <<"\t" << "dy = " << std::setw(7) << dy[i] <<"\n";

	}


	for(int i=0;i<size;i++){
	  this->itsMeanDx += dx[i];
	  this->itsMeanDy += dy[i];
	}
	this->itsMeanDx /= double(size);
	this->itsMeanDy /= double(size);
	for(int i=0;i<size;i++){
	  this->itsRmsDx += (dx[i]-this->itsMeanDx)*(dx[i]-this->itsMeanDx);
	  this->itsRmsDy += (dy[i]-this->itsMeanDy)*(dy[i]-this->itsMeanDy);
	}
	this->itsRmsDx = sqrt( this->itsRmsDx / (double(size-1)) );
	this->itsRmsDy = sqrt( this->itsRmsDy / (double(size-1)) );
	ASKAPLOG_INFO_STR(logger, "Offsets between the two is dx=" << this->itsMeanDx << "+-" << this->itsRmsDx 
			  <<", dy=" << this->itsMeanDy << "+-" << this->itsRmsDy << "\n");
 

      }


      void Matcher::outputMatches()
      {

	std::ofstream fout(this->itsOutputBestFile.c_str());
	fout.precision(3);
	fout.setf(std::ios::fixed);
	std::vector<std::pair<Point,Point> >::iterator match;
	for(match=this->itsMatchingPixList.begin(); match<this->itsMatchingPixList.end(); match++){

	  fout << "[" << match->first.ID() << "]\t"
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

      void Matcher::outputMisses()
      {

	std::ofstream fout(this->itsOutputMissFile.c_str());
	fout.precision(3);
	fout.setf(std::ios::fixed);
	std::vector<Point>::iterator pt;
	std::vector<std::pair<Point,Point> >::iterator match;
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


      }



    }

  }

}

