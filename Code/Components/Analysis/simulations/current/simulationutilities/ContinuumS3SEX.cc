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
#include <simulationutilities/ContinuumS3SEX.h>

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

        ContinuumS3SEX::ContinuumS3SEX():
                Continuum()
        {
            this->defineSource(0., 0., 1400.);
        }

        ContinuumS3SEX::ContinuumS3SEX(Spectrum &s):
                Continuum(s)
        {
            this->defineSource(0., 0., 1400.);
        }

        ContinuumS3SEX::ContinuumS3SEX(std::string &line)
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
	    ss >> this->itsComponentNum >> this->itsGalaxyNum >> this->itsStructure 
	       >> this->itsRA >> this->itsDec >> pa >> maj >> min 
	       >> this->itsI151 >> this->itsI610 >> this->itsI1400 >> this->itsI4860 >> this->itsI18000;
	    if(maj>=min){
	      this->itsComponent.setMajor(maj);
	      this->itsComponent.setMinor(min);
	    } else{
	      this->itsComponent.setMajor(min);
	      this->itsComponent.setMinor(maj);
	    }
            this->itsComponent.setPA(pa);
	    if(this->itsNuZero<610.e6){
	      this->itsAlpha = (this->itsI610-this->itsI151)/log10(610./151.);
	      flux = this->itsI151 + this->itsAlpha * log10(this->itsNuZero/151.e6);
	    }
	    else if(this->itsNuZero < 1400.e6){
	      this->itsAlpha = (this->itsI1400-this->itsI610)/log10(1400./610.);
	      flux = this->itsI610 + this->itsAlpha * log10(this->itsNuZero/610.e6);
	    }
	    else if(this->itsNuZero < 4.86e9){
	      this->itsAlpha = (this->itsI4860-this->itsI1400)/log10(4860./1400.);
	      flux = this->itsI1400 + this->itsAlpha * log10(this->itsNuZero/1400.e6);
	    }
	    else{
	      this->itsAlpha = (this->itsI18000-this->itsI4860)/log10(18000./4860.);
	      flux = this->itsI4860 + this->itsAlpha * log10(this->itsNuZero/4860.e6);
	    }
		//	    this->itsAlpha = (this->itsI1400-this->itsI610)/log10(1400./610.);
		//	    flux = pow(10,this->itsI1400);
		//            this->itsComponent.setPeak(flux);
	    this->itsComponent.setPeak(pow(10,flux));
	    this->itsBeta = 0.;
        }

        ContinuumS3SEX::ContinuumS3SEX(const ContinuumS3SEX& c):
                Continuum(c)
        {
            operator=(c);
        }

        ContinuumS3SEX& ContinuumS3SEX::operator= (const ContinuumS3SEX& c)
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


      void ContinuumS3SEX::print(std::ostream& theStream)
      {
	theStream.setf(std::ios::showpoint);
	theStream << std::setw(11) << this->itsComponentNum << " " 
		  << std::setw(9) << this->itsGalaxyNum << " " 
		  << std::setw(9)  << this->itsStructure << " "
		  << std::setw(15) << std::setprecision(5) << this->itsRA << " " 
		  << std::setw(11) << std::setprecision(5) << this->itsDec << " "
		  << std::setw(14) << std::setprecision(3) << this->itsComponent.pa() << " " 
		  << std::setw(10) << std::setprecision(3) << this->itsComponent.maj() << " " 
		  << std::setw(10) << std::setprecision(3) << this->itsComponent.min() << " " 
		  << std::setw(7) << std::setprecision(4) << this->itsI151 << " " 
		  << std::setw(7) << std::setprecision(4) << this->itsI610 << " " 
		  << std::setw(7) << std::setprecision(4) << this->itsI1400 << " " 
		  << std::setw(7) << std::setprecision(4) << this->itsI4860 << " " 
		  << std::setw(7) << std::setprecision(4) << this->itsI18000 << "\n";
      }
        std::ostream& operator<< (std::ostream& theStream, ContinuumS3SEX &cont)
        {
            /// @details Prints a summary of the parameters to the stream
            /// @param theStream The destination stream
            /// @param prof The profile object
            /// @return A reference to the stream

	  cont.print(theStream);
	  return theStream;
        }
    }


}
