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
#include <coordutils/PositionUtilities.h>

#include <Common/ParameterSet.h>

#include <vector>
ASKAP_LOGGER(logger, ".matching.PointCat");

namespace askap {

  namespace analysis {

    namespace matching {

      PointCatalogue::PointCatalogue():
	  itsFilename(""),itsTrimSize(0),itsRatioLimit(defaultRatioLimit),itsFlagOffsetPositions(false),itsRAref(0.),itsDECref(0.),itsRadius(0.)
      {
	// if(this->itsTrimSize <= 2) ASKAPLOG_WARN_STR(logger, "Since trimsize<=2, the entire point list will be used to generate triangles.");
	this->itsFullPointList = std::vector<Point>(0); 
	this->itsWorkingPointList = std::vector<Point>(0); 
	this->itsTriangleList = std::vector<Triangle>(0);
     }

      PointCatalogue::PointCatalogue(LOFAR::ParameterSet &parset)
      {
	this->itsFilename = parset.getString("filename","");
	ASKAPCHECK(this->itsFilename != "", "No filename provided for the catalogue.");
	if(parset.getString("database", "Continuum")=="Selavy"){
	  parset.replace("useDeconvolvedSizes","true");  // set this to true, just so we don't have to worry about the SelavyImage
	}
	this->itsFactory = analysisutilities::ModelFactory(parset);
	this->itsTrimSize = parset.getUint32("trimsize",0);
	if(this->itsTrimSize <= 2) ASKAPLOG_WARN_STR(logger, "Since trimsize<=2, the entire point list will be used to generate triangles.");
	this->itsRatioLimit = parset.getFloat("ratioLimit",defaultRatioLimit);
	this->itsFullPointList = std::vector<Point>(0); 
	this->itsWorkingPointList = std::vector<Point>(0); 
	this->itsTriangleList = std::vector<Triangle>(0);
	std::string raRef=parset.getString("raRef","");
	std::string decRef=parset.getString("decRef","");
	this->itsFlagOffsetPositions = (raRef!="" && decRef!="");
	if(this->itsFlagOffsetPositions){
	  this->itsRAref = analysisutilities::raToDouble(raRef);
	  this->itsDECref = analysisutilities::decToDouble(decRef);
	  ASKAPLOG_DEBUG_STR(logger, "Using reference position (RA,DEC)=("<<this->itsRAref<<","<<this->itsDECref<<")");
	}
	else{
	  if(raRef!="" || decRef!="")
	    ASKAPLOG_WARN_STR(logger, "To offset positions, you need to provide both raRef and decRef parameters");
	}
	this->itsRadius = parset.getDouble("radius",-1.);
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
	this->itsFullPointList = other.itsFullPointList;
	this->itsWorkingPointList = other.itsWorkingPointList;
	this->itsTriangleList = other.itsTriangleList;
	this->itsFlagOffsetPositions = other.itsFlagOffsetPositions;
	this->itsRAref = other.itsRAref;
	this->itsDECref = other.itsDECref;
	this->itsRadius = other.itsRadius;
	return *this;
      }

      void PointCatalogue::read()
      {
	std::ifstream fin(this->itsFilename.c_str());
	this->itsFullPointList = std::vector<Point>(0);
	if(!fin.is_open())
	  ASKAPLOG_WARN_STR(logger,"Could not open filename " << this->itsFilename << ".");
	else {
	  std::string line;
	  while (getline(fin, line),
		 !fin.eof()) {
	    if (line[0] != '#') {  // ignore commented lines
	      analysisutilities::Spectrum *spec=this->itsFactory.read(line);
	      if(this->itsFlagOffsetPositions){
		  double radius=analysisutilities::angularSeparation(this->itsRAref,this->itsDECref,spec->raD(),spec->decD());
		  if(this->itsRadius < 0. || radius<this->itsRadius){
		      this->itsFullPointList.push_back(spec);
		      this->itsFullPointList.back().setX(analysisutilities::angularSeparation(this->itsRAref,0,spec->raD(),0.));
		      this->itsFullPointList.back().setY(analysisutilities::angularSeparation(0,this->itsDECref,0,spec->decD()));
		      ASKAPLOG_DEBUG_STR(logger, "Read source at position ("<<spec->raD()<<","<<spec->decD()
					 <<"), and storing point with (x,y)=("<<itsFullPointList.back().x()<<","<<itsFullPointList.back().y()<<")");
		  }
	      }
	      else this->itsFullPointList.push_back(spec);
	      delete spec;
	    }
	  }
	}
	this->itsWorkingPointList = this->itsFullPointList;
	this->makeTriangleList();

      }

      void PointCatalogue::makeTriangleList()
      {
	std::sort(this->itsWorkingPointList.begin(),this->itsWorkingPointList.end());
	std::reverse(this->itsWorkingPointList.begin(), this->itsWorkingPointList.end());
	size_t maxPoint=this->itsWorkingPointList.size();
	if(this->itsTrimSize>2) maxPoint = std::min(this->itsTrimSize, this->itsWorkingPointList.size());

	this->itsTriangleList = std::vector<Triangle>(0);
	for (size_t i = 0; i < maxPoint - 2; i++) {
	  for (size_t j = i + 1; j < maxPoint - 1; j++) {
	    for (size_t k = j + 1; k < maxPoint; k++) {
	      Triangle tri(this->itsWorkingPointList[i], this->itsWorkingPointList[j], this->itsWorkingPointList[k]);

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
	this->itsWorkingPointList = std::vector<Point>(0);
	for(mine=this->itsFullPointList.begin();mine<this->itsFullPointList.end();mine++){
	  bool stop=false;
	  for(theirs=other.begin();theirs<other.end()&&!stop;theirs++){
	    
	    if(theirs->sep(*mine) < maxSep){
	      this->itsWorkingPointList.push_back(*mine);
	      stop=true;
	    }
	  }

	}

	bool matchWorked = (this->itsWorkingPointList.size()>0);
	if(matchWorked){
	  ASKAPLOG_DEBUG_STR(logger, "Reduced list from " << this->itsFullPointList.size() << " points to " << this->itsWorkingPointList.size() << " points");
	  this->makeTriangleList();
	}
	else{
	  ASKAPLOG_WARN_STR(logger, "Crude matching of point lists did not return any matches");
	  this->itsWorkingPointList = this->itsFullPointList;
	}

	return matchWorked;

      }

    }

  }

}
