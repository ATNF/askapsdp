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

#include <simulationutilities/HIprofileS3SEX.h>
#include <simulationutilities/SimulationUtilities.h>
#include <simulationutilities/SpectralUtilities.h>
#include <iostream>
#include <math.h>
#include <stdlib.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

ASKAP_LOGGER(logger, ".hiprofiles3sex");


namespace askap {

    namespace simulations {

        HIprofileS3SEX::HIprofileS3SEX(std::string &line)
        {
            /// @details Constructs a HIprofileS3SEX object from a
            /// line of text from an ascii file. Uses the
            /// HIprofileS3SEX::define() function.
	  this->define(line);
	}

        void HIprofileS3SEX::define(std::string &line)
        {
            /// @details Defines a HIprofileS3SEX object from a line of
            /// text from an ascii file. This line should be formatted in
            /// the correct way to match the output from the appropriate
            /// python script. The columns should be: RA - DEC - Flux -
            /// Alpha - Beta - Major axis - Minor axis - Pos.Angle -
            /// redshift - HI Mass - galaxy type. (Alpha & Beta are the
            /// spectral index and spectral curvature - these are produced
            /// by the python scripts, but not used for the HI profiles,
            /// only Continuum profiles.)  The define() function is called
            /// to set up the profile description.
            /// @param line A line from the ascii input file

	  double flux, maj, min, pa;
            int type;
            std::stringstream ss(line);
            ss >> this->itsRA >> this->itsDec >> flux >> this->itsAlpha >> this->itsBeta >> maj >> min >> pa >> this->itsRedshift >> this->itsMHI >> type;
            this->itsSourceType = GALTYPE(type);
            this->itsComponent.setPeak(flux);
	    if(maj>=min){
	      this->itsComponent.setMajor(maj);
	      this->itsComponent.setMinor(min);
	    } else{
	      this->itsComponent.setMajor(min);
	      this->itsComponent.setMinor(maj);
	    }
            this->itsComponent.setPA(pa);

            this->setup(this->itsSourceType, this->itsRedshift, this->itsMHI, this->maj(), this->min());
        }

        HIprofileS3SEX::HIprofileS3SEX(const HIprofileS3SEX& h):
                HIprofile(h)
        {
            operator=(h);
        }

        HIprofileS3SEX& HIprofileS3SEX::operator= (const HIprofileS3SEX& h)
        {
            if (this == &h) return *this;

            ((HIprofile &) *this) = h;
            this->itsSourceType = h.itsSourceType;
            this->itsVelZero = h.itsVelZero;
            this->itsVRot = h.itsVRot;
            this->itsDeltaVel = h.itsDeltaVel;
            this->itsDipAmp = h.itsDipAmp;
            this->itsSigmaEdge = h.itsSigmaEdge;
            this->itsSigmaDip = h.itsSigmaDip;
            this->itsMaxVal = h.itsMaxVal;
            this->itsIntFlux = h.itsIntFlux;
            this->itsEdgeFlux = h.itsEdgeFlux;
            this->itsMiddleFlux = h.itsMiddleFlux;
            this->itsProfileFlux = h.itsProfileFlux;
            return *this;
        }

      void HIprofileS3SEX::diagnostic(std::ostream& theStream)
      {
            theStream << "HI profile summary:\n";
            theStream << "z=" << this->itsRedshift << "\n";
            theStream << "M_HI=" << this->itsMHI << "\n";
            theStream << "V_0=" << this->itsVelZero << "\n";
            theStream << "Vrot=" << this->itsVRot << "\n";
            theStream << "Vwidth=" << this->itsDeltaVel << "\n";
            theStream << "Dip Amplitude=" << this->itsDipAmp << "\n";
            theStream << "Sigma_edge=" << this->itsSigmaEdge << "\n";
            theStream << "Sigma_dip=" << this->itsSigmaDip << "\n";
            theStream << "Peak value=" << this->itsMaxVal << "\n";
            theStream << "Integrated Flux=" << this->itsIntFlux << "\n";
            theStream << "Edge int. flux=" << this->itsEdgeFlux << "\n";
            theStream << "Middle int. flux=" << this->itsMiddleFlux << "\n";
            theStream << "Profile int. flux=" << this->itsProfileFlux << "\n";

      }

      void HIprofileS3SEX::print(std::ostream& theStream)
      {
	theStream << this->itsRA << "\t" << this->itsDec << "\t" 
		  << this->itsComponent.peak() << "\t" << this->itsAlpha << "\t" << this->itsBeta << "\t" 
		  << this->itsComponent.maj() << "\t" << this->itsComponent.min() << "\t" << this->itsComponent.pa() << "\t"
		  << this->itsRedshift << "\t" << this->itsMHI << "\t" << int(this->itsSourceType) << "\n";
      }

        std::ostream& operator<< (std::ostream& theStream, HIprofileS3SEX &prof)
        {
            /// @details Prints a summary of the parameters to the stream
            /// @param theStream The destination stream
            /// @param prof The profile object
            /// @return A reference to the stream

	  prof.print(theStream);
	  return theStream;
        }

        void HIprofileS3SEX::setup(GALTYPE type, double z, double mhi, double maj, double min)
        {
            /// @details This function assigns values to all the
            /// parameters of the profile. The profile is described by
            /// Gaussian shapes: the edges of the profile are Gaussian
            /// tails
            /// \f$f(V) = M \exp( -(V-(V_0\pm\Delta V))^2/2\sigma_e^2), |V-V_0|>\Delta V\f$,
            /// while the dip between the peaks is an inverted Gaussian:
            /// \f$f(V) = M - D \exp( -(V-V_0)^2/2\sigma_d^2 ) + D \exp( -\Delta V^2/2\sigma_d^2 ), |V-V_0|<\Delta V \f$
            /// There are a number of randomly generated values: itsVRot, itsSigmaEdge and itsDipAmp
            /// @param type The type of galaxy: used to give the range of VRot values. Only SFG and SBG give non-zero values.
            /// @param z The redshift of the profile
            /// @param mhi The HI mass - used to get the integrated flux
            /// @param maj The major axis - used to get the inclination angle, and hence the \f$\Delta V\f$ value from VRot
            /// @param min The minor axis - used to get the inclination angle, and hence the \f$\Delta V\f$ value from VRot

            const double rootTwoPi = 4. * M_SQRT1_2 / M_2_SQRTPI;  // sqrt(2pi), using, from math.h: M_SQRT1_2=1/sqrt(2) and M_2_SQRTPI=2/sqrt(pi),

            this->itsIntFlux = this->integratedFlux(z, mhi);
            this->itsVRot = vrotMin[type] + (vrotMax[type] - vrotMin[type]) * random() / (RAND_MAX + 1.0);

            if (maj == min) this->itsDeltaVel = 0.01 * this->itsVRot;
            else this->itsDeltaVel = this->itsVRot * sin(acos(min / maj));

            this->itsVelZero = redshiftToVel(z);

            this->itsSigmaEdge = normalRandomVariable(doubleHornShape[EDGE_SIG_MEAN], doubleHornShape[EDGE_SIG_SD]);
            this->itsSigmaEdge = std::max(this->itsSigmaEdge, doubleHornShape[EDGE_SIG_MIN]);
            this->itsSigmaEdge = std::min(this->itsSigmaEdge, doubleHornShape[EDGE_SIG_MAX]);
            this->itsMaxVal = 1. / (rootTwoPi * this->itsSigmaEdge);

            double ampDipFactor = doubleHornShape[DIP_MIN] + (doubleHornShape[DIP_MAX] - doubleHornShape[DIP_MIN]) * random() / (RAND_MAX + 1.0);
            this->itsDipAmp = ampDipFactor * this->itsMaxVal;
            this->itsSigmaDip = doubleHornShape[DIP_SIG_SCALE] * this->itsDeltaVel;

            this->itsEdgeFlux = 0.5 * this->itsMaxVal * rootTwoPi * this->itsSigmaEdge;
            this->itsMiddleFlux = 2. * this->itsDeltaVel *
                                  (this->itsMaxVal + this->itsDipAmp / exp(this->itsDeltaVel * this->itsDeltaVel / (2.*this->itsSigmaDip * this->itsSigmaDip))) -
                                  this->itsDipAmp * rootTwoPi * this->itsSigmaDip * erf(this->itsDeltaVel / (M_SQRT2 * this->itsSigmaDip));

            this->itsProfileFlux = 2.*this->itsEdgeFlux + this->itsMiddleFlux;

        }

        double HIprofileS3SEX::flux(double nu)
        {
            /// @details This function returns the flux value at a
            /// particular frequency, using the expressions shown in the
            /// comments for define(). This is a monochromatic flux, not
            /// integrated.
            /// @param nu The frequency, in Hz.
            /// @return The flux, in Jy.

            double flux;
            double vdiff = freqToHIVel(nu) - this->itsVelZero;

            if (vdiff < (-this->itsDeltaVel)) {
                double v = vdiff + this->itsDeltaVel;
                flux = this->itsMaxVal * exp(-(v * v) / (2.*this->itsSigmaEdge * this->itsSigmaEdge));
            } else if (vdiff > this->itsDeltaVel) {
                double v = vdiff - this->itsDeltaVel;
                flux = this->itsMaxVal * exp(-(v * v) / (2.*this->itsSigmaEdge * this->itsSigmaEdge));
            } else {
                flux = this->itsMaxVal - this->itsDipAmp * exp(-vdiff * vdiff / (2.*this->itsSigmaDip * this->itsSigmaDip)) +
                       this->itsDipAmp * exp(-this->itsDeltaVel * this->itsDeltaVel / (2.*this->itsSigmaDip * this->itsSigmaDip));
            }

            return flux * this->itsIntFlux / this->itsProfileFlux;

        }


        double HIprofileS3SEX::flux(double nu1, double nu2)
        {
            /// @details This function returns the flux integrated between
            /// two frequencies. This can be used to calculate the flux in
            /// a given channel, for instance. The flux is divided by the
            /// frequency range, so that units of Jy are returned.
            /// @param nu1 One frequency, in Hz.
            /// @param nu2 The second frequency, in Hz.
            /// @return The flux, in Jy.

            const double rootPiOnTwo = 2.* M_SQRT1_2 / M_2_SQRTPI; // sqrt(pi/2), using, from math.h: M_SQRT1_2=1/sqrt(2) and M_2_SQRTPI=2/sqrt(pi),

            double v[2], f[2];
            v[0] = freqToHIVel(std::max(nu1, nu2)); // lowest velocty
            v[1] = freqToHIVel(std::min(nu1, nu2)); // highest velocity
            f[0] = f[1] = 0.;
            int loc[2];

            double minPeak = this->itsVelZero - this->itsDeltaVel;
            double maxPeak = this->itsVelZero + this->itsDeltaVel;

//  ASKAPLOG_DEBUG_STR(logger, "Finding flux b/w " << nu1 << " & " << nu2 << " --> or " << v[0] << " and " << v[1] << "  (with minpeak="<<minPeak<<" and maxpeak="<<maxPeak<<")");
            for (int i = 0; i < 2; i++) {
                if (v[i] < minPeak) {
                    f[i] += rootPiOnTwo * this->itsMaxVal * this->itsSigmaEdge * erfc((minPeak - v[i]) / (M_SQRT2 * this->itsSigmaEdge));
                    loc[i] = 1;
                } else {
                    f[i] += this->itsEdgeFlux;

                    if (v[i] < maxPeak) {
                        double norm = (v[i] - minPeak) * (this->itsMaxVal + this->itsDipAmp / exp(this->itsDeltaVel * this->itsDeltaVel / (2.*this->itsSigmaDip * this->itsSigmaDip)));
                        double dip = rootPiOnTwo * this->itsDipAmp * this->itsSigmaDip * (erfc(-1.*this->itsDeltaVel / (M_SQRT2 * this->itsSigmaDip)) -
                                     erfc((v[i] - this->itsVelZero) / (M_SQRT2 * this->itsSigmaDip)));
                        f[i] += (norm - dip);
//        ASKAPLOG_DEBUG_STR(logger, "In loc 2, norm="<<norm<<", dip="<<dip);
                        loc[i] = 2;
                    } else {
                        f[i] += this->itsMiddleFlux;
                        f[i] += rootPiOnTwo * this->itsMaxVal * this->itsSigmaEdge * erf((v[i] - maxPeak) / (M_SQRT2 * this->itsSigmaEdge));
                        loc[i] = 3;
                    }
                }
            }

            double flux = (f[1] - f[0]) / (v[1] - v[0]);
//  ASKAPLOG_DEBUG_STR(logger, "Fluxes: " << f[1] << "  " << f[0] << "  ---> " << flux << "    locations="<<loc[1]<<","<<loc[0]);
            return flux * this->itsIntFlux / this->itsProfileFlux;

        }


    }

}
