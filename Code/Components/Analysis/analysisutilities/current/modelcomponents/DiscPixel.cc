/// @file
///
/// Class to handle operations on a single pixel that makes up a uniform-surface-brightness elliptical disc
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
#include <modelcomponents/DiscPixel.h>
#include <modelcomponents/Ellipse.h>
#include <vector>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
ASKAP_LOGGER(logger, ".discpixel");

namespace askap {

    namespace analysisutilities {

	DiscPixel::DiscPixel(double x, double y):
	    itsX(x),itsY(y)
	{
	    this->itsTmin = -1.;
	    this->itsTmax = -1.;
	    this->itsWidth = 1.;
	    this->itsIsEdge = false;
	    this->itsResolutionLimit = defaultResolution;
	    this->itsDecimationFactor = defaultDecimationFactor;
	}


	DiscPixel& DiscPixel::operator=(const DiscPixel& other)
	{
	    if(this==&other) return *this;
	    this->itsX = other.itsX;
	    this->itsY = other.itsY;
	    this->itsWidth = other.itsWidth;
	    this->itsTmin = other.itsTmin;
	    this->itsTmax = other.itsTmax;
	    this->itsEllipse = other.itsEllipse;
	    this->itsResolutionLimit = other.itsResolutionLimit;
	    this->itsIsEdge = other.itsIsEdge;
	    this->itsDecimationFactor = other.itsDecimationFactor;
	    return *this;    
	}

	std::vector<DiscPixel> DiscPixel::decimate()
	{
	    int num=this->itsDecimationFactor*this->itsDecimationFactor;
	    std::vector<DiscPixel> outlist(num,*this);
	    double xmin=itsX-itsWidth/2.,ymin=itsY-itsWidth/2.;
	    for(int i=0;i<num;i++){
		outlist[i].itsWidth /= this->itsDecimationFactor;
		outlist[i].itsX = xmin + (i%this->itsDecimationFactor + 0.5)*outlist[i].itsWidth;
		outlist[i].itsY = ymin + (i/this->itsDecimationFactor + 0.5)*outlist[i].itsWidth;
		outlist[i].itsIsEdge = false;
		outlist[i].itsTmin=-1.;
		outlist[i].itsTmax=-1.;
	    }
	    // ASKAPLOG_DEBUG_STR(logger, "Returning decimated sublist of length " << outlist.size() << "   (should be " << num<<")");
	    return outlist;
	}

	double DiscPixel::flux()
	{

	    if(!this->itsIsEdge){
		if(this->itsEllipse->isIn(this->itsX,this->itsY)) return this->itsWidth*this->itsWidth;
		else return 0.;
	    }
	    else{
		if(this->itsWidth < this->itsResolutionLimit){ // stopping condition
		    int nVerticesGood=0;
		    if(this->itsEllipse->isIn(this->itsX+this->itsWidth/2.,this->itsY+this->itsWidth/2.)) nVerticesGood++;
		    if(this->itsEllipse->isIn(this->itsX-this->itsWidth/2.,this->itsY+this->itsWidth/2.)) nVerticesGood++;
		    if(this->itsEllipse->isIn(this->itsX+this->itsWidth/2.,this->itsY-this->itsWidth/2.)) nVerticesGood++;
		    if(this->itsEllipse->isIn(this->itsX-this->itsWidth/2.,this->itsY-this->itsWidth/2.)) nVerticesGood++;
		    return nVerticesGood * this->itsWidth * this->itsWidth / 4.;
		}
		else {
		    std::vector<DiscPixel> subpixels = this->processedSublist();
		    // ASKAPLOG_DEBUG_STR(logger, "Got list of subpixels of length " << subpixels.size());
		    double flux=0;
		    for(size_t i=0;i<subpixels.size();i++) flux += subpixels[i].flux();
		    return flux;
		}	
	
	    }
    

	}
	
	std::vector<DiscPixel> DiscPixel::processedSublist()
	{

	    std::vector<DiscPixel> subpixels=this->decimate();
	    if(this->itsTmin > this->itsTmax) this->itsTmax += 2.*M_PI;

	    double xmin=this->itsX-this->itsWidth/2.;
	    double ymin=this->itsY-this->itsWidth/2.;
	    double pixstep=subpixels[0].itsWidth;
	    int oldx=0,oldy=0;
	    size_t oldpos=0;
	    double tstep=(this->itsTmax-this->itsTmin)/defaultTresolution;
	    for(double t=this->itsTmin; t<this->itsTmax; t += tstep){ 
		double x=this->itsEllipse->parametricX(t);
		double y=this->itsEllipse->parametricY(t);
		if(fabs(x-this->itsX)<this->itsWidth/2. && fabs(y-this->itsY)<this->itsWidth/2.){
		    int xloc= lround((x-xmin-pixstep/2.)/pixstep);
		    int yloc= lround((y-ymin-pixstep/2.)/pixstep);
		    oldpos=(oldx) + (oldy)*this->itsDecimationFactor;
		    ASKAPASSERT(oldpos<subpixels.size());
		    size_t newpos=(xloc) + (yloc)*this->itsDecimationFactor;
		    ASKAPASSERT(newpos<subpixels.size());
		    if(xloc!=oldx || yloc!=oldy || t==this->itsTmin){
			if(t>this->itsTmin) subpixels[oldpos].addTmax(t);
			oldx=xloc;
			oldy=yloc;
			subpixels[newpos].addTmin(t-tstep);
		    }
		    subpixels[newpos].itsIsEdge = true;
		}
	    }
	    subpixels[oldpos].addTmax(this->itsTmax);

	    return subpixels;

	}



    }

}
