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
#include <askap_analysisutilities.h>

#include <modelcomponents/FullStokesContinuumHI.h>
#include <modelcomponents/Spectrum.h>
#include <modelcomponents/Continuum.h>
#include <modelcomponents/ContinuumS3SEX.h>
#include <modelcomponents/HIprofileS3SEX.h>
#include <modelcomponents/FullStokesContinuum.h>

#include <mathsutils/MathsUtils.h>
#include <cosmology/Cosmology.h>

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

ASKAP_LOGGER(logger, ".fullstokescontinuumHI");

namespace askap {

    namespace analysisutilities {

      GALTYPE getGaltype(int sftype, int agntype)
      {
	GALTYPE type=UNKNOWN;
	switch(sftype)
	  {
	  case 0:
	    switch(agntype)
	      {
	      case 0:
		ASKAPLOG_ERROR_STR(logger, "Both sftype and agntype = 0.");
		break;
	      case 1:
		type = RQAGN; break;
	      case 2:
		type = FRI; break;
	      case 3:
		type = FRII; break;
	      case 4:
		type = GPS; break;
	      default:
		ASKAPLOG_ERROR_STR(logger, "Unknown value " << agntype << " for agntype");
	      };
	    break;
	  case 1:
	    type = SFG; break;
	  case 2:
	    type = SBG; break;
	  default:
	    break;
	  }

	return type;

      }

        FullStokesContinuumHI::FullStokesContinuumHI():
	  FullStokesContinuum(),itsHIprofile()
        {
        }

        FullStokesContinuumHI::FullStokesContinuumHI(ContinuumS3SEX &c):
	  FullStokesContinuum(c),itsHIprofile()
        {
        }

        FullStokesContinuumHI::FullStokesContinuumHI(Continuum &c):
	  FullStokesContinuum(c),itsHIprofile()
        {
        }

        FullStokesContinuumHI::FullStokesContinuumHI(Spectrum &s):
	  FullStokesContinuum(s),itsHIprofile()
        {
        }

        FullStokesContinuumHI::FullStokesContinuumHI(std::string &line)
        {
            /// @details Constructs a FullStokesContinuumHI object from a line of
            /// text from an ascii file. Uses the FullStokesContinuumHI::define()
            /// function.
	  this->define(line);
	}

        void FullStokesContinuumHI::define(const std::string &line)
        {
            /// @details Defines a FullStokesContinuumHI object from a
            /// line of text from an ascii file. The line is
            /// interpreted by FullStokesContinuum::define, and then
            /// the HI mass of the object is calculated. We use the
            /// expression from Wilman et al (2008): 
	    /// log M_HI = 0.44 log L_1.4 + 0.48 +- delta
	    /// where delta is drawn from a normal distribution with
	    /// sigma=0.3. This means the HI mass is randomly created
	    /// each time - if you want to record what was used you
	    /// need to write it out at time of execution. Note that the luminosity is in units of W/Hz, so we need to correct the value from Jy.
 	    ///  @param line A line from the ascii input file

	  this->FullStokesContinuum::define(line);

	  double HImass=0.;
	  GALTYPE type = getGaltype(this->itsSFtype, this->itsAGNtype);
	  if(type == SFG || type == SBG){
	    cosmology::Cosmology cosmo;
	    double lum = cosmo.lum(this->itsRedshift, this->itsI1400-26.);
	    ASKAPLOG_DEBUG_STR(logger, "Lum of object = " << lum);
	    lum *= M_LN10; // convert to natural log from log_10
	    HImass = 0.44 * lum  + 0.48 + normalRandomVariable(0.,0.3);
	    HImass = exp(HImass);
	    ASKAPLOG_DEBUG_STR(logger, "Creating HI profile with M_HI = " << HImass<<", using log10(flux)="<<this->itsI1400<<" to get a lum of " << lum);
	  }
	  
	  this->itsHIprofile = HIprofileS3SEX(type, this->itsRedshift, HImass, this->itsMaj, this->itsMin);

      }

      void FullStokesContinuumHI::print(std::ostream &theStream)
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
	theStream << std::setw(10) << std::setprecision(6) << log10(this->itsHIprofile.mHI());
	theStream << "\n";
      }

      std::ostream& operator<<(std::ostream &theStream, FullStokesContinuumHI &stokes)
      {
	stokes.print(theStream);
	return theStream;
      }


        FullStokesContinuumHI::FullStokesContinuumHI(const FullStokesContinuumHI& c):
	  FullStokesContinuum(c)
        {
            operator=(c);
        }

        FullStokesContinuumHI& FullStokesContinuumHI::operator= (const FullStokesContinuumHI& c)
        {
            if (this == &c) return *this;

            ((FullStokesContinuum &) *this) = c;
	    this->itsHIprofile = c.itsHIprofile;
            return *this;
        }

        FullStokesContinuumHI& FullStokesContinuumHI::operator= (const Continuum& c)
        {
            if (this == &c) return *this;

            ((Continuum &) *this) = c;
            this->defineSource(0., 0., POLREFFREQ);
            return *this;
        }

        FullStokesContinuumHI& FullStokesContinuumHI::operator= (const ContinuumS3SEX& c)
        {
            if (this == &c) return *this;

            ((ContinuumS3SEX &) *this) = c;
            this->defineSource(0., 0., POLREFFREQ);
            return *this;
        }

        FullStokesContinuumHI& FullStokesContinuumHI::operator= (const Spectrum& c)
        {
            if (this == &c) return *this;

            ((Spectrum &) *this) = c;
            this->defineSource(0., 0., POLREFFREQ);
            return *this;
        }

    }

}
