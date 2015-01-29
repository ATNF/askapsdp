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

#include <modelcomponents/HIprofileS3SEX.h>
#include <coordutils/SpectralUtilities.h>
#include <mathsutils/MathsUtils.h>
#include <iostream>
#include <math.h>
#include <stdlib.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

ASKAP_LOGGER(logger, ".hiprofiles3sex");


namespace askap {

namespace analysisutilities {

HIprofileS3SEX::HIprofileS3SEX(const HIprofileS3SEX& h):
    HIprofile(h)
{
    operator=(h);
}

HIprofileS3SEX& HIprofileS3SEX::operator= (const HIprofileS3SEX& h)
{
    if (this == &h) return *this;

    ((HIprofile &) *this) = h;
    itsSourceType = h.itsSourceType;
    itsVelZero = h.itsVelZero;
    itsVRot = h.itsVRot;
    itsDeltaVel = h.itsDeltaVel;
    itsDipAmp = h.itsDipAmp;
    itsSigmaEdge = h.itsSigmaEdge;
    itsSigmaDip = h.itsSigmaDip;
    itsMaxVal = h.itsMaxVal;
    itsIntFlux = h.itsIntFlux;
    itsEdgeFlux = h.itsEdgeFlux;
    itsMiddleFlux = h.itsMiddleFlux;
    itsProfileFlux = h.itsProfileFlux;
    return *this;
}

void HIprofileS3SEX::init()
{
    itsVelZero = 0.;
    itsVRot = 0.;
    itsDeltaVel = 0.;
    itsDipAmp = 0.;
    itsSigmaEdge = 0.;
    itsSigmaDip = 0.;
    itsMaxVal = 0.;
    itsIntFlux = 0.;
    itsEdgeFlux = 0.;
    itsMiddleFlux = 0.;
    itsProfileFlux = 0.;
}

HIprofileS3SEX::HIprofileS3SEX(const std::string &line)
{
    this->define(line);
}

HIprofileS3SEX::HIprofileS3SEX(const GALTYPE type,
                               const double z,
                               const double mhi,
                               const double maj,
                               const double min):
    itsSourceType(type)
{
    this->init();
    itsRedshift = z;
    itsMHI = mhi;
    itsMaj = maj;
    itsMin = min;
    this->prepareForUse();
}


HIprofileS3SEX::HIprofileS3SEX(const GALTYPE type,
                               const double z,
                               const double mhi,
                               const double maj,
                               const double min,
                               const long componentNum,
                               const long galaxyNum):
    itsSourceType(type)
{
    this->init();
    itsRedshift = z;
    itsMHI = mhi;
    itsMaj = maj;
    itsMin = min;
    this->prepareForUse(componentNum, galaxyNum);
}


void HIprofileS3SEX::define(const std::string &line)
{

    int type;
    std::stringstream ss(line);
    ss >> itsRA >> itsDec >> itsFlux
       >> itsAlpha >> itsBeta
       >> itsMaj >> itsMin >> itsPA
       >> itsRedshift >> itsMHI >> type;
    itsSourceType = GALTYPE(type);
    this->PosToID();
    this->checkShape();
    this->prepareForUse();
}

void HIprofileS3SEX::diagnostic(std::ostream& theStream)
{
    theStream << "HI profile summary:\n";
    theStream << "z=" << itsRedshift << "\n";
    theStream << "M_HI=" << itsMHI << "\n";
    theStream << "V_0=" << itsVelZero << "\n";
    theStream << "Vrot=" << itsVRot << "\n";
    theStream << "Vwidth=" << itsDeltaVel << "\n";
    theStream << "Dip Amplitude=" << itsDipAmp << "\n";
    theStream << "Sigma_edge=" << itsSigmaEdge << "\n";
    theStream << "Sigma_dip=" << itsSigmaDip << "\n";
    theStream << "Peak value=" << itsMaxVal << "\n";
    theStream << "Integrated Flux=" << itsIntFlux << "\n";
    theStream << "Edge int. flux=" << itsEdgeFlux << "\n";
    theStream << "Middle int. flux=" << itsMiddleFlux << "\n";
    theStream << "Profile int. flux=" << itsProfileFlux << "\n";
    theStream << "Min Freq=" << itsMinFreq << "\n";
    theStream << "Max Freq=" << itsMaxFreq << "\n";

}

void HIprofileS3SEX::print(std::ostream& theStream) const
{
    theStream << itsRA << "\t" << itsDec << "\t" << itsFlux << "\t"
              << itsAlpha << "\t" << itsBeta << "\t"
              << itsMaj << "\t" << itsMin << "\t" << itsPA << "\t"
              << itsRedshift << "\t" << itsMHI
              << "\t" << int(itsSourceType) << "\n";
}

std::ostream& operator<< (std::ostream& theStream, const HIprofileS3SEX &prof)
{

    prof.print(theStream);
    return theStream;
}

// void HIprofileS3SEX::setup(GALTYPE type, double z, double mhi, double maj, double min)
// {
//     /// @details This function assigns values to all the
//     /// parameters of the profile. The profile is described by
//     /// Gaussian shapes: the edges of the profile are Gaussian
//     /// tails
//     /// \f$f(V) = M \exp( -(V-(V_0\pm\Delta V))^2/2\sigma_e^2), |V-V_0|>\Delta V\f$,
//     /// while the dip between the peaks is an inverted Gaussian:
//     /// \f$f(V) = M - D \exp( -(V-V_0)^2/2\sigma_d^2 ) + D \exp( -\Delta V^2/2\sigma_d^2 ), |V-V_0|<\Delta V \f$
//     /// There are a number of randomly generated values: itsVRot, itsSigmaEdge and itsDipAmp
//     /// @param type The type of galaxy: used to give the range of VRot values. Only SFG and SBG give non-zero values.
//     /// @param z The redshift of the profile
//     /// @param mhi The HI mass - used to get the integrated flux
//     /// @param maj The major axis - used to get the inclination angle, and hence the \f$\Delta V\f$ value from VRot
//     /// @param min The minor axis - used to get the inclination angle, and hence the \f$\Delta V\f$ value from VRot

//   itsSourceType = type;
//   itsRedshift = z;
//   itsMHI = mhi;
//   itsMaj = maj;
//   itsMin = min;

//   this->prepareForUse();

// }

void HIprofileS3SEX::prepareForUse()
{
    if (itsSourceType == SBG || itsSourceType == SFG) {

        double n1 = random() / (RAND_MAX + 1.0);
        double n2 = random() / (RAND_MAX + 1.0);
        prepareForUse(n1, n2);

    }
}

void HIprofileS3SEX::prepareForUse(const long num1, const long num2)
{
    if (itsSourceType == SBG || itsSourceType == SFG) {
        double n1 = (num1 % 1000 + 0.5) / 1000.;
        double n2 = (num2 % 1000 + 0.5) / 1000.;

        prepareForUse(n1, n2);
    }
}

void HIprofileS3SEX::prepareForUse(const double n1, const double n2)
{
    if (itsSourceType == SBG || itsSourceType == SFG) {

        itsVRot = vrotMin[itsSourceType] +
                  (vrotMax[itsSourceType] - vrotMin[itsSourceType]) * n1;
        itsSigmaEdge = normalRandomVariable(doubleHornShape[EDGE_SIG_MEAN],
                                            doubleHornShape[EDGE_SIG_SD]);
        itsSigmaEdge = std::max(itsSigmaEdge, doubleHornShape[EDGE_SIG_MIN]);
        itsSigmaEdge = std::min(itsSigmaEdge, doubleHornShape[EDGE_SIG_MAX]);
        itsMaxVal = 1. / (rootTwoPi * itsSigmaEdge);
        itsDipAmp = (doubleHornShape[DIP_MIN] +
                     (doubleHornShape[DIP_MAX] - doubleHornShape[DIP_MIN]) * n2) *
                    itsMaxVal;

        this->setup();
    }
}

void HIprofileS3SEX::setup()
{
    // must have run one of the prepareForUse functions first.

    itsIntFlux = this->integratedFlux(itsRedshift, itsMHI);

    if (itsMaj == itsMin) {
        itsDeltaVel = 0.01 * itsVRot;
    } else {
        itsDeltaVel = itsVRot * sin(acos(itsMin / itsMaj));
    }

    itsVelZero = redshiftToVel(itsRedshift);

    itsSigmaDip = doubleHornShape[DIP_SIG_SCALE] * itsDeltaVel;

    itsEdgeFlux = 0.5 * itsMaxVal * rootTwoPi * itsSigmaEdge;
    double exponent = itsDeltaVel * itsDeltaVel /
                      (2.*itsSigmaDip * itsSigmaDip);
    itsMiddleFlux = 2. * itsDeltaVel *
                    (itsMaxVal + itsDipAmp / exp(exponent)) -
                    itsDipAmp * rootTwoPi * itsSigmaDip *
                    erf(itsDeltaVel / (M_SQRT2 * itsSigmaDip));

    itsProfileFlux = 2.*itsEdgeFlux + itsMiddleFlux;

    itsMinFreq =  HIVelToFreq(itsVelZero - itsDeltaVel -
                              itsSigmaEdge * sqrt(2.*log(MAXFLOAT *
                                      itsMaxVal)));
    itsMaxFreq =  HIVelToFreq(itsVelZero + itsDeltaVel +
                              itsSigmaEdge * sqrt(2.*log(MAXFLOAT *
                                      itsMaxVal)));

    if (itsMinFreq > itsMaxFreq) {
        std::swap(itsMinFreq, itsMaxFreq);
    }

}

const double HIprofileS3SEX::flux(const double nu, const int istokes)
{

    if (istokes > 0) return 0.;
    else {
        if (itsMHI > 0.) {
            double flux;
            double vdiff = freqToHIVel(nu) - itsVelZero;

            if (vdiff < (-itsDeltaVel)) {
                double v = vdiff + itsDeltaVel;
                double exponent = -(v * v) / (2.*itsSigmaEdge * itsSigmaEdge);
                flux = itsMaxVal * exp(exponent);
            } else if (vdiff > itsDeltaVel) {
                double v = vdiff - itsDeltaVel;
                double exponent = -(v * v) / (2.*itsSigmaEdge * itsSigmaEdge);
                flux = itsMaxVal * exp(exponent);
            } else {
                double exponent1 = -(vdiff * vdiff) / (2.*itsSigmaDip * itsSigmaDip);
                double exponent2 = -(itsDeltaVel * itsDeltaVel) /
                                   (2.*itsSigmaDip * itsSigmaDip);
                flux = itsMaxVal - itsDipAmp * exp(exponent1) +
                       itsDipAmp * exp(exponent2);
            }

            return flux * itsIntFlux / itsProfileFlux;

        } else return 0.;
    }
}


const double HIprofileS3SEX::fluxInt(const double nu1, const double nu2, const int istokes)
{

    if (istokes > 0) return 0.;
    else {
        if (itsMHI > 0.) {

            // sqrt(pi/2), using, from math.h: M_SQRT1_2=1/sqrt(2) and M_2_SQRTPI=2/sqrt(pi),
            const double rootPiOnTwo = 2.* M_SQRT1_2 / M_2_SQRTPI;

            double v[2], f[2];
            v[0] = freqToHIVel(std::max(nu1, nu2)); // lowest velocty
            v[1] = freqToHIVel(std::min(nu1, nu2)); // highest velocity
            f[0] = f[1] = 0.;

            double minPeak = itsVelZero - itsDeltaVel;
            double maxPeak = itsVelZero + itsDeltaVel;

            for (int i = 0; i < 2; i++) {
                if (v[i] < minPeak) {
                    f[i] += rootPiOnTwo * itsMaxVal * itsSigmaEdge *
                            erfc((minPeak - v[i]) / (M_SQRT2 * itsSigmaEdge));
                } else {
                    f[i] += itsEdgeFlux;

                    if (v[i] < maxPeak) {
                        double exponent = itsDeltaVel * itsDeltaVel /
                                          (2.*itsSigmaDip * itsSigmaDip);
                        double norm = (v[i] - minPeak) *
                                      (itsMaxVal + itsDipAmp / exp(exponent));

                        double err1 = erfc(-1.*itsDeltaVel /
                                           (M_SQRT2 * itsSigmaDip));
                        double err2 = erfc((v[i] - itsVelZero) /
                                           (M_SQRT2 * itsSigmaDip));

                        double dip = rootPiOnTwo * itsDipAmp * itsSigmaDip *
                                     (err1 - err2);
                        f[i] += (norm - dip);
                    } else {
                        f[i] += itsMiddleFlux;
                        f[i] += rootPiOnTwo * itsMaxVal * itsSigmaEdge *
                                erf((v[i] - maxPeak) / (M_SQRT2 * itsSigmaEdge));
                    }
                }
            }

            double flux = (f[1] - f[0]) / (v[1] - v[0]);
            return flux * itsIntFlux / itsProfileFlux;
        } else return 0.;
    }
}


}

}

