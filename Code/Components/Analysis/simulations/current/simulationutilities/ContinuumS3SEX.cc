/// @file
///
/// Provides utility functions for simulations package
///
/// @copyright (c) 2007 CSIRO
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
/// @author Matthew Whiting <matthew.whiting@csiro.au>
///
#include <simulationutilities/Spectrum.h>
#include <simulationutilities/Continuum.h>

#include <sourcefitting/Component.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <utility>
#include <string>
#include <stdlib.h>
#include <math.h>

ASKAP_LOGGER(logger, ".continuum");

namespace askap {

    namespace simulations {

        ContinuumS3SEX::Continuum():
                Continuum()
        {
            this->defineSource(0., 0., 1400.);
        }

        ContinuumS3SEX::Continuum(Spectrum &s):
                Continuum(s)
        {
            this->defineSource(0., 0., 1400.);
        }

        ContinuumS3SEX::Continuum(std::string &line)
        {
            /// @details Constructs a Continuum object from a line of
            /// text from an ascii file. Uses the ContinuumS3SEX::define()
            /// function.
	  this->define(line);
	}

        void ContinuumS3SEX::define(std::string &line)
        {
            /// @details Defines a Continuum object from a line of
            /// text from an ascii file. This line should be formatted in
            /// the correct way to match the output from the appropriate
            /// python script. The columns should accepted by this function are:
            /// RA - DEC - Flux - Alpha - Beta - Major axis - Minor axis - Pos.Angle
            /// (Alpha & Beta are the spectral index & spectral curvature).
            /// @param line A line from the ascii input file

            double flux, maj, min, pa;
            std::stringstream ss(line);
	    int component,galaxy,structure;
	    ss >> component >> galaxy >> structure 
	       >> this->itsRA >> this->itsDec >> pa >> maj >> min 
	       >> this->itsI151 >> this->itsI610 >> this->itsI1400 >> this->itsI4860 >> this->itsI18000;
	    flux = pow(10,this->itsI1400);
            this->itsComponent.setPeak(flux);
	    if(maj>=min){
	      this->itsComponent.setMajor(maj);
	      this->itsComponent.setMinor(min);
	    } else{
	      this->itsComponent.setMajor(min);
	      this->itsComponent.setMinor(maj);
	    }
            this->itsComponent.setPA(pa);
	    this->itsAlpha = (this->itsI1400-this->itsI610)/log10(1400./610.);
	    this->itsBeta = 0.;
        }

        ContinuumS3SEX::Continuum(const Continuum& c):
                Spectrum(c)
        {
            operator=(c);
        }

        ContinuumS3SEX& ContinuumS3SEX::operator= (const Continuum& c)
        {
            if (this == &c) return *this;

            ((Continuum &) *this) = c;
            this->itsAlpha      = c.itsAlpha;
            this->itsBeta       = c.itsBeta;
            this->itsNuZero     = c.itsNuZero;
            return *this;
        }

        ContinuumS3SEX& ContinuumS3SEX::operator= (const Spectrum& c)
        {
            if (this == &c) return *this;

            ((Continuum &) *this) = c;
            this->defineSource(0., 0., 1400.);
            return *this;
        }

        double ContinuumS3SEX::flux(double freq)
        {
            /// @details Returns the flux at a given frequency.
            /// @param freq The frequency, in Hz
            /// @return The flux, in Jy
            double powerTerm = this->itsAlpha + this->itsBeta * log(freq / this->itsNuZero);
            return this->fluxZero() * pow(freq / this->itsNuZero, powerTerm);
        }

      double ContinuumS3SEX::flux(double freq1, double freq2)
      {
	
	if(fabs(this->itsBeta)>0) ASKAPLOG_ERROR_STR(logger,"Cannot yet integrate with non-zero curvature.");

	double powerTerm = this->itsAlpha;

	double flux = this->fluxZero() * (pow(std::max(freq1,freq2),powerTerm+1) - pow(std::min(freq1,freq2),powerTerm+1)) /
	  ((powerTerm+1) * pow(this->itsNuZero,powerTerm));
	
	return flux/fabs(freq2-freq1);
      }

        std::ostream& operator<< (std::ostream& theStream, ContinuumS3SEX &cont)
        {
            /// @details Prints a summary of the parameters to the stream
            /// @param theStream The destination stream
            /// @param prof The profile object
            /// @return A reference to the stream

	  theStream << cont.itsRA << " " << cont.itsDec << " " << cont.itsContinuumFlux << " " 
		    << cont.itsComponent.pa() << " " << cont.itsComponent.maj() << " " << cont.itsComponent.min() << " " 
		    << cont.itsPeakOpticalDepth << " " << cont.itsCentreRedshift << " " << cont.itsVelocityWidth << "\n";
            return theStream;
        }
    }


}
