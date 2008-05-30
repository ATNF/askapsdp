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
      const double defaultEpsilon=1.0;

      /// @brief Maximum size for list of points
      const unsigned int maxSizePointList=30;

      /// @brief Class to handle matching of patterns of sources
      /// @details This class uses Triangle and Point classes to match
      /// lists of points. It handles the file input and output, as well as the
      ///actual matching.
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
	/// @brief Constructor, using an input parameter set
	Matcher(const LOFAR::ACC::APS::ParameterSet& parset);

	/// @brief Define the triangle lists from the (existing) point lists, and find matching triangles.
	void setTriangleLists();

	/// @brief Find the points in each list that match.
	void findMatches();

	/// @brief Find the linear offsets between the two lists of points.
	void findOffsets();

	/// @brief Using the known offsets, find matches that were missed by the pattern matching.
	void addNewMatches();

	/// @brief Output lists of matching and isolated points.
	void outputLists(){ outputMatches(); outputMisses(); };
	/// @brief Output the list of matching points
	void outputMatches();
	/// @brief Output the list of points that were not matched.
	void outputMisses();

      protected:
	/// @brief The file with the list of points to be matched
	std::string itsSrcFile;
	/// @brief The file with the list of reference points
	std::string itsRefFile;
	/// @brief The fiducial right ascension for the source list
	std::string itsSrcPosRA;
	/// @brief The fiducial declianation for the source list
	std::string itsSrcPosDec; 
	/// @brief The fiducial right ascencsion for the reference list
	std::string itsRefPosRA;
	/// @brief The fiducial declianation for the reference list
	std::string itsRefPosDec;

	/// @brief The list of source points (those to be matched)
	std::vector<Point> itsSrcPixList;
	/// @brief The list of reference points
	std::vector<Point> itsRefPixList;
	/// @brief The list of triangles from the source list
	std::vector<Triangle> itsSrcTriList;
	/// @brief The list of triangles from the reference list
	std::vector<Triangle> itsRefTriList;

	/// @brief The list of matching triangles
	std::vector<std::pair<Triangle,Triangle> > itsMatchingTriList;
	/// @brief The list of matching points
	std::vector<std::pair<Point,Point> > itsMatchingPixList;

	/// @brief The epsilon error parameter for matching
	double itsEpsilon;
	/// @brief The mean offset in the x-direction between the lists
	double itsMeanDx;
	/// @brief The mean offset in the y-direction between the lists
	double itsMeanDy;
	/// @brief The rms offset in the x-direction between the lists
	double itsRmsDx;
	/// @brief The rms offset in the y-direction between the lists
	double itsRmsDy;

	/// @brief The number of matches just from the pattern matching
	int itsNumMatch1;
	/// @brief The number of matches after the subsequent matching step
	int itsNumMatch2;

	/// @brief The sense of the matches
	bool itsSenseMatch; // true for same sense, false for opposite sense

	/// @brief The file to which the matched points are written
	std::string itsOutputBestFile;
	/// @brief The file to which the non-matched points are written
	std::string itsOutputMissFile;

      };

    }

  }

}

#endif
