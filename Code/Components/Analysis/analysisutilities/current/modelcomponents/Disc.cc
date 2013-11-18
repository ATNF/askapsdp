/// @file
///
/// Class to calculate pixel fluxes for a uniform-surface-brightness elliptical disc
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
#include <modelcomponents/Disc.h>
#include <modelcomponents/DiscEllipse.h>
#include <modelcomponents/DiscPixel.h>
#include <modelcomponents/Spectrum.h>
#include <Common/ParameterSet.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

ASKAP_LOGGER(logger, ".disc");

namespace askap { 
    
    namespace analysisutilities {


	Disc::Disc()
	{
	    this->itsResolutionLimit = defaultResolution;
	    this->itsTresolution = defaultTresolution;
	    this->itsDecimationFactor = defaultDecimationFactor;
	}

	Disc::Disc(const LOFAR::ParameterSet &parset)
	{
	    this->itsResolutionLimit = parset.getDouble("resolutionLimit",defaultResolution);
	    this->itsTresolution = parset.getDouble("tResolution",defaultTresolution);
	    this->itsDecimationFactor = parset.getUint("decimationFactor",defaultDecimationFactor);
	}

	Disc::Disc(const Disc& other)
	{
	    this->operator=(other);
	}

	Disc& Disc::operator= (const Disc& other)
	{
	    if(this == &other) return *this;
	    this->itsEllipse = other.itsEllipse;
	    this->itsPixelSet = other.itsPixelSet;
	    this->itsResolutionLimit = other.itsResolutionLimit;
	    this->itsTresolution = other.itsTresolution;
	    this->itsDecimationFactor = other.itsDecimationFactor;
	    return *this;
	}

	void Disc::setup(double x0, double y0, double maj, double min, double pa)
	{
	    this->itsEllipse = DiscEllipse(x0,y0,maj,min,pa);
	    this->itsPixelSet = this->itsEllipse.boundingSet(2.*M_PI/this->itsTresolution);
	    for(size_t i=0; i<this->itsPixelSet.size(); i++){
		this->itsPixelSet[i].setResolutionLimit(this->itsResolutionLimit);
		this->itsPixelSet[i].setDecimationFactor(this->itsDecimationFactor);
	    }	    
	}

	// void Disc::setup(Spectrum *spec, double x0, double y0)
	// {
	//     this->setup(x0,y0,spec->maj(),spec->min(),spec->pa());
	// }

	double Disc::flux(int x, int y)
	{
	    // ASKAPCHECK(x>=this->xmin(), "Disc::flux : value of x ("<<x<<") is out of range ["<<this->xmin()<<","<<this->xmax()<<"]");
	    // ASKAPCHECK(x<=this->xmax(), "Disc::flux : value of x ("<<x<<") is out of range ["<<this->xmin()<<","<<this->xmax()<<"]");
	    // ASKAPCHECK(y>=this->ymin(), "Disc::flux : value of y ("<<y<<") is out of range ["<<this->ymin()<<","<<this->ymax()<<"]");
	    // ASKAPCHECK(y<=this->ymax(), "Disc::flux : value of y ("<<y<<") is out of range ["<<this->ymin()<<","<<this->ymax()<<"]");
	    
	    bool pixIsValid = (x>=this->xmin() && x<=this->xmax() && y>=this->ymin() && y<=this->ymax());

	    if(pixIsValid){
		if(this->itsPixelSet.size()==1) return 1.;
		else{
		    size_t dimx=this->xmax()-this->xmin()+1;
		    size_t pix=(x-this->xmin()) + dimx * (y-this->ymin());
		    return this->itsPixelSet[pix].flux() / this->itsEllipse.area();
		}
	    }
	    else return 0.;
	}

	
	std::ostream &operator<<(std::ostream &theStream, Disc &disc)
	{
	    theStream << disc.itsEllipse;
	    return theStream;
	}


	// void Disc::populate(float *array)//, std::vector<unsigned int> axes, Flux)
	// {

	// }

    }

}
