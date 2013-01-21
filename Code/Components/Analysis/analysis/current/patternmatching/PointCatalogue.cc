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
#include <patternmatching/PointCatalogue.h>
#include <patternmatching/Point.h>
#include <patternmatching/Triangle.h>
#include <modelcomponents/ModelFactory.h>
#include <modelcomponents/Spectrum.h>

#include <Common/ParameterSet.h>

#include <vector>
ASKAP_LOGGER(logger, ".matching.PointCat");

namespace askap {

  namespace analysis {

    namespace matching {

      PointCatalogue::PointCatalogue():
	itsFilename(""),itsTrimSize(0),itsRatioLimit(defaultRatioLimit)
      {
	if(this->itsTrimSize <= 2) ASKAPLOG_WARN_STR(logger, "Since trimsize<=2, the entire point list will be used to generate triangles.");
	this->itsPointList = std::vector<Point>(0); 
	this->itsTriangleList = std::vector<Triangle>(0);
     }

      PointCatalogue::PointCatalogue(LOFAR::ParameterSet &parset)
      {
	this->itsFilename = parset.getString("filename","");
	if(parset.getString("database", "Continuum")=="Selavy"){
	  parset.replace("useDeconvolvedSizes","true");  // set this to true, just so we don't have to worry about the SelavyImage
	}
	this->itsFactory = analysisutilities::ModelFactory(parset);
	this->itsTrimSize = parset.getUint32("trimsize",0);
	if(this->itsTrimSize <= 2) ASKAPLOG_WARN_STR(logger, "Since trimsize<=2, the entire point list will be used to generate triangles.");
	this->itsRatioLimit = parset.getFloat("ratioLimit",defaultRatioLimit);
	this->itsPointList = std::vector<Point>(0); 
	this->itsTriangleList = std::vector<Triangle>(0);
      }

      PointCatalogue::PointCatalogue(const PointCatalogue& other)
      {
	this->operator=(other);
      }
      
      PointCatalogue& PointCatalogue::operator= (const PointCatalogue& other)
      {
	if(this == &other) return *this;
	this->itsFilename = other.itsFilename;
	this->itsFactory = other.itsFactory;
	this->itsTrimSize = other.itsTrimSize;
	this->itsRatioLimit = other.itsRatioLimit;
	this->itsPointList = other.itsPointList;
	this->itsTriangleList = other.itsTriangleList;
	return *this;
      }

      void PointCatalogue::read()
      {
	std::ifstream fin(this->itsFilename.c_str());
	this->itsPointList = std::vector<Point>(0);
	if(!fin.is_open())
	  ASKAPLOG_WARN_STR(logger,"Could not open filename " << this->itsFilename << ".");
	else {
	  std::string line;
	  while (getline(fin, line),
		 !fin.eof()) {
	    if (line[0] != '#') {  // ignore commented lines
	      analysisutilities::Spectrum *spec=this->itsFactory.read(line);
	      this->itsPointList.push_back(spec);
	      delete spec;
	    }
	  }
	}
	this->makeTriangleList();

      }

      void PointCatalogue::makeTriangleList()
      {
	std::sort(this->itsPointList.begin(),this->itsPointList.end());
	std::reverse(this->itsPointList.begin(), this->itsPointList.end());
	size_t maxPoint=this->itsPointList.size();
	if(this->itsTrimSize>2) maxPoint = std::min(this->itsTrimSize, this->itsPointList.size());

	this->itsTriangleList = std::vector<Triangle>(0);
	for (size_t i = 0; i < maxPoint - 2; i++) {
	  for (size_t j = i + 1; j < maxPoint - 1; j++) {
	    for (size_t k = j + 1; k < maxPoint; k++) {
	      Triangle tri(this->itsPointList[i], this->itsPointList[j], this->itsPointList[k]);

	      if (tri.ratio() < this->itsRatioLimit ) this->itsTriangleList.push_back(tri);
	    }
	  }
	}

	ASKAPLOG_INFO_STR(logger, "Generated a list of " << this->itsTriangleList.size() << " triangles");

      }

      bool PointCatalogue::crudeMatch(std::vector<Point> &other, double maxSep)
      {
	ASKAPLOG_DEBUG_STR(logger, "Performing crude match with maximum separation = " << maxSep);
	std::vector<Point>::iterator mine,theirs;
	std::vector<Point> newlist(0);
	for(mine=this->itsPointList.begin();mine<this->itsPointList.end();mine++){
	  bool stop=false;
	  for(theirs=other.begin();theirs<other.end()&&!stop;theirs++){
	    
	    if(theirs->sep(*mine) < maxSep){
	      newlist.push_back(*mine);
	      stop=true;
	    }
	  }

	}

	bool matchWorked = (newlist.size()>0);
	if(matchWorked){
	  ASKAPLOG_DEBUG_STR(logger, "Reduced list from " << this->itsPointList.size() << " points to " << newlist.size() << " points");
	  this->itsPointList = newlist;
	  this->makeTriangleList();
	}
	else{
	  ASKAPLOG_WARN_STR(logger, "Crude matching of point lists did not return any matches");
	}

	return matchWorked;

      }

    }

  }

}
