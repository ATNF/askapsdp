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
#ifndef ASKAP_ANALYSIS_POINTCAT_H_
#define ASKAP_ANALYSIS_POINTCAT_H_

#include <patternmatching/Point.h>
#include <patternmatching/Triangle.h>
#include <modelcomponents/ModelFactory.h>

#include <Common/ParameterSet.h>

#include <vector>

namespace askap {

  namespace analysis {

    namespace matching {

      const double defaultRatioLimit = 10.;

      class PointCatalogue
      {
      public:
	PointCatalogue();
	PointCatalogue(LOFAR::ParameterSet &parset);
	PointCatalogue(const PointCatalogue& other);
	PointCatalogue& operator= (const PointCatalogue& other);
	virtual ~PointCatalogue(){};

	bool read();
	void makeTriangleList();
	bool crudeMatch(std::vector<Point> &other, double maxSep);

	std::string filename(){return itsFilename;};
	std::vector<Point> &fullPointList(){return itsFullPointList;};
	std::vector<Point> &pointList(){return itsWorkingPointList;};
	std::vector<Triangle> &triangleList(){return itsTriangleList;};
	double ratioLimit(){return itsRatioLimit;};
	double raRef(){return itsRAref;};
	double decRef(){return itsDECref;};
	double radius(){return itsRadius;};

      protected:
	std::vector<Point> itsFullPointList;
	std::vector<Point> itsWorkingPointList;
	std::vector<Triangle> itsTriangleList;
	std::string itsFilename;
	analysisutilities::ModelFactory itsFactory;
	size_t itsTrimSize; // only use the first itsTrimSize points to make the triangle list
	double itsRatioLimit;
	bool   itsFlagOffsetPositions;
	double itsRAref;
	double itsDECref;
	double itsRadius;
	std::string itsReferenceImage;
      };

    }
  }
}

#endif
