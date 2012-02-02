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
#include <askap_simulations.h>

#include <simulationutilities/Spectrum.h>
#include <simulationutilities/Continuum.h>
#include <simulationutilities/ContinuumS3SEX.h>
#include <simulationutilities/FullStokesContinuum.h>

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
	  ContinuumS3SEX()
        {
            this->defineSource(0., 0., 1400.e6);
        }

        FullStokesContinuum::FullStokesContinuum(ContinuumS3SEX &c):
	  ContinuumS3SEX(c)
        {
            this->defineSource(0., 0., 1400.e6);
        }

        FullStokesContinuum::FullStokesContinuum(Continuum &c):
	  ContinuumS3SEX(c)
        {
            this->defineSource(0., 0., 1400.e6);
        }

        FullStokesContinuum::FullStokesContinuum(Spectrum &s):
	  ContinuumS3SEX(s)
        {
            this->defineSource(0., 0., 1400.e6);
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

	  float flux1400;
            std::stringstream ss(line);
            ss >> this->itsComponentNum >> this->itsClusterID >> this->itsGalaxyNum 
	       >> this->itsSFtype >> this->itsAGNtype >> this->itsStructure 
	       >> this->itsRA >> this->itsDec >> this->itsDistance >> this->itsRedshift 
	       >> this->itsPA >> this->itsMaj >> this->itsMin 
	       >> this->itsI151 >> this->itsI610 >> flux1400 
	       >> this->itsStokesQref >> this->itsStokesUref >> this->itsPolFluxRef >> this->itsPolFracRef 
	       >> this->itsI4860 >> this->itsI18000 >> this->itsCosVA >> this->itsRM >> this->itsRMflag;
	    
	    this->itsI1400 = log10(flux1400);
	    // ASKAPLOG_DEBUG_STR(logger, "Full Stokes S3SEX object, with flux1400="<<flux1400<<" and itsI1400="<<this->itsI1400);
	    this->checkShape();

	    this->itsStokesRefFreq = POLREFFREQ;
	    this->itsStokesVref = 0.;     // Setting Stokes V to be zero for now!
	    if(this->itsPolFluxRef>0.)
	      this->itsPolAngleRef = acos(this->itsStokesQref/this->itsPolFluxRef);
	    else 
	      this->itsPolAngleRef = 0.;
// 	    this->itsAlpha = (log10(this->itsFlux)-this->itsI610)/log10(1400./610.);
        }

      void FullStokesContinuum::print(std::ostream &theStream)
      {
	theStream.setf(std::ios::showpoint);
	theStream << this->itsComponentNum << std::setw(7)<<this->itsClusterID << std::setw(11)<<this->itsGalaxyNum
		  << std::setw(3)<<this->itsSFtype << std::setw(3)<<this->itsAGNtype << std::setw(3)<<this->itsStructure;
	theStream << std::setw(12)<<this->itsRA << std::setw(12)<<this->itsDec;
	theStream.setf(std::ios::fixed); theStream.unsetf(std::ios::scientific);
	theStream << std::setprecision(3)<<std::setw(11)<<this->itsDistance << std::setprecision(6)<<std::setw(11)<<this->itsRedshift;
	theStream.precision(3);
	theStream << std::setw(10)<<this->itsPA << std::setw(10)<<this->itsMaj << std::setw(10)<<this->itsMin;
	theStream.precision(4);
	theStream << std::setw(10)<<this->itsI151 << std::setw(10)<<this->itsI610;
	theStream.setf(std::ios::scientific); theStream.unsetf(std::ios::fixed); 
	theStream << std::setw(12)<<this->itsFlux << std::setw(12)<<this->itsStokesQref << std::setw(12)<<this->itsStokesUref << std::setw(12)<<this->itsPolFluxRef;
	theStream.setf(std::ios::fixed); theStream.unsetf(std::ios::scientific);
	theStream << std::setw(10)<<this->itsPolFracRef << std::setw(10)<<this->itsI4860 << std::setw(10)<<this->itsI18000 << std::setw(10)<<this->itsCosVA 
		  << std::setw(11)<<this->itsRM << std::setw(11)<<this->itsRMflag;
	theStream << "\n";
      }

      std::ostream& operator<<(std::ostream &theStream, FullStokesContinuum &stokes)
      {
	stokes.print(theStream);
	return theStream;
      }


        FullStokesContinuum::FullStokesContinuum(const FullStokesContinuum& c):
                ContinuumS3SEX(c)
        {
            operator=(c);
        }

        FullStokesContinuum& FullStokesContinuum::operator= (const FullStokesContinuum& c)
        {
            if (this == &c) return *this;

            ((ContinuumS3SEX &) *this) = c;
	    this->itsClusterID = c.itsClusterID;
	    this->itsSFtype = c.itsSFtype;
	    this->itsAGNtype = c.itsAGNtype;
	    this->itsDistance = c.itsDistance;
	    this->itsRedshift = c.itsRedshift;
	    this->itsCosVA = c.itsCosVA;
	    this->itsStokesRefFreq = c.itsStokesRefFreq;
	    this->itsStokesQref = c.itsStokesQref;
	    this->itsStokesUref = c.itsStokesUref;
	    this->itsStokesVref = c.itsStokesVref;
	    this->itsPolFluxRef = c.itsPolFluxRef;
	    this->itsPolFracRef = c.itsPolFracRef;
	    this->itsPolAngleRef = c.itsPolAngleRef;
	    this->itsRM = c.itsRM;
	    this->itsRMflag = c.itsRMflag;
            return *this;
        }

        FullStokesContinuum& FullStokesContinuum::operator= (const Continuum& c)
        {
            if (this == &c) return *this;

            ((Continuum &) *this) = c;
            this->defineSource(0., 0., 1400.e6);
            return *this;
        }

        FullStokesContinuum& FullStokesContinuum::operator= (const ContinuumS3SEX& c)
        {
            if (this == &c) return *this;

            ((ContinuumS3SEX &) *this) = c;
            this->defineSource(0., 0., 1400.e6);
            return *this;
        }

        FullStokesContinuum& FullStokesContinuum::operator= (const Spectrum& c)
        {
            if (this == &c) return *this;

            ((Spectrum &) *this) = c;
            this->defineSource(0., 0., 1400.e6);
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
	  flux = polFlux * cos( 2. * (this->itsPolAngleRef + angle) );
	  break;
	case 2: // Stokes U
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
