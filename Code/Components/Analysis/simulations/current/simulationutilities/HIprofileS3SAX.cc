/// @file
///
/// Class to manage HI profiles for the SKADS S3-SAX simulation.
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

#include <simulationutilities/SpectralUtilities.h>
#include <simulationutilities/HIprofile.h>
#include <simulationutilities/HIprofileS3SAX.h>
#include <simulationutilities/HIprofileS3SAX.h>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <math.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

ASKAP_LOGGER(logger, ".hiprofiles3sax");


namespace askap {

    namespace simulations {

        HIprofileS3SAX::HIprofileS3SAX(const HIprofileS3SAX& h):
                HIprofile(h)
        {
            operator=(h);
        }

        HIprofileS3SAX& HIprofileS3SAX::operator= (const HIprofileS3SAX& h)
        {
            if (this == &h) return *this;

            ((HIprofile &) *this) = h;
            this->itsFluxPeak = h.itsFluxPeak;
            this->itsFlux0 = h.itsFlux0;
            this->itsWidthPeak = h.itsWidthPeak;
            this->itsWidth50 = h.itsWidth50;
            this->itsWidth20 = h.itsWidth20;
            this->itsIntFlux = h.itsIntFlux;
            this->itsSideFlux = h.itsSideFlux;
            this->itsMiddleFlux = h.itsMiddleFlux;
            this->itsKpar = h.itsKpar;
            return *this;
        }

      void HIprofileS3SAX::diagnostic(std::ostream& theStream)
      {
	theStream << "HI profile summary:\n";
	theStream << "z=" << this->itsRedshift << "\n";
	theStream << "M_HI=" << this->itsMHI << "\n";
	theStream << "Fpeak=" << this->itsFluxPeak << "\n";
	theStream << "F0=" << this->itsFlux0 << "\n";
	theStream << "Wpeak=" << this->itsWidthPeak << "\n";
	theStream << "W50=" << this->itsWidth50 << "\n";
	theStream << "W20=" << this->itsWidth20 << "\n";
	theStream << "IntFlux=" << this->itsIntFlux << "\n";
	theStream << "Side Flux=" << this->itsSideFlux << "\n";
	theStream << "Middle Flux=" << this->itsMiddleFlux << "\n";
	theStream << "K[] = [" << this->itsKpar[0];

	for (int i = 1; i < 5; i++)
	  theStream << "," << this->itsKpar[i];

	theStream << "]\n";
	    
	std::pair<double,double> freqRange = this->freqLimits();
	theStream << "Freq Range = " << freqRange.first << " - " << freqRange.second << "\n";
  
      }

      void HIprofileS3SAX::print(std::ostream& theStream)
      {
	theStream << this->itsRA << "\t" << this->itsDec << "\t" << this->itsIntFlux << "\t" 
		  << this->itsComponent.maj() << "\t" << this->itsComponent.min() << "\t" << this->itsComponent.pa() <<"\t"
		  << this->itsRedshift << "\t" << this->itsMHI << "\t" 
		  << this->itsFlux0 << "\t" << this->itsFluxPeak << "\t" << this->itsWidthPeak << "\t" << this->itsWidth50 << "\t" << this->itsWidth20 << "\n";
      }

        std::ostream& operator<< (std::ostream& theStream, HIprofileS3SAX &prof)
        {
            /// @details Prints a summary of the parameters to the stream
            /// @param theStream The destination stream
            /// @param prof The profile object
            /// @return A reference to the stream

	  prof.print(theStream);
	  return theStream;
        }

        HIprofileS3SAX::HIprofileS3SAX(std::string &line)
        {
            /// @details Constructs a HIprofileS3SAX object from a
            /// line of text from an ascii file. Uses the
            /// HIprofileS3SAX::define() function.
	  this->define(line);
	}

        void HIprofileS3SAX::define(std::string &line)
        {
            /// @details Defines a HIprofileS3SAX object from a line of
            /// text from an ascii file. This line should be formatted in
            /// the correct way to match the output from the appropriate
            /// python script. The columns should be: RA - DEC - Flux -
            /// Alpha - Beta - Major axis - Minor axis - Pos.Angle -
            /// redshift - HI Mass - f_0 - f_p - W_p - W_50 - W_20 (Alpha & Beta are the
            /// spectral index and spectral curvature - these are produced
            /// by the python scripts, but not used for the HI profiles,
            /// only Continuum profiles.)  The define() function is called
            /// to set up the profile description.
            /// @param line A line from the ascii input file

	  double maj, min, pa;
            std::stringstream ss(line);
            ss >> this->itsRA >> this->itsDec >> this->itsIntFlux 
	       >> maj >> min >> pa
	       >> this->itsRedshift >> this->itsMHI 
	       >> this->itsFlux0 >> this->itsFluxPeak >> this->itsWidthPeak >> this->itsWidth50 >> this->itsWidth20;
            this->itsComponent.setPeak(this->itsFluxPeak * this->itsIntFlux);
	    if(maj>=min){
	      this->itsComponent.setMajor(maj);
	      this->itsComponent.setMinor(min);
	    } else{
	      this->itsComponent.setMajor(min);
	      this->itsComponent.setMinor(maj);
	    }
            this->itsComponent.setPA(pa);
            this->setup();
        }

        void HIprofileS3SAX::setup()
        {
            /// @details Sets up the \f$k_i\f$ parameters and the
            /// integrated fluxes, according to the equations described in
            /// the class definition. Need to have assigned the values of
            /// the other parameters (define() is usually called by the
            /// HIprofileS3SAX(std::string &line) constructor).

            const double lnhalf = log(0.5);
            const double lnfifth = log(0.2);
            this->itsKpar = std::vector<double>(5);
            double a = this->itsFlux0, b = this->itsFluxPeak, c = this->itsWidthPeak, d = this->itsWidth50, e = this->itsWidth20;
            this->itsKpar[0] = 0.25 * (lnhalf * (c * c - e * e) + lnfifth * (d * d - c * c)) / (lnhalf * (c - e) + lnfifth * (d - c));
            this->itsKpar[1] = (0.25 * (c * c - d * d) + this->itsKpar[0] * (d - c)) / lnhalf;
            this->itsKpar[2] = b * exp((2.*this->itsKpar[0] - c) * (2.*this->itsKpar[0] - c) / (4.*this->itsKpar[1]));

            if (fabs(a - b) / a < 1.e-8)
                this->itsKpar[3] = this->itsKpar[4] = 0.;
            else if (c > 0) {
                this->itsKpar[3] = c * c * b * b / (4.*(b * b - a * a));
                this->itsKpar[4] = a * sqrt(this->itsKpar[3]);
            } else
                this->itsKpar[3] = this->itsKpar[4] = 0.;

            this->itsSideFlux = (this->itsKpar[2] * sqrt(this->itsKpar[1]) / M_2_SQRTPI) * erfc((0.5 * c - this->itsKpar[0]) / sqrt(this->itsKpar[1]));

            if (fabs(a - b) / a < 1.e-8)
                this->itsMiddleFlux = a * c;
            else if (c > 0)
                this->itsMiddleFlux = 2. * this->itsKpar[4] * atan(c / sqrt(4.*this->itsKpar[3] - c * c));
            else
                this->itsMiddleFlux = 0.;

        }

        double HIprofileS3SAX::flux(double nu)
        {
            /// @details This function returns the flux value at a
            /// particular frequency, using the expressions shown in the
            /// comments for define(). This is a monochromatic flux, not
            /// integrated.
            /// @param nu The frequency, in Hz.
            /// @return The flux, in Jy.

            double flux;
            double dvel = freqToHIVel(nu) - redshiftToVel(this->itsRedshift);

            if (fabs(dvel) < 0.5*this->itsWidthPeak)
                flux = this->itsKpar[4] / sqrt(this->itsKpar[3] - dvel * dvel);
            else {
                double temp = fabs(dvel) - this->itsKpar[0];
                flux = this->itsKpar[2] * exp(-temp * temp / this->itsKpar[1]);
            }

            return flux * this->itsIntFlux;
        }


        double HIprofileS3SAX::flux(double nu1, double nu2)
        {
            /// @details This function returns the flux integrated between
            /// two frequencies. This can be used to calculate the flux in
            /// a given channel, for instance. The flux is divided by the
            /// frequency range, so that units of Jy are returned.
            /// @param nu1 One frequency, in Hz.
            /// @param nu2 The second frequency, in Hz.
            /// @return The flux, in Jy.

            double f[2], dv[2];
            double a = this->itsFlux0, b = this->itsFluxPeak, c = this->itsWidthPeak;
            int loc[2];
            dv[0] = freqToHIVel(std::max(nu1, nu2)) - redshiftToVel(this->itsRedshift); // lowest relative velocty
            dv[1] = freqToHIVel(std::min(nu1, nu2)) - redshiftToVel(this->itsRedshift); // highest relative velocity
            f[0] = f[1] = 0.;
//  ASKAPLOG_DEBUG_STR(logger, "Finding flux b/w " << nu1 << " & " << nu2 << " --> or " << dv[0] << " and " << dv[1] << "  (with peaks at +-"<<0.5*this->itsWidthPeak<<")");

            for (int i = 0; i < 2; i++) {
                if (dv[i] < -0.5*c) {
                    f[i] += (this->itsKpar[2] * sqrt(this->itsKpar[1]) / M_2_SQRTPI) * erfc((0. - dv[i] - this->itsKpar[0]) / sqrt(this->itsKpar[1]));
                    loc[i] = 1;
                } else {
                    f[i] += this->itsSideFlux;

                    if (dv[i] < 0.5*c) {
                        if (fabs(a - b) / a < 1.e-8) f[i] += a * (dv[i] + 0.5 * c);
                        else     f[i] += this->itsKpar[4] * (atan(dv[i] / sqrt(this->itsKpar[3] - dv[i] * dv[i])) + atan(c / sqrt(4.*this->itsKpar[3] - c * c)));

                        loc[i] = 2;
                    } else {
                        f[i] += this->itsMiddleFlux;
                        f[i] += this->itsSideFlux - (this->itsKpar[2] * sqrt(this->itsKpar[1]) / M_2_SQRTPI) * erfc((dv[i] - this->itsKpar[0]) / sqrt(this->itsKpar[1]));
                        loc[i] = 3;
                    }
                }

            }

            double flux = (f[1] - f[0]) / (dv[1] - dv[0]);
            return flux * this->itsIntFlux;

        }

      
      std::pair<double,double> HIprofileS3SAX::freqLimits()
      {
	/// @details This function returns the minimum & maximum 
	/// frequencies (in that order) that will be affected by a
	/// given source. This takes the limit of the exponential
	/// tails as the location where the flux drops below the
	/// minimum float value.
	/// @return A std::pair<double,double>, where the first value
	/// is the maximum velocity, and the second is the minimum
	/// velocity.

	double maxAbsVel = this->itsKpar[0] + sqrt( this->itsKpar[1] * log(this->itsKpar[2]*MAXFLOAT) );
	std::pair<double,double> freqLimits;
        float vel0 = redshiftToVel(this->itsRedshift);
	freqLimits.first = HIVelToFreq( vel0 + maxAbsVel );
	freqLimits.second = HIVelToFreq( vel0 - maxAbsVel );
	return freqLimits;

      }



    }

}
