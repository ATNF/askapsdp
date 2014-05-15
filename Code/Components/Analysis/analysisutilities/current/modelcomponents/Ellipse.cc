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
#include <modelcomponents/Ellipse.h>
#include <iostream>
#include <math.h>
#include <algorithm>

namespace askap {

    namespace analysisutilities {

	Ellipse::Ellipse():
	    itsX0(0.),itsY0(0.),itsMaj(0.),itsMin(0.),itsAngle(0.)
	{
	    this->initialise();
	}

	Ellipse::Ellipse(double x0, double y0, double maj, double min, double pa): 
	    itsX0(x0),itsY0(y0),itsMaj(std::max(maj,min)),itsMin(std::min(maj,min)),itsAngle(pa+M_PI/2.) 
	{
	    this->initialise();
	}


	void Ellipse::initialise()
	{	 
	    itsCos=cos(itsAngle); 
	    itsSin=sin(itsAngle);
	    itsMajCos=itsMaj*itsCos; 
	    itsMajSin=itsMaj*itsSin;
	    itsMinSin=itsMin*itsSin; 
	    itsMinCos=itsMin*itsCos;
	    itsArea=itsMaj*itsMin*M_PI;
	}

	Ellipse::Ellipse(const Ellipse& other)
	{
	    this->operator=(other);
	}

	Ellipse& Ellipse::operator= (const Ellipse& other)
	{
	    if(this == &other) return *this;
	    this->itsX0 =      other.itsX0;     
	    this->itsY0 =      other.itsY0;     
	    this->itsMaj =     other.itsMaj;    
	    this->itsMin =     other.itsMin;    
	    this->itsAngle =   other.itsAngle;  
	    this->itsCos =     other.itsCos;    
	    this->itsSin =     other.itsSin;    
	    this->itsMajCos =  other.itsMajCos; 
	    this->itsMajSin =  other.itsMajSin; 
	    this->itsMinCos =  other.itsMinCos; 
	    this->itsMinSin =  other.itsMinSin; 
	    this->itsArea =    other.itsArea;   
	    return *this;
	}

	std::ostream& operator<<(std::ostream &theStream, Ellipse &ell)
	{
	    theStream << "[ ("<<ell.itsX0<<","<<ell.itsY0<<"), "<<ell.itsMaj<<"x"<<ell.itsMin<<", "<<(ell.itsAngle-M_PI/2)*180./M_PI << " ]";
	    return theStream;
	}


    }

}
