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
#include <askap_analysisutilities.h>

#include <modelcomponents/ContinuumID.h>
#include <modelcomponents/Spectrum.h>
#include <modelcomponents/Continuum.h>

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

    namespace analysisutilities {

        ContinuumID::ContinuumID():
                Continuum()
        {
        }

        ContinuumID::ContinuumID(Spectrum &s):
	  Continuum(s)
        {
        }

        ContinuumID::ContinuumID(std::string &line)
        {
            /// @details Constructs a ContinuumID object from a line of
            /// text from an ascii file. Uses the ContinuumID::define()
            /// function.
	  this->define(line);
	}

        void ContinuumID::define(const std::string &line)
        {
            /// @details Defines a ContinuumID object from a line of
            /// text from an ascii file. This line should be formatted in
            /// the following way, explicitly setting the ID:
            /// ID - RA - DEC - Flux - Alpha - Beta - Major axis - Minor axis - Pos.Angle
            /// (Alpha & Beta are the spectral index & spectral curvature). *** The Flux provided in the text file is no longer assumed to be in log space.***
            /// @param line A line from the ascii input file

            std::stringstream ss(line);
	    ss >> this->itsID >> this->itsRA >> this->itsDec >> this->itsFlux >> this->itsAlpha >> this->itsBeta >> this->itsMaj >> this->itsMin >> this->itsPA;
	    this->checkShape();

        }

        ContinuumID::ContinuumID(const ContinuumID& c):
                Continuum(c)
        {
            operator=(c);
        }

        ContinuumID& ContinuumID::operator= (const ContinuumID& c)
        {
            if (this == &c) return *this;
            ((Continuum &) *this) = c;
            return *this;
        }

        ContinuumID& ContinuumID::operator= (const Continuum& c)
        {
            if (this == &c) return *this;
            ((Continuum &) *this) = c;
            return *this;
        }

        ContinuumID& ContinuumID::operator= (const Spectrum& c)
        {
            if (this == &c) return *this;
            ((Spectrum &) *this) = c;
            this->defineSource(0., 0., 1400.);
            return *this;
        }

      void ContinuumID::print(std::ostream& theStream)
      {
	theStream << this->itsID << "\t" << this->itsRA << "\t" << this->itsDec << "\t" 
		  << this->itsFlux << "\t" << this->itsAlpha << "\t" << this->itsBeta << "\t" 
		  << this->itsMaj << "\t" << this->itsMin << "\t" << this->itsPA << "\n";
      }

        std::ostream& operator<< (std::ostream& theStream, ContinuumID &cont)
	{
	  cont.print(theStream);
	  return theStream;
	}

    }


}
