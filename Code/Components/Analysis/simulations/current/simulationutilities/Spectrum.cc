/// @file
///
/// Base class functions for spectral profiles.
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
#include <askap_simulations.h>

#include <simulationutilities/Spectrum.h>
#include <sourcefitting/Component.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

ASKAP_LOGGER(logger, ".spectrum");


namespace askap {

    namespace simulations {

        Spectrum::Spectrum(std::string &line)
        {
            /// @details Constructs a Spectrum object from a line of
            /// text from an ascii file. Uses the Spectrum::define()
            /// function.
	  this->define(line);
	}

        void Spectrum::define(std::string &line)
        {
            /// @details Defines the Spectrum object from a line of
            /// text from an ascii file. This line should be formatted in
            /// the correct way to match the output from the appropriate
            /// python script. The columns should accepted by this function are:
            /// RA - DEC - Flux - Major axis - Minor axis - Pos.Angle
            /// @param line A line from the ascii input file

            double flux, maj, min, pa;
            std::stringstream ss(line);
            ss >> this->itsRA >> this->itsDec >> flux >> maj >> min >> pa;
            this->itsComponent.setPeak(flux);
	    if(maj>=min){
	      this->itsComponent.setMajor(maj);
	      this->itsComponent.setMinor(min);
	    } else{
	      this->itsComponent.setMajor(min);
	      this->itsComponent.setMinor(maj);
	    }
            this->itsComponent.setPA(pa);
        }

        Spectrum::Spectrum(const Spectrum& s)
        {
            this->itsRA = s.itsRA;
            this->itsDec = s.itsDec;
            this->itsComponent = s.itsComponent;
        }

      void Spectrum::setRA(double r, int prec)
      {
	std::stringstream ss;
	ss << std::fixed << std::setprecision(prec) << r;
	this->itsRA = ss.str();
      }

      void Spectrum::setDec(double d, int prec)
      {
	std::stringstream ss;
	ss << std::fixed << std::setprecision(prec) << d;
	this->itsDec = ss.str();
      }


      void Spectrum::print(std::ostream& theStream, std::string ra, std::string dec)
      {
	std::string oldRA=this->itsRA;
	std::string oldDec=this->itsDec;
	this->itsRA = ra;
	this->itsDec = dec;
	this->print(theStream);
	this->itsRA = oldRA;
	this->itsDec = oldDec;
      }

      void Spectrum::print(std::ostream& theStream, double ra, double dec, int prec)
      {
	std::string oldRA=this->itsRA;
	std::string oldDec=this->itsDec;
	this->setRA(ra);
	this->setDec(dec);
	this->print(theStream);
	this->itsRA = oldRA;
	this->itsDec = oldDec;
      }
      void Spectrum::print(std::ostream& theStream)
      {
	theStream << this->itsRA << "\t" << this->itsDec << "\t" << this->itsComponent.peak() << "\t" 
		  << this->itsComponent.maj() << "\t" << this->itsComponent.min() << "\t" << this->itsComponent.pa() << "\n"; 
      }

        std::ostream& operator<< (std::ostream& theStream, Spectrum &spec)
        {
            /// @details Prints a summary of the parameters to the stream
            /// @param theStream The destination stream
            /// @param prof The profile object
            /// @return A reference to the stream

	  spec.print(theStream);
	  return theStream;
        }



    }

}
