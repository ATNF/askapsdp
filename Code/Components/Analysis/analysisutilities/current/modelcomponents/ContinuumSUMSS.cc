/// @file
///
/// Continuum source from the SUMSS catalogue, version 2.1, as obtained from http://www.physics.usyd.edu.au/sifa/Main/SUMSS
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
#include <askap_analysisutilities.h>

#include <modelcomponents/Spectrum.h>
#include <modelcomponents/Continuum.h>
#include <modelcomponents/ContinuumSUMSS.h>

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

ASKAP_LOGGER(logger, ".ContSUMSS");

namespace askap {

    namespace analysisutilities {

        ContinuumSUMSS::ContinuumSUMSS():
                Continuum()
        {
            this->defineSource(0., 0., 1400.);
        }

        ContinuumSUMSS::ContinuumSUMSS(Spectrum &s):
                Continuum(s)
        {
            this->defineSource(0., 0., 1400.);
        }

        ContinuumSUMSS::ContinuumSUMSS(std::string &line)
        {
            /// @details Constructs a Continuum object from a line of
            /// text from an ascii file. Uses the ContinuumSUMSS::define()
            /// function.
	  this->define(line);
	}

        void ContinuumSUMSS::define(const std::string &line)
        {
            /// @details Defines a Continuum object from a line of
            /// text from an ascii file. This line should be taken
            /// from the CDS output from an SUMSS query, formatted in
            /// ascii text/plain format.
	    ///  @param line A line from the ascii input file

	  this->itsInputLine      = line;
	  std::stringstream ss(line);
	  ss >> this->itsRAh >> this->itsRAm >> this->itsRAs >> this->itsDECd >> this->itsDECm >> this->itsDECs
	     >> this->itsRAerr >> this->itsDECerr 
	     >> this->itsPeakFlux >> this->itsPeakFluxErr
	     >> this->itsTotalFlux >> this->itsTotalFluxErr
	     >> this->itsFittedMajorAxis >> this->itsFittedMinorAxis >> this->itsFittedPositionAngle
	     >> this->itsDeconvMajorAxis >> this->itsDeconvMinorAxis >> this->itsDeconvPositionAngleString
	     >> this->itsMosaicName >> this->itsNumMosaics >> this->itsXpos >> this->itsYpos;
	  
	  this->itsRA = this->itsRAh+":"+this->itsRAm+":"+this->itsRAs;
	  this->itsDec = this->itsDECd+":"+this->itsDECm+":"+this->itsDECs;
	  this->PosToID();

	  this->itsFlux = this->itsTotalFlux / 1.e3;  //convert to Jy
	  this->itsMaj = this->itsDeconvMajorAxis;
	  this->itsMin = this->itsDeconvMinorAxis;
	  this->itsPA = (this->itsDeconvPositionAngleString=="---") ? 0. : this->itsFittedPositionAngle;

	  this->itsAlpha = 0.;
	  this->itsBeta = 0.;

	  this->checkShape();

        }

        ContinuumSUMSS::ContinuumSUMSS(const ContinuumSUMSS& c):
                Continuum(c)
        {
            operator=(c);
        }

        ContinuumSUMSS& ContinuumSUMSS::operator= (const ContinuumSUMSS& c)
        {
            if (this == &c) return *this;

            ((Continuum &) *this) = c;
            this->itsAlpha      = c.itsAlpha;
            this->itsBeta       = c.itsBeta;
            this->itsNuZero     = c.itsNuZero;
            return *this;
        }

        ContinuumSUMSS& ContinuumSUMSS::operator= (const Spectrum& c)
        {
            if (this == &c) return *this;

            ((Continuum &) *this) = c;
            this->defineSource(0., 0., 1400.);
            return *this;
        }


      void ContinuumSUMSS::print(std::ostream& theStream)
      {
	theStream << this->itsInputLine << "\n";
      }

        std::ostream& operator<< (std::ostream& theStream, ContinuumSUMSS &cont)
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
