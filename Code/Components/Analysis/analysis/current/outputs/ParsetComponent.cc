/// @file
///
/// Class to manage the data for a component written to a parset
///
/// @copyright (c) 2014 CSIRO
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
/// @author Matthew Whiting <Matthew.Whiting@csiro.au>
///
#include <outputs/ParsetComponent.h>
#include <askap_analysis.h>
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <outputs/CataloguePreparation.h>

#include <sourcefitting/RadioSource.h>
#include <coordutils/PositionUtilities.h>
#include <mathsutils/MathsUtils.h>

#include <scimath/Functionals/Gaussian2D.h>

///@brief Where the log messages go.
ASKAP_LOGGER(logger, ".parsetcomponent");

namespace askap {

    namespace analysis { 

	ParsetComponent::ParsetComponent():
	    itsHead(0),itsFlagReportSize(true)
	{
	}

	ParsetComponent::ParsetComponent(const ParsetComponent& other)
	{
	    this->operator=(other);
	}

	ParsetComponent& ParsetComponent::operator= (const ParsetComponent& other)
	{
	    if(this == &other) return *this;
	    this->itsHead = other.itsHead;
	    this->itsFlux = other.itsFlux;
	    this->itsRAref = other.itsRAref;
	    this->itsDECref = other.itsDECref;
	    this->itsRAoff = other.itsRAoff;
	    this->itsDECoff = other.itsDECoff;
	    this->itsFlagReportSize = other.itsFlagReportSize;
	    this->itsBmaj = other.itsBmaj;
	    this->itsBmin = other.itsBmin;
	    this->itsBpa = other.itsBpa;
	    this->itsID = other.itsID;
	    return *this;
	}


	void ParsetComponent::defineComponent(sourcefitting::RadioSource *src, size_t fitNum, std::string fitType)
	{
	    ASKAPCHECK(this->itsHead!=0,"Have not set the FITS header for the parset component");

	    casa::Gaussian2D<Double> gauss=src->gaussFitSet(fitType).at(fitNum);

	    double thisRA,thisDec,zworld;
	    this->itsHead->pixToWCS(gauss.xCenter(), gauss.yCenter(), src->getZcentre(),
                                    thisRA, thisDec, zworld);
	    this->itsDECoff = (thisDec-this->itsDECref)*M_PI/180.;
	    this->itsRAoff = (thisRA-this->itsRAref)*M_PI/180. * cos(this->itsDECref*M_PI/180.);

	    this->itsFlux = gauss.flux();
	    if (this->itsHead->needBeamSize())
		this->itsFlux /= this->itsHead->beam().area(); // Convert from Jy/beam to Jy

	    if(this->itsFlagReportSize){
		std::vector<Double> deconv =
                    analysisutilities::deconvolveGaussian(gauss, this->itsHead->getBeam());
		this->itsBmaj = deconv[0]*this->itsHead->getAvPixScale()*3600.;
		this->itsBmin = deconv[1]*this->itsHead->getAvPixScale()*3600.;
		this->itsBpa  = deconv[2]*180. / M_PI;
	    }
	    else this->itsBmaj = this->itsBmin = this->itsBpa = 0.;

	    std::stringstream ID;
	    ID << src->getID() << getSuffix(fitNum);
	    this->itsID=ID.str();
	}

	std::ostream& operator<<(std::ostream &theStream, ParsetComponent &comp)
	{
	    std::stringstream prefix;
	    prefix << "sources.src"<< comp.itsID;

	    theStream << prefix.str() << ".flux.i        = " << comp.itsFlux <<"\n";
	    theStream << prefix.str() << ".direction.ra  = " << comp.itsRAoff << "\n";
	    theStream << prefix.str() << ".direction.dec = " << comp.itsDECoff << "\n";
	    theStream << prefix.str() << ".shape.bmaj  = " << comp.itsBmaj << "\n";
	    theStream << prefix.str() << ".shape.bmin  = " << comp.itsBmin << "\n";
	    theStream << prefix.str() << ".shape.bpa   = " << comp.itsBpa << "\n";

	    return theStream;
	}



    }

}
