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
#include <askap_analysisutilities.h>

#include <iostream>
#include <iomanip>

#include <modelcomponents/Spectrum.h>
#include <coordutils/PositionUtilities.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

ASKAP_LOGGER(logger, ".spectrum");


namespace askap {

    namespace analysisutilities {

        Spectrum::Spectrum(std::string &line)
        {
            /// @details Constructs a Spectrum object from a line of
            /// text from an ascii file. Uses the Spectrum::define()
            /// function.
	  this->define(line);
	}

        void Spectrum::define(const std::string &line)
        {
	  /// @details Defines the Spectrum object from a line of
	  /// text from an ascii file. The columns should accepted
	  /// by this function are: RA - DEC - Flux - Major axis -
	  /// Minor axis - Pos.Angle 
	  /// itsID is constructed from the RA & Dec
	  /// @param line A line from the ascii input file

            std::stringstream ss(line);
            ss >> this->itsRA >> this->itsDec >> this->itsFlux >> this->itsMaj >> this->itsMin >> this->itsPA;
	    this->PosToID();
	    this->checkShape();
        }

      void Spectrum::PosToID()
      {
	///@details This creates an ID string by combining the RA & Dec strings, separated by an underscore.
	this->itsID = this->itsRA+"_"+this->itsDec;
      }

      void Spectrum::checkShape()
      {
	if(this->itsMaj < this->itsMin){
	  float t=this->itsMaj;
	  this->itsMaj = this->itsMin;
	  this->itsMin = t;
	}
      }
	

        Spectrum::Spectrum(const Spectrum& s)
        {
	  this->itsID = s.itsID;
	  this->itsRA = s.itsRA;
	  this->itsDec = s.itsDec;
	  this->itsFlux = s.itsFlux;
	  this->itsMaj = s.itsMaj;
	  this->itsMin = s.itsMin;
	  this->itsPA = s.itsPA;
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

      double Spectrum::raD()
      {
	return raToDouble(this->itsRA);
      }

      double Spectrum::decD()
      {
	return decToDouble(this->itsDec);
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
	theStream << this->itsRA << "\t" << this->itsDec << "\t" << this->itsFlux << "\t" 
		  << this->itsMaj << "\t" << this->itsMin << "\t" << this->itsPA << "\n"; 
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
