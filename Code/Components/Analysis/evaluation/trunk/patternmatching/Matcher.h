/// @file
///
/// Provides base class for handling the matching of lists of points
///
/// (c) 2007 ASKAP, All Rights Reserved.
/// @author Matthew Whiting <matthew.whiting@csiro.au>
///
#ifndef ASKAP_EVALUATION_MATCHER_H_
#define ASKAP_EVALUATION_MATCHER_H_

#include <askap_evaluation.h>

#include <patternmatching/GrothTriangles.h>

#include <APS/ParameterSet.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <vector>
#include <utility>
#include <string>
#include <math.h>

namespace askap
{

  namespace evaluation
  {

    namespace matching
    {

      /// @brief Default value of the error parameter used in pattern matching
      const double defaultEpsilon=0.01;

      /// @brief Maximum size for list of points
      const unsigned int maxSizePointList=30;

      /// @brief Class to handle matching of patterns of sources
      class Matcher
      {
      public:
	Matcher(){	
	  itsMeanDx=0.;
	  itsMeanDy=0.;
	  itsRmsDx=0.;
	  itsRmsDy=0.;
	};

	virtual ~Matcher(){};
	Matcher(int argc, const char** argv, const LOFAR::ACC::APS::ParameterSet& parset);

	void setTriangleLists();

	void findMatches();

	void findOffsets();

	void outputLists(){ outputMatches(); outputMisses(); };
	void outputMatches();
	void outputMisses();

      protected:
	std::string itsSrcFile;
	std::string itsRefFile;
	std::string itsSrcPosRA;
	std::string itsSrcPosDec; 
	std::string itsRefPosRA;
	std::string itsRefPosDec;

	std::vector<Point> itsSrcPixList;
	std::vector<Point> itsRefPixList;
	std::vector<Triangle> itsSrcTriList;
	std::vector<Triangle> itsRefTriList;

	std::vector<std::pair<Triangle,Triangle> > itsMatchingTriList;
	std::vector<std::pair<Point,Point> > itsMatchingPixList;

	double itsEpsilon;
	double itsMeanDx;
	double itsMeanDy;
	double itsRmsDx;
	double itsRmsDy;

	bool itsSenseMatch; // true for same sense, false for opposite sense

	std::string itsOutputBestFile;
	std::string itsOutputMissFile;

      };

    }

  }

}

#endif
