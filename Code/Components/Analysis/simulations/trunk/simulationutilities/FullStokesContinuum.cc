/// @file
///
/// XXX Notes on program XXX
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
/// @author XXX XXX <XXX.XXX@csiro.au>
///
#include <simulationutilities/Spectrum.h>
#include <simulationutilities/Continuum.h>
#include <simulationutilities/FullStokesContinuum.h>

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

ASKAP_LOGGER(logger, ".fullstokescontinuum");

namespace askap {

    namespace simulations {

        FullStokesContinuum::FullStokesContinuum():
                Continuum()
        {
            this->defineSource(0., 0., 1400.);
        }

        FullStokesContinuum::FullStokesContinuum(Continuum &c):
                Continuum(c)
        {
            this->defineSource(0., 0., 1400.);
        }

        FullStokesContinuum::FullStokesContinuum(Spectrum &s):
	  Continuum(s)
        {
            this->defineSource(0., 0., 1400.);
        }

        FullStokesContinuum::FullStokesContinuum(std::string &line)
        {
            /// @details Constructs a FullStokesContinuum object from a line of
            /// text from an ascii file. Uses the FullStokesContinuum::define()
            /// function.
	  this->define(line);
	}

        void FullStokesContinuum::define(std::string &line)
        {
            /// @details Defines a FullStokesContinuum object from a line of
            /// text from an ascii file. The format of the line is currently taken from the POSSUM catalogue supplied by Jeroen Stil.
            /// @param line A line from the ascii input file

	    int sourceID,clusterID,GalaxyID,SFtype,AGNtype,structure;
	    std::string ra,dec;
            double distance,redshift,pa,maj,min,i151L,i610L,i1420,stokesQ,stokesU,polFlux,fracPol,i4p8L,i18L,cosva,rm,rmflag;
            std::stringstream ss(line);
            ss >> sourceID >> clusterID >> GalaxyID >> SFtype >> AGNtype >> structure >> 
	      ra >> dec >> distance >> redshift >> pa >> maj >> min >> 
	      i151L >> i610L >> i1420 >> stokesQ >> stokesU >> polFlux >> fracPol >> i4p8L >> i18L >> 
	      cosva >> rm >> rmflag;
	    this->itsRA = ra;
	    this->itsDec = dec;
            this->itsComponent.setPeak(i1420);
	    if(maj>=min){
	      this->itsComponent.setMajor(maj);
	      this->itsComponent.setMinor(min);
	    } else{
	      this->itsComponent.setMajor(min);
	      this->itsComponent.setMinor(maj);
	    }
            this->itsComponent.setPA(pa);
	    this->itsStokesRefFreq = 1.4e9;
	    this->itsStokesQref = stokesQ;
	    this->itsStokesUref = stokesU;
	    this->itsStokesVref = 0.;     // Setting Stokes V to be zero for now!
	    this->itsPolFracRef = fracPol;
	    if(polFlux>0.)
	      this->itsPolAngleRef = acos(stokesQ/polFlux);
	    else 
	      this->itsPolAngleRef = 0.;
	    this->itsRM = rm;
	    this->itsAlpha = (log10(i1420)-i610L)/log10(1420./610.);
        }

        FullStokesContinuum::FullStokesContinuum(const FullStokesContinuum& c):
                Continuum(c)
        {
            operator=(c);
        }

        FullStokesContinuum& FullStokesContinuum::operator= (const FullStokesContinuum& c)
        {
            if (this == &c) return *this;

            ((Spectrum &) *this) = c;
	    this->itsStokesRefFreq = c.itsStokesRefFreq;
	    this->itsStokesQref = c.itsStokesQref;
	    this->itsStokesUref = c.itsStokesUref;
	    this->itsStokesVref = c.itsStokesVref;
	    this->itsPolFracRef = c.itsPolFracRef;
	    this->itsPolAngleRef = c.itsPolAngleRef;
	    this->itsRM = c.itsRM;
            return *this;
        }

        FullStokesContinuum& FullStokesContinuum::operator= (const Continuum& c)
        {
            if (this == &c) return *this;

            ((Continuum &) *this) = c;
            this->defineSource(0., 0., 1400.);
            return *this;
        }

        FullStokesContinuum& FullStokesContinuum::operator= (const Spectrum& c)
        {
            if (this == &c) return *this;

            ((Spectrum &) *this) = c;
            this->defineSource(0., 0., 1400.);
            return *this;
        }

      double FullStokesContinuum::flux(int istokes, double freq)
      {

	double lambda2,lambdaRef2,angle=0.,flux;
	if(istokes>0){
	  lambda2 = C*C/(freq*freq);
	  lambdaRef2 = C*C/(this->itsStokesRefFreq*this->itsStokesRefFreq);
	  angle = (lambda2 - lambdaRef2)*this->itsRM;
	}

	double stokesIFlux =  this->Continuum::flux(freq);
	double polFlux = stokesIFlux * this->itsPolFracRef; // Assume constant fractional polarisation

	switch(istokes){
	case 0: // Stokes I
	  flux = stokesIFlux;
	  break;
	case 1: // Stokes Q
// 	  flux =  this->itsStokesQref * cos(2.*angle) - this->itsStokesUref * sin(2.*angle);
	  flux = polFlux * cos( 2. * (this->itsPolAngleRef + angle) );
	  break;
	case 2: // Stokes U
// 	  flux =  this->itsStokesUref * cos(2.*angle) + this->itsStokesQref * sin(2.*angle);
	  flux = polFlux * sin( 2. * (this->itsPolAngleRef + angle) );
	  break;
	case 3: // Stokes V
	  flux = 0.;   // Setting stokes V to zero!
	  break;
	default:
	  ASKAPLOG_ERROR_STR(logger, "The istokes parameter provided ("<<istokes<<") needs to be in [0,3]");
	  flux =0.;
	  break;
	}
	return flux;

      }


    }

}
