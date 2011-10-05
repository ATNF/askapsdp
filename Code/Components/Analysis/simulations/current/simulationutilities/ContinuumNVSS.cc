/// @file
///
/// Continuum source from the NVSS catalogue, using the full content as obtained from CDS, with ascii text/plain option
///
/// @copyright (c) 2010 CSIRO
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
#include <simulationutilities/ContinuumNVSS.h>

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

ASKAP_LOGGER(logger, ".ContNVSS");

namespace askap {

    namespace simulations {

        ContinuumNVSS::ContinuumNVSS():
                Continuum()
        {
            this->defineSource(0., 0., 1400.);
        }

        ContinuumNVSS::ContinuumNVSS(Spectrum &s):
                Continuum(s)
        {
            this->defineSource(0., 0., 1400.);
        }

        ContinuumNVSS::ContinuumNVSS(std::string &line)
        {
            /// @details Constructs a Continuum object from a line of
            /// text from an ascii file. Uses the ContinuumNVSS::define()
            /// function.
	  this->define(line);
	}

        void ContinuumNVSS::define(std::string &line)
        {
            /// @details Defines a Continuum object from a line of
            /// text from an ascii file. This line should be taken
            /// from the CDS output from an NVSS query, formatted in
            /// ascii text/plain format.
	    ///  @param line A line from the ascii input file

	  this->itsInputLine      = line;
	  this->itsRadius         = atof(line.substr(0,9).c_str());
	  this->itsXoff           = atof(line.substr(10,10).c_str());
	  this->itsYoff           = atof(line.substr(21,10).c_str());
	  this->itsRecno          = atoi(line.substr(32,8).c_str());
	  this->itsField          = line.substr(41,8);
	  this->itsFieldXpos      = atof(line.substr(50,7).c_str());
	  this->itsFieldYpos      = atof(line.substr(58,7).c_str());
	  this->itsName           = line.substr(66,14);
	  this->itsRAstring       = line.substr(81,11);
	  this->itsDecstring      = line.substr(93,11);
	  this->itsRA_err         = atof(line.substr(105,5).c_str());
	  this->itsDec_err        = atof(line.substr(111,4).c_str());
	  this->itsS1400          = atof(line.substr(116,8).c_str());
	  this->itsS1400_err      = atof(line.substr(125,7).c_str());
	  this->itsMajorAxisLimit = line[133];
	  this->itsMajorAxis      = atof(line.substr(135,5).c_str());
	  this->itsMinorAxisLimit = line[141];
	  this->itsMinorAxis      = atof(line.substr(143,5).c_str());
	  this->itsPA             = atof(line.substr(149,5).c_str());
	  this->itsMajorAxis_err  = atof(line.substr(155,4).c_str());
	  this->itsMinorAxis_err  = atof(line.substr(160,4).c_str());
	  this->itsPA_err         = atof(line.substr(165,4).c_str());
	  this->itsFlagResidual   = line.substr(170,2);
	  this->itsResidualFlux   = atoi(line.substr(173,4).c_str());
	  this->itsPolFlux        = atof(line.substr(178,6).c_str());
	  this->itsPolPA          = atof(line.substr(185,5).c_str());
	  this->itsPolFlux_err    = atof(line.substr(191,5).c_str());
	  this->itsPolPA_err      = atof(line.substr(197,4).c_str());
	  
	  this->itsRA = this->itsRAstring;
	  for(size_t i=0;i<this->itsRA.size();i++) if(this->itsRA[i]==' ') this->itsRA[i]=':';
	  this->itsDec = this->itsDecstring;
	  for(size_t i=0;i<this->itsDec.size();i++) if(this->itsDec[i]==' ') this->itsDec[i]=':';

	  this->itsComponent.setPeak(this->itsS1400/1.e3); // put into Jy
	  this->itsComponent.setMajor( this->itsMajorAxisLimit=='<' ? 0. : this->itsMajorAxis );
	  this->itsComponent.setMinor( this->itsMinorAxisLimit=='<' ? 0. : this->itsMinorAxis );
	  if(this->itsComponent.maj()<this->itsComponent.min()){
	    this->itsComponent.setMajor(this->itsComponent.min());
	    this->itsComponent.setMinor(this->itsComponent.maj());
	  }
	  this->itsComponent.setPA(this->itsPA);
	  
        }

        ContinuumNVSS::ContinuumNVSS(const ContinuumNVSS& c):
                Continuum(c)
        {
            operator=(c);
        }

        ContinuumNVSS& ContinuumNVSS::operator= (const ContinuumNVSS& c)
        {
            if (this == &c) return *this;

            ((Continuum &) *this) = c;
            this->itsAlpha      = c.itsAlpha;
            this->itsBeta       = c.itsBeta;
            this->itsNuZero     = c.itsNuZero;
            return *this;
        }

        ContinuumNVSS& ContinuumNVSS::operator= (const Spectrum& c)
        {
            if (this == &c) return *this;

            ((Continuum &) *this) = c;
            this->defineSource(0., 0., 1400.);
            return *this;
        }


      void ContinuumNVSS::print(std::ostream& theStream)
      {
	theStream << this->itsInputLine << "\n";
	// theStream.setf(std::ios::showpoint);
	// theStream << std::setw(11) << this->itsComponentNum << " " 
	// 	  << std::setw(9) << this->itsGalaxyNum << " " 
	// 	  << std::setw(9)  << this->itsStructure << " "
	// 	  << std::setw(15) << std::setprecision(5) << this->itsRA << " " 
	// 	  << std::setw(11) << std::setprecision(5) << this->itsDec << " "
	// 	  << std::setw(14) << std::setprecision(3) << this->itsComponent.pa() << " " 
	// 	  << std::setw(10) << std::setprecision(3) << this->itsComponent.maj() << " " 
	// 	  << std::setw(10) << std::setprecision(3) << this->itsComponent.min() << " " 
	// 	  << std::setw(7) << std::setprecision(4) << this->itsI151 << " " 
	// 	  << std::setw(7) << std::setprecision(4) << this->itsI610 << " " 
	// 	  << std::setw(7) << std::setprecision(4) << this->itsI1400 << " " 
	// 	  << std::setw(7) << std::setprecision(4) << this->itsI4860 << " " 
	// 	  << std::setw(7) << std::setprecision(4) << this->itsI18000 << "\n";
      }
        std::ostream& operator<< (std::ostream& theStream, ContinuumNVSS &cont)
        {
            /// @details Prints a summary of the parameters to the stream
            /// @param theStream The destination stream
            /// @param prof The profile object
            /// @return A reference to the stream

	  cont.print(theStream);
	  return theStream;
        }

      void ContinuumNVSS::printDetails(std::ostream& theStream)
      {
	theStream << "radius = " << this->itsRadius
		  << "\nXoff = " << this->itsXoff
		  << "\nYoff = " << this->itsYoff
		  << "\nRecno = " << this->itsRecno 
		  << "\nField = " << this->itsField
		  << "\nXpos = " << this->itsFieldXpos
		  << "\nYpos = " << this->itsFieldYpos
		  << "\nName = " << this->itsName
		  << "\nRA = " << this->itsRAstring << " +- " << this->itsRA_err
		  << "\nDec = " << this->itsDecstring << " +- " << this->itsDec_err
		  << "\nFlux = " << this->itsS1400 << " +- " << this->itsS1400_err
		  << "\nMajor axis = " << this->itsMajorAxisLimit << " " << this->itsMajorAxis << " +- " << this->itsMajorAxis_err
		  << "\nMinor axis = " << this->itsMinorAxisLimit << " " << this->itsMinorAxis << " +- " << this->itsMinorAxis_err
		  << "\nPA = " << this->itsPA << " +- " << this->itsPA_err
		  << "\nResidual = " << this->itsFlagResidual << " " << this->itsResidualFlux
		  << "\nPol flux = " << this->itsPolFlux << " +- " << this->itsPolFlux_err
		  << "\nPol PA = " << this->itsPolPA << " +- " << this->itsPolPA_err
		  << "\n"
		  << "\nRA = " << this->itsRA
		  << "\nDec = " << this->itsDec
		  << "\nComponent = " << this->itsComponent
		  << "\n";
      }
      
    }


}
