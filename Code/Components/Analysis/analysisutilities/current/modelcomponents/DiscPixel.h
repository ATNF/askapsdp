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
#ifndef ASKAP_ANALYSIS_DISCPIXEL_H_
#define ASKAP_ANALYSIS_DISCPIXEL_H_
#include <modelcomponents/Ellipse.h>
#include <vector>

namespace askap {

    namespace analysisutilities {

	const double defaultResolution = 1.e-3;
	const double defaultTresolution = 1000.;
	const unsigned int defaultDecimationFactor = 10;

	class DiscPixel
	{
	public:
	    DiscPixel(double x, double y);
	    DiscPixel(const DiscPixel& other){operator=(other);};
	    DiscPixel& operator= (const DiscPixel& other);  
	    virtual ~DiscPixel(){};
	    double flux();
	    std::vector<DiscPixel> decimate();
	    std::vector<DiscPixel> processedSublist();

	    void addTmin(double t){if(itsTmin>0) itsTmin=std::min(itsTmin,t); else itsTmin=t;};
	    void addTmax(double t){if(itsTmax>0) itsTmax=std::max(itsTmin,t); else itsTmax=t;};

	    void setResolutionLimit(double res){itsResolutionLimit = res;};
	    void setEllipse(Ellipse *ell){itsEllipse=ell;};
	    void setIsEdge(bool b){itsIsEdge = b;};
	    void setDecimationFactor(unsigned int i){itsDecimationFactor = i;};

	    double x(){return itsX;}
	    double y(){return itsY;}
	    double width(){return itsWidth;};
	    double tmin(){return itsTmin;};
	    double tmax(){return itsTmax;};
	    double resolutionLimit(){return itsResolutionLimit;};
	    bool isEdge(){return itsIsEdge;};
	    unsigned int decimationFactor(){return itsDecimationFactor;};
	    

	protected:
	    double itsX;
	    double itsY;
	    double itsWidth;
	    double itsTmin;
	    double itsTmax;
	    Ellipse *itsEllipse;
	    double itsResolutionLimit;
	    bool itsIsEdge;
	    unsigned int itsDecimationFactor;

	};


    }

}


#endif
