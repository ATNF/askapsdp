/// @file
///
/// XXX Notes on program XXX
///
/// @copyright (c) 2010 CSIRO
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

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <sourcefitting/SubThresholder.h>
#include <sourcefitting/RadioSource.h>
#include <sourcefitting/Component.h>

#include <duchamp/Cubes/cubes.hh>
#include <casa/Arrays/Slicer.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>

#include <math.h>
#include <vector>

///@brief Where the log messages go.
ASKAP_LOGGER(logger, ".subthresholder");

namespace askap {

  namespace analysis {
	  
    namespace sourcefitting {

      SubThresholder::SubThresholder(const SubThresholder &s) {
	operator=(s);
      }
		  
      SubThresholder::~SubThresholder(){
      }
	
      SubThresholder& SubThresholder::operator=(const SubThresholder &s) {
	if(this == &s) return *this;
	this->itsFirstGuess = s.itsFirstGuess;
	this->itsSourceBox = s.itsSourceBox;
	this->itsNumThresholds = s.itsNumThresholds;
	this->itsBaseThreshold = s.itsBaseThreshold;
	this->itsThreshIncrement = s.itsThreshIncrement;
	this->itsPeakFlux = s.itsPeakFlux;
	this->itsSourceSize = s.itsSourceSize;
	this->itsDim = s.itsDim;
	this->itsFluxArray = s.itsFluxArray;
	return *this;
		
      }
	  
	

      void SubThresholder::saveArray(RadioSource *src, casa::Matrix<casa::Double> pos, casa::Vector<casa::Double> &f) {
	int xmin = src->boxXmin();
	int ymin = src->boxYmin();
	int xsize= src->boxXsize();
	int ysize= src->boxYsize();
	int size=xsize*ysize;
	ASKAPLOG_DEBUG_STR(logger, "Primary array saving, with size = " << xsize*ysize); 
	this->itsFluxArray = casa::Vector<float>(size);
	PixelInfo::Object2D spatMap = src->getSpatialMap();
	
	for (int i = 0; i < size; i++) this->itsFluxArray[i] = 0.;
		
	for (size_t i = 0; i < f.size(); i++) {
	  int x = int(pos(i, 0));
	  int y = int(pos(i, 1));
	  
	  if (spatMap.isInObject(x, y)) {
	    int loc = (x - xmin) + xsize * (y - ymin);
	    this->itsFluxArray[loc] = float(f(i));
	  }
	}

      }
		  
      void SubThresholder::saveArray(casa::Vector<float> &f) {
	ASKAPLOG_DEBUG_STR(logger, "Secondary array saving, with size = " << f.size());
	this->itsFluxArray = f;
      }
		  
      void SubThresholder::define(RadioSource *src, casa::Matrix<casa::Double> pos, casa::Vector<casa::Double> &array){
	this->saveArray(src,pos,array);		
	this->define(src);			  
      }

      void SubThresholder::define(RadioSource *src, casa::Vector<float> &array){
	this->saveArray(array);		
	this->define(src);			  
      }
		  
      void SubThresholder::define(RadioSource *src){
			  
	this->itsPeakFlux = src->getPeakFlux();
	this->itsSourceSize = src->getSize();
				
	this->itsDim = casa::Vector<long>(2);
	this->itsDim[0] = src->boxXsize(); 
	this->itsDim[1] = src->boxYsize();

	this->itsFirstGuess.setPeak(this->itsPeakFlux);
	this->itsFirstGuess.setX(src->getXPeak());
	this->itsFirstGuess.setY(src->getYPeak());
	double a, b, c;
		
	if (this->itsSourceSize < 3) {
	  this->itsFirstGuess.setPA(0);
	  this->itsFirstGuess.setMajor(1.);
	  this->itsFirstGuess.setMinor(1.);
	}
	else {
	  src->getFWHMestimate(this->itsFluxArray.data(), a, b, c);
	  this->itsFirstGuess.setPA(a);
	  this->itsFirstGuess.setMajor(b);
	  this->itsFirstGuess.setMinor(c);
	}
		
	this->itsNumThresholds = src->fitparams().numSubThresholds();
	this->itsBaseThreshold = src->detectionThreshold() > 0 ? log10(src->detectionThreshold()) : -6.;
	this->itsThreshIncrement = (log10(this->itsPeakFlux) - this->itsBaseThreshold) / float(this->itsNumThresholds + 1);
		
	this->itsSourceBox = src->box();
		
      }
		  
      void SubThresholder::keepObject(PixelInfo::Object2D &obj){
			  
	for (int i = 0; i < this->itsDim[0]*this->itsDim[1]; i++) {
	  int xbox = i % this->itsDim[0];
	  int ybox = i / this->itsDim[1];
				  
	  if (!obj.isInObject(xbox + this->itsSourceBox.start()[0], ybox + this->itsSourceBox.start()[1])) 
	    this->itsFluxArray[i] = 0.;
	}
			  
      }

      std::vector<SubComponent> SubThresholder::find() {
		
	std::vector<SubComponent> fullList;

	if (this->itsSourceSize < 3) {
	  fullList.push_back(itsFirstGuess);
	  return fullList;
	}

	int threshCtr = 0;
	std::vector<PixelInfo::Object2D> objlist;
	std::vector<PixelInfo::Object2D>::iterator obj;
	bool keepGoing;

	duchamp::Image *theImage = new duchamp::Image(this->itsDim.data());

	if(this->itsFluxArray.size()>0){
	  ASKAPCHECK(this->itsFluxArray.size() == (this->itsDim[0]*this->itsDim[1]), "Size of flux array ("<<this->itsFluxArray.size()<<") doesn't match dimensions ("<<this->itsDim[0]<<"x"<<this->itsDim[1]<<"="<<this->itsDim[0]*this->itsDim[1]<<")!");
	  theImage->saveArray(this->itsFluxArray.data(), this->itsFluxArray.size());	
	}
	theImage->setMinSize(1);
		
	float thresh;
	do {
	  threshCtr++;
	  thresh = pow(10., this->itsBaseThreshold + threshCtr * this->itsThreshIncrement);
	  theImage->stats().setThreshold(thresh);
	  objlist = theImage->findSources2D();
	  keepGoing = (objlist.size() == 1);
	} while (keepGoing && (threshCtr < this->itsNumThresholds));

	delete theImage;
		
	if (!keepGoing) {
		  
	  FittingParameters baseParams;
	  baseParams.setNumSubThresholds(this->itsNumThresholds);
			
	  for (obj = objlist.begin(); obj < objlist.end(); obj++) {

	    RadioSource *src = new RadioSource;
	    src->addChannel(0, *obj);
	    src->setFitParams(baseParams);
	    src->setDetectionThreshold(thresh);
	    src->setBox(this->itsSourceBox);
	    src->calcFluxes(this->itsFluxArray.data(),this->itsDim.data());
	    duchamp::Param par;
	    par.setXOffset(this->itsSourceBox.start()[0]);
	    par.setYOffset(this->itsSourceBox.start()[1]);
	    src->setOffsets(par);
	    src->addOffsets();
	    SubThresholder *newthresher = new SubThresholder();
	    newthresher->define(src,this->itsFluxArray);
	    newthresher->keepObject(*obj);
	    std::vector<SubComponent> newlist = newthresher->find();
	    delete newthresher;
	    delete src;
	    for (uInt i = 0; i < newlist.size(); i++) fullList.push_back(newlist[i]);
	  }
	} 
	else {
	  fullList.push_back(this->itsFirstGuess);
	}
		
	if (fullList.size() > 1) {
	  std::sort(fullList.begin(), fullList.end());
	  std::reverse(fullList.begin(), fullList.end());
	}

	return fullList;

      }

    }
  }

}
