/// @file
///
/// Implementation of RM Synthesis
///
/// @copyright (c) 2014 CSIRO
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
#include <polarisation/RMSynthesis.h>
#include <askap_analysis.h>

// #include <polarisation/PolarisationData.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/MaskArrMath.h>
#include <complex>
#include <casa/BasicSL/Complex.h>
#include <scimath/Fitting/FitGaussian.h>

#include <Common/ParameterSet.h>

///@brief Where the log messages go.
ASKAP_LOGGER(logger, ".rmsynthesis");

namespace askap {

namespace analysis {

/// The default type of weighting, if not specified in the parset.
static const std::string defaultWeight = "variance";

RMSynthesis::RMSynthesis(const LOFAR::ParameterSet &parset):
    itsWeightType(parset.getString("weightType", defaultWeight)),
    itsNumPhiChan(parset.getUint("numPhiChan", 0)),
    itsDeltaPhi(parset.getFloat("deltaPhi", 0.)),
    itsPhiZero(parset.getFloat("phiZero", 0.)),
    itsPhi(0),
    itsFaradayDF(0),
    itsPhiForRMSF(0),
    itsRMSF(0)

{

    ASKAPCHECK(itsNumPhiChan > 0,
               "numPhiChan (given as " << itsNumPhiChan << ") needs to be > 0");
    ASKAPCHECK(itsDeltaPhi > 0.,
               "deltaPhi (given as " << itsDeltaPhi << ") needs to be > 0");

    if (itsWeightType != "uniform" && itsWeightType != "variance") {
        ASKAPLOG_WARN_STR(logger,
                          "RMSynthesis: weightType must be either " <<
                          "'uniform' or 'variance' (you have " <<
                          itsWeightType << "). Setting to " << defaultWeight);
        itsWeightType = defaultWeight;
    }

    this->defineVectors();
}

void RMSynthesis::defineVectors()
{
    itsPhi.resize(itsNumPhiChan);
    casa::indgen<float>(itsPhi,
                        -0.5 * itsNumPhiChan * itsDeltaPhi,
                        itsDeltaPhi);

    itsPhiForRMSF.resize(2 * itsNumPhiChan);
    casa::indgen<float>(itsPhiForRMSF,
                        -1.*itsNumPhiChan * itsDeltaPhi,
                        itsDeltaPhi);

    itsFaradayDF.resize(itsNumPhiChan);
    itsFaradayDF = 0.;

    itsRMSF.resize(2 * itsNumPhiChan);
    itsRMSF = 0.;

}


// ** PolarisationData not yet implemented fully **
// void RMSynthesis::calculate(PolarisationData &poldata)
// {
//     casa::Vector<float> lsq = poldata.l2();
//     // q = Q/Imod, u = U/Imod, p = q + iu

//     casa::Vector<float> q=poldata.Q().spectrum() / poldata.Imod();
//     casa::Vector<float> u=poldata.U().spectrum() / poldata.Imod();

//     this->calculate(lsq,q,u,poldata.noise());

// }

void RMSynthesis::calculate(const casa::Vector<float> &lsq,
                            const casa::Vector<float> &q,
                            const casa::Vector<float> &u,
                            const casa::Vector<float> &noise)
{

    // Ensure all arrays are the same size
    ASKAPASSERT(lsq.size() == q.size());
    ASKAPASSERT(lsq.size() == u.size());
    ASKAPASSERT(lsq.size() == noise.size());

    // p = q + iu
    casa::Vector<casa::Complex> p = casa::makeComplex(q, u);

    if (itsWeightType == "variance") {
        itsWeights = casa::Vector<float>(noise.size(), 0.);
        itsWeights = casa::pow(noise(noise>0.F), -2.);
    } else {
        itsWeights = casa::Vector<float>(noise.size(), 1.);
    }

    itsFDFnoise = casa::mean(noise) / sqrt(noise.size());

    // K = \sum(w_i)^-1
    itsNormalisation = 1. / casa::sum(itsWeights);

    // \lambda^2_0 = K * \sum(w_i*\lambda^2_i)
    itsRefLambdaSquared = itsNormalisation * casa::sum(itsWeights * lsq);

    // variance in the lsq distribution
    itsLambdaSquaredVariance = (casa::sum(lsq * lsq) - pow(casa::sum(lsq), 2) / lsq.size()) /
                               float(lsq.size() - 1);

    // Compute FDF
    for (size_t j = 0; j < itsNumPhiChan; j++) {
        casa::Vector<float> phase = -2.F * itsPhi[j] * (lsq - itsRefLambdaSquared);
        casa::Vector<casa::Complex> sampling = casa::makeComplex(itsWeights * cos(phase),
                                               itsWeights * sin(phase));
        itsFaradayDF[j] = itsNormalisation * casa::sum(p * sampling);
    }

    // Compute RMSF
    for (size_t j = 0; j < 2.*itsNumPhiChan; j++) {
        casa::Vector<float> phase = -2.F * itsPhiForRMSF[j] * (lsq - itsRefLambdaSquared);
        casa::Vector<casa::Complex> sampling = casa::makeComplex(itsWeights * cos(phase),
                                               itsWeights * sin(phase));
        itsRMSF[j] = itsNormalisation * casa::sum(sampling);
    }

    this->fitRMSF();

}

void RMSynthesis::fitRMSF()
{
// ** not yet fully implemented - TBC **

    casa::Vector<float> rmsf_p = casa::amplitude(itsRMSF);
    float minRMSF, maxRMSF;
    casa::IPosition locMin, locMax;
    casa::minMax<float>(minRMSF, maxRMSF, locMin, locMax, rmsf_p);

    // move left
    int limitLower = locMax[0];
    bool keepGoing=true;
    for(; limitLower>0 && keepGoing; limitLower--){
        keepGoing = (rmsf_p[limitLower] > rmsf_p[limitLower-1]);
    }

    // move right
    int limitUpper = locMax[0];
    int max=rmsf_p.size()-1;
    keepGoing=true;
    for(; limitUpper<max && keepGoing; limitUpper++){
        keepGoing = (rmsf_p[limitUpper] > rmsf_p[limitUpper+1]);
    }

    ASKAPLOG_DEBUG_STR(logger, "Fitting to peak of RMSF between phi channels " <<
                       limitLower << " and " << limitUpper);

    int size = limitUpper - limitLower + 1;
    casa::Matrix<casa::Double> pos(size, 1);
    casa::Vector<casa::Double> f(size);
    casa::Vector<casa::Double> sigma(size);

    for (int i = limitLower; i <= limitUpper; i++) {
        pos.row(i - limitLower) = itsPhiForRMSF[i];
        f(i - limitLower) = rmsf_p[i];
        sigma(i - limitLower) = 1.;
    }

    // ASKAPLOG_DEBUG_STR(logger, "RMSF fit: pos = " << pos);
    // ASKAPLOG_DEBUG_STR(logger, "RMSF fit: f = " << f);

    casa::FitGaussian<casa::Double> fitter;
    fitter.setDimensions(1);
    fitter.setNumGaussians(1);
    casa::Matrix<casa::Double> estimate;
    estimate.resize(1, 3);
    estimate(0, 0) = 1.;
    estimate(0, 1) = itsNumPhiChan - limitLower;
    estimate(0, 2) = itsDeltaPhi;
    fitter.setFirstEstimate(estimate);
    casa::Matrix<casa::Double> solution = fitter.fit(pos, f, sigma);

    if (!fitter.converged())
        ASKAPLOG_WARN_STR(logger, "RMSF fit did not converge!");
    else {
        itsRMSFwidth = solution(0, 2);
    }


}

}

}
