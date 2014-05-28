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
#ifndef ASKAP_ANALYSIS_CATMATCHER_H_
#define ASKAP_ANALYSIS_CATMATCHER_H_

#include <patternmatching/Triangle.h>
#include <patternmatching/Point.h>
#include <patternmatching/PointCatalogue.h>

#include <Common/ParameterSet.h>
#include <casa/Quanta.h>

#include <vector>
#include <utility>
#include <string>
#include <math.h>

namespace askap {

  namespace analysis {

    namespace matching { 

      class CatalogueMatcher
      {
      public:
	CatalogueMatcher();
	CatalogueMatcher(const LOFAR::ParameterSet& parset);
	CatalogueMatcher(const CatalogueMatcher& other);
	CatalogueMatcher& operator= (const CatalogueMatcher& other);
	virtual ~CatalogueMatcher(){};
	
	/// @brief Change epsilon to pixel units
	void convertEpsilon();

	/// @brief Read in the lists of source and reference objects.
	bool read();

	/// @brief Define the triangle lists from the (existing) point lists, and find matching triangles.
	void setTriangleLists();

	/// @brief Find the points in each list that match.
	void findMatches();

	/// @brief Find matching points assuming no significant net spatial offset
	void zeroOffsetMatch();

	/// @brief Find the linear offsets between the two lists of points.
	void findOffsets();

	/// @brief Using the known offsets, find matches that were missed by the pattern matching.
	void addNewMatches();

	/// @brief Remove multiple references to objects in the match list.
	void rejectMultipleMatches();

	/// @brief Output lists of matching and isolated points.
	void outputLists() { outputMatches(); outputMisses(); };
	/// @brief Output the list of matching points
	void outputMatches();
	/// @brief Output the list of points that were not matched.
	void outputMisses();
	/// @brief Output the list of sources with any matches from the other list
	void outputSummary();

	unsigned int srcListSize(){return this->itsSrcCatalogue.pointList().size();}
	unsigned int refListSize(){return this->itsRefCatalogue.pointList().size();}

      protected:
	/// @brief Hold lists for the source catalogue
	PointCatalogue itsSrcCatalogue;
	/// @brief Hold lists for the reference catalogue
	PointCatalogue itsRefCatalogue;
	/// @brief Image to get world-pixel conversion from
	std::string itsReferenceImage;
	/// @brief The list of matching triangles
	std::vector<std::pair<Triangle, Triangle> > itsMatchingTriList;
	/// @brief The list of matching points
	std::vector<std::pair<Point, Point> > itsMatchingPixList;
	/// @brief The epsilon error parameter for matching
	double itsEpsilon;
	casa::Unit itsEpsilonUnits;
	casa::Unit itsPositionUnits;
	/// @brief The number of matches after the initial attempt
	unsigned int itsNumInitialMatches;
	/// @brief Do the two catalogues have the same sense?
	bool itsSenseMatch;

	/// @brief Mean x- and y-offsets for the matches
	/// @{
	double itsMeanDx;
	double itsMeanDy;
	/// @}

	/// @brief The output files for the matches and misses
	/// @{
	std::string itsMatchFile;
	std::string itsMissFile;
	std::string itsSourceSummaryFile;
	std::string itsReferenceSummaryFile;
	/// @}

      };


    }
  }
}
#endif
