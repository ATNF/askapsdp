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

#include <math.h>

///@brief Where the log messages go.
ASKAP_LOGGER(logger, ".subthresholder");

namespace askap {

  namespace analysis {
	  
	  namespace sourcefitting {

    SubThresholder::SubThresholder(const SubThresholder &s) {
      operator=(s);
    }
	
    SubThresholder& SubThresholder::operator=(const SubThresholder &s) {
		if(this == &s) return *this;
		this->itsFirstGuess = s.itsFirstGuess;
		this->itsImage = s.itsImage;
		this->itsSourceBox = s.itsSourceBox;
		this->itsNumThresholds = s.itsNumThresholds;
		this->itsBaseThreshold = s.itsBaseThreshold;
		this->itsThreshIncrement = s.itsThreshIncrement;
		this->itsPeakFlux = s.itsPeakFlux;
		this->itsSourceSize = s.itsSourceSize;
		if(itsDim != 0) delete [] this->itsDim;
		this->itsDim = new long[2];
		this->itsDim[0] = s.itsDim[0];
		this->itsDim[1] = s.itsDim[1];
		if(this->itsFluxArray != 0) delete [] this->itsFluxArray;
		this->itsFluxArray = new float[this->itsDim[0]*this->itsDim[1]];
		for(int i=0;i<this->itsDim[0]*this->itsDim[1];i++)
			this->itsFluxArray[i] = s.itsFluxArray[i];
		return *this;
		
    }
	  
	void SubThresholder::saveArray(float *array, long *dim) {

		ASKAPLOG_DEBUG_STR(logger, "Setting the SubThresholder's array");
		
		if(this->itsFluxArray != 0)
			delete [] this->itsFluxArray;

		size_t size = dim[0]*dim[1];
		this->itsFluxArray = new float[size];
		for(size_t i=0;i<size;i++)
			this->itsFluxArray[i] = array[i];
		
		ASKAPLOG_DEBUG_STR(logger, "SubThresholder's array set successfully");
		

	}
	
    void SubThresholder::define(RadioSource *src, float *array) {

		ASKAPLOG_DEBUG_STR(logger, "Defining a SubThresholder");
		this->itsPeakFlux = src->getPeakFlux();
		this->itsSourceSize = src->getSize();
		
		this->itsSourceBox = src->box();
		this->itsDim = new long[2];
		this->itsDim[0] = src->boxXsize(); 
		this->itsDim[1] = src->boxYsize();
		
		this->saveArray(array,this->itsDim);		
		
		ASKAPLOG_DEBUG_STR(logger, "Defining duchamp Image");
		this->itsImage = duchamp::Image(this->itsDim);
		ASKAPLOG_DEBUG_STR(logger, "Setting Image's array");
		if(array!=0)
			this->itsImage.saveArray(array, src->boxSize());
		this->itsImage.setMinSize(1);
		ASKAPLOG_DEBUG_STR(logger, "Image defined");
		ASKAPLOG_DEBUG_STR(logger, "About to set initial guess for subcomponent parameters");
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
			src->getFWHMestimate(array, a, b, c);
			this->itsFirstGuess.setPA(a);
			this->itsFirstGuess.setMajor(b);
			this->itsFirstGuess.setMinor(c);
		}
		
		ASKAPLOG_DEBUG_STR(logger, "Have defined initial guess for subcomponent parameters " << this->itsFirstGuess);
		
		this->itsNumThresholds = src->fitparams().numSubThresholds();
		this->itsBaseThreshold = src->detectionThreshold() > 0 ? log10(src->detectionThreshold()) : -6.;
		this->itsThreshIncrement = (log10(this->itsPeakFlux) - this->itsBaseThreshold) / float(this->itsNumThresholds + 1);
		
		
		
    }

    std::vector<SubComponent> SubThresholder::find() {

		ASKAPLOG_DEBUG_STR(logger, "Commencing a search for subcomponents.");
		
		std::vector<SubComponent> fullList;

		if (this->itsSourceSize < 3) {
			fullList.push_back(itsFirstGuess);
			return fullList;
		}

		int threshCtr = 0;
      std::vector<PixelInfo::Object2D> objlist;
      std::vector<PixelInfo::Object2D>::iterator obj;
      bool keepGoing;

      do {
		  threshCtr++;
		  float thresh = pow(10., this->itsBaseThreshold + threshCtr * this->itsThreshIncrement);
		  this->itsImage.stats().setThreshold(thresh);
		  objlist = this->itsImage.findSources2D();
		  keepGoing = (objlist.size() == 1);
      } while (keepGoing && (threshCtr < this->itsNumThresholds));

      if (!keepGoing) {
		  
		  for (obj = objlist.begin(); obj < objlist.end(); obj++) {
			  // now change the flux array so that we only see the current object
			  float *newfluxarray = new float[this->itsDim[0]*this->itsDim[1]];

			  for (int i = 0; i < this->itsDim[0]*this->itsDim[1]; i++) {
				  int xbox = i % this->itsDim[0];
				  int ybox = i / this->itsDim[1];

				  if (obj->isInObject(xbox + this->itsSourceBox.start()[0], ybox + this->itsSourceBox.start()[1])) 
					  newfluxarray[i] = this->itsFluxArray[i];
				  else newfluxarray[i] = 0.;
			  }

			  
			  // Now clone the current SubThresholder, change the flux array, then call find() on it
			  SubThresholder newthresher = *this;
			  newthresher.saveArray(newfluxarray, this->itsDim);
			  std::vector<SubComponent> newlist = newthresher.find();
			  
			  delete [] newfluxarray;

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
