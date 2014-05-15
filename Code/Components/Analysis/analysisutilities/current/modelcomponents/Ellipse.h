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
#ifndef ASKAP_ANALYSIS_ELLIPSE_H_
#define ASKAP_ANALYSIS_ELLIPSE_H_
#include <iostream>
#include <math.h>

namespace askap {

    namespace analysisutilities {

	class Ellipse
	{
	public:
	    Ellipse();
	    Ellipse(double x0, double y0, double maj, double min, double pa);
	    Ellipse(const Ellipse& other);
	    Ellipse& operator= (const Ellipse& other);
	    virtual ~Ellipse(){};
	    virtual void initialise();

	    virtual double parametricX(double t){return itsX0 + itsMajCos*cos(t) - itsMinSin*sin(t);};
	    virtual double parametricY(double t){return itsY0 + itsMajSin*cos(t) + itsMinCos*sin(t);};
	    virtual std::pair<double,double> parametric(double t){double cost=cos(t); double sint=sin(t); return std::pair<double,double>(itsX0+itsMajCos*cost-itsMinSin*sint,itsY0+itsMajSin*cost+itsMinCos*sint);};
	    virtual double nonRotX(double x, double y){return (x-itsX0)*itsCos+(y-itsY0)*itsSin;};
	    virtual double nonRotY(double x, double y){return -(x-itsX0)*itsSin+(y-itsY0)*itsCos;};
	    virtual std::pair<double,double> nonRot(double x, double y){std::pair<double,double> pos; pos.first=(x-itsX0)*itsCos+(y-itsY0)*itsSin; pos.second=-(x-itsX0)*itsSin+(y-itsY0)*itsCos; return pos;};
	    virtual bool isIn(double x, double y){std::pair<double,double> pos=nonRot(x,y); return ( pos.first*pos.first/(itsMaj*itsMaj) + pos.second*pos.second/(itsMin*itsMin)) < 1.;};
	    virtual double area(){return itsArea;};
	    virtual bool is2D(){return itsMin>0.;};

	    friend std::ostream& operator<<(std::ostream &theStream, Ellipse &ell);


	protected:
	    double itsX0;
	    double itsY0;
	    double itsMaj;
	    double itsMin;
	    double itsAngle;
	    double itsCos;
	    double itsSin;
	    double itsMajCos;
	    double itsMajSin;
	    double itsMinCos;
	    double itsMinSin;
	    double itsArea;
    
	};



    }

}




#endif
