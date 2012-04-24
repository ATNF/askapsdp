/// @file
///
/// Base functions for spectral-line profile classes
///
/// @copyright (c) 2008 CSIRO
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
#include <askap_analysisutilities.h>

#include <modelcomponents/GaussianProfile.h>
#include <coordconversions/SpectralUtilities.h>
#include <iostream>
#include <math.h>
#include <stdlib.h>

#include <scimath/Functionals/Gaussian1D.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

ASKAP_LOGGER(logger, ".gaussianprofile");


namespace askap {

    namespace analysisutilities {

 
        GaussianProfile::GaussianProfile():
                Spectrum()
        {
	  this->itsRestFreq = nu0_HI;
	  this->itsAxisType = FREQUENCY;
        }

        GaussianProfile::GaussianProfile(float restfreq):
                Spectrum()
        {
	  this->itsRestFreq = restfreq;
	  this->itsAxisType = FREQUENCY;
        }

        GaussianProfile::GaussianProfile(double &height, double &centre, double &width, AXISTYPE &type):
	  Spectrum(), itsGaussian(height,centre,width), itsAxisType(type)
	{
	  this->itsRestFreq = nu0_HI;
	}

        GaussianProfile::GaussianProfile(const GaussianProfile& h):
                Spectrum(h)
        {
            operator=(h);
        }

        GaussianProfile& GaussianProfile::operator= (const GaussianProfile& h)
        {
            if (this == &h) return *this;

            ((Spectrum &) *this) = h;
            this->itsGaussian = h.itsGaussian;
	    this->itsAxisType = h.itsAxisType;
	    this->itsRestFreq = h.itsRestFreq;
            return *this;
        }

        void GaussianProfile::define(std::string &line)
        {
            /// @details Defines a GaussianProfile object from a line of
            /// text from an ascii file. This line should be formatted in
            /// the correct way to match the output from the appropriate
            /// python script. The columns should be: RA - DEC - Flux -
            /// Peak height - central position - FWHM.
            /// @param line A line from the ascii input file

	    double peak, centre, width;
            std::stringstream ss(line);
            ss >> this->itsRA >> this->itsDec >> this->itsFlux >> this->itsMaj >> this->itsMin >> this->itsPA >> peak >> centre >> width;

	    this->checkShape();
	    if(this->itsMaj<this->itsMin) std::swap(this->itsMaj,this->itsMin);

	    this->itsGaussian.setHeight(peak);
	    this->itsGaussian.setCenter(centre);
	    this->itsGaussian.setWidth(width);

        }

      double GaussianProfile::flux(double nu, int istokes)
        {
	  if(istokes>0) return 0.;
	  else{
	    double flux=0.;
	    switch(this->itsAxisType){
	    case PIXEL:
	      ASKAPLOG_ERROR_STR(logger, "Cannot use axis type PIXEL");
	      break;
	    case FREQUENCY:
	      flux = this->itsGaussian(nu);
	      break;
	    case VELOCITY:
	      flux = this->itsGaussian(freqToVel(nu,this->itsRestFreq));
	      break;
	    case REDSHIFT:
	      flux = this->itsGaussian(freqToRedshift(nu,this->itsRestFreq));
	      break;
	    }
	    return flux;
	  }
	}

        double GaussianProfile::flux(double nu1, double nu2, int istokes)
	{
	  if(istokes>0) return 0.;
	  else {
	    double scale,first,last,v1,v2,z1,z2;
	    double flux=0.;
	    switch(this->itsAxisType){
	    case PIXEL:
	      ASKAPLOG_ERROR_STR(logger, "Cannot use axis type PIXEL");
	      break;
	    case FREQUENCY:
	      scale = this->itsGaussian.width() * this->itsGaussian.height()  / (2.*sqrt(M_LN2)) / M_2_SQRTPI;
	      first = (std::min(nu1,nu2)-this->itsGaussian.center()) * 2 * sqrt(M_LN2) / this->itsGaussian.width();
	      last = (std::max(nu1,nu2)-this->itsGaussian.center()) * 2 * sqrt(M_LN2) / this->itsGaussian.width();
	      flux = scale * (erf(last) - erf(first));
	      break;
	    case VELOCITY:
	      v1 = freqToVel(nu1,this->itsRestFreq);
	      v2 = freqToVel(nu2,this->itsRestFreq);
	      scale =  this->itsGaussian.width() * this->itsGaussian.height()  / (2.*sqrt(M_LN2)) / M_2_SQRTPI;
	      first = (std::min(v1,v2)-this->itsGaussian.center()) * 2 * sqrt(M_LN2) / this->itsGaussian.width();
	      last = (std::max(v1,v2)-this->itsGaussian.center()) * 2 * sqrt(M_LN2) / this->itsGaussian.width();
	      flux = scale * (erf(last) - erf(first));
	      break;
	    case REDSHIFT:
	      z1 = freqToRedshift(nu1,this->itsRestFreq);
	      z2 = freqToRedshift(nu2,this->itsRestFreq);
	      scale =  this->itsGaussian.width() * this->itsGaussian.height()  / (2.*sqrt(M_LN2)) / M_2_SQRTPI;
	      first = (std::min(z1,z2)-this->itsGaussian.center()) * 2 * sqrt(M_LN2) / this->itsGaussian.width();
	      last = (std::max(z1,z2)-this->itsGaussian.center()) * 2 * sqrt(M_LN2) / this->itsGaussian.width();
	      flux = scale * (erf(last) - erf(first));
	      break;
	    }
	    flux = flux / fabs(nu2-nu1);
	    // ASKAPLOG_DEBUG_STR(logger, "Flux between " << nu1 << " and " << nu2 << " is " << flux << " with scale=" << scale << " and basic integral b/w " << first << "and " << last << " = " << erf(last)-erf(first) );
	    return flux ;
	  }
	}

        std::ostream& operator<< (std::ostream& theStream, GaussianProfile &prof)
        {
            /// @details Prints a summary of the parameters to the stream
            /// @param theStream The destination stream
            /// @param prof The profile object
            /// @return A reference to the stream

            theStream << "Gaussian profile summary:\n";
            theStream << prof.itsGaussian << "\n";
            return theStream;
        }

    }

}
