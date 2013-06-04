/// @file
///
/// Class to handle basic calculations needed for ellipses
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
#include <modelcomponents/DiscEllipse.h>
#include <modelcomponents/Ellipse.h>
#include <modelcomponents/DiscPixel.h>
#include <math.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
ASKAP_LOGGER(logger, ".discellipse");

namespace askap {

    namespace analysisutilities {

	DiscEllipse::DiscEllipse():
	    Ellipse()
	{
	}

	DiscEllipse::DiscEllipse(double x0, double y0, double maj, double min, double pa): 
	    Ellipse(x0,y0,maj,min,pa)
	{
	}

	DiscEllipse::DiscEllipse(const DiscEllipse& other):
	    Ellipse(other)
	{
	    this->operator=(other);
	}

	DiscEllipse& DiscEllipse::operator= (const DiscEllipse& other)
	{
	    if(this == &other) return *this;
            ((Ellipse &) *this) = other;
	    this->itsXmin = other.itsXmin;
	    this->itsXmax = other.itsXmax;
	    this->itsYmin = other.itsYmin;
	    this->itsYmax = other.itsYmax;
	    return *this;
	}


	std::vector<DiscPixel> DiscEllipse::boundingSet(unsigned int numberOfSteps)
	{
	    std::vector<DiscPixel> pixlist;
	    this->itsXmin = lround(this->itsX0-this->itsMaj);
	    this->itsXmax = lround(this->itsX0+this->itsMaj);
	    this->itsYmin = lround(this->itsY0-this->itsMaj);
	    this->itsYmax = lround(this->itsY0+this->itsMaj);

	    for(int y=this->itsYmin; y<=this->itsYmax; y++){ 
		for(int x=this->itsXmin; x<=this->itsXmax; x++){ 

		    DiscPixel pix(x,y);
		    pix.setEllipse(this);
		    pixlist.push_back(pix);

		}
	    }

	    int dimx=this->itsXmax-this->itsXmin+1;
	    int oldx,oldy;
	    size_t oldpos;
	    double tstep=2.*M_PI/double(numberOfSteps);
	    for(unsigned int i=0;i<numberOfSteps;i++) {
		double t=i*tstep;
		int xloc = lround(this->parametricX(t));
		int yloc = lround(this->parametricY(t));
		oldpos=(oldx-this->itsXmin)+(oldy-this->itsYmin)*dimx;
		size_t newpos=(xloc-this->itsXmin)+(yloc-this->itsYmin)*dimx;
		if(xloc!=oldx || yloc!=oldy || i==0){
		    if(i>0) pixlist[oldpos].addTmax(t);
		    oldx=xloc;
		    oldy=yloc;
		    pixlist[newpos].addTmin(t-tstep);
		}
		pixlist[newpos].setIsEdge(true);
	    }
	    pixlist[oldpos].addTmax(2*M_PI);

	    return pixlist;
	}

    }

}
