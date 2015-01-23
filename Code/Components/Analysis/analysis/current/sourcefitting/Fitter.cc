/// @file
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
/// @author Matthew Whiting <matthew.whiting@csiro.au>
///

#include <askap_analysis.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <sourcefitting/Fitter.h>
#include <sourcefitting/Component.h>
#include <mathsutils/MathsUtils.h>

#include <scimath/Fitting/FitGaussian.h>
#include <scimath/Functionals/Gaussian1D.h>
#include <scimath/Functionals/Gaussian2D.h>
#include <scimath/Functionals/Gaussian3D.h>
#include <casa/namespace.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <utility>
#include <math.h>

///@brief Where the log messages go.
ASKAP_LOGGER(logger, ".sourcefitting");

using namespace duchamp;

namespace askap {

namespace analysis {

namespace sourcefitting {

void Fitter::setEstimates(std::vector<SubComponent> cmpntList,
                          duchamp::FitsHeader &head)
{

    itsFitter.setDimensions(2);
    itsFitter.setNumGaussians(itsNumGauss);

    casa::Matrix<casa::Double> estimate;
    estimate.resize(itsNumGauss, 6);

    uInt nCmpnt = cmpntList.size();

    for (uInt g = 0; g < itsNumGauss; g++) {
        uInt cmpnt = g % nCmpnt;
        estimate(g, 0) = cmpntList[cmpnt].peak();
        estimate(g, 1) = cmpntList[cmpnt].x();
        estimate(g, 2) = cmpntList[cmpnt].y();
        estimate(g, 3) = cmpntList[cmpnt].maj();
        estimate(g, 4) = cmpntList[cmpnt].min() / cmpntList[cmpnt].maj();
        estimate(g, 5) = cmpntList[cmpnt].pa();

        if (head.beam().originString() != "EMPTY") { // if the beam is known,
            bool size = (head.beam().maj() > cmpntList[cmpnt].maj());

            // if the subcomponent is smaller than the beam, or if we
            // don't want to fit the size parameters, change the
            // estimates of the parameters to the beam size

            // WTF??? WE MAY WANT TO KEEP THE SHAPE CONSTANT AT SOMETHING OTHER THAN THE BEAM. REMOVING CHECKS ON THE FIT FLAGS
            //                        if (size || !itsParams.flagFitThisParam(3))
            if (size)
                estimate(g, 3) = head.beam().maj();

            //                        if (size || !itsParams.flagFitThisParam(4))
            if (size)
                estimate(g, 4) = head.beam().min() / head.beam().maj();

            //                        if (size || !itsParams.flagFitThisParam(5))
            if (size)
                estimate(g, 5) = head.beam().pa() * M_PI / 180.;
        }

    }


    itsFitter.setFirstEstimate(estimate);

    if (head.beam().min() > 0) {
        itsParams.setBeamSize(head.beam().min());
    } else {
        itsParams.setBeamSize(1.);
    }

    ASKAPLOG_DEBUG_STR(logger, "Initial estimates of parameters follow: ");
    logparameters(estimate);

    if (nCmpnt == 1) {
        casa::Gaussian2D<casa::Double>
        gauss(estimate(0, 0),
              estimate(0, 1), estimate(0, 2),
              estimate(0, 3), estimate(0, 4), estimate(0, 5));
    }
}

//**************************************************************//

void Fitter::setRetries()
{
    casa::Matrix<casa::Double> retryfactors;
    casa::Matrix<casa::Double> baseRetryfactors;
    baseRetryfactors.resize(1, 6);
    retryfactors.resize(itsNumGauss, 6);
    baseRetryfactors(0, 0) = 1.1;
    baseRetryfactors(0, 1) = 0.1;
    baseRetryfactors(0, 2) = 0.1;
    baseRetryfactors(0, 3) = 1.1;
    baseRetryfactors(0, 4) = 1.01;
    baseRetryfactors(0, 5) = M_PI / 180.;

    for (unsigned int g = 0; g < itsNumGauss; g++) {
        for (unsigned int i = 0; i < 6; i++) {
            retryfactors(g, i) = baseRetryfactors(0, i);
        }
    }

    //  itsFitter.setRetryFactors(retryfactors);
    // Try not setting these for now and just use the defaults.
}

//**************************************************************//

void Fitter::setMasks()
{
    // mask the beam parameters
    for (unsigned int g = 0; g < itsNumGauss; g++) {
        for (unsigned int p = 0; p < 6; p++) {
            itsFitter.mask(g, p) = itsParams.flagFitThisParam(p);
        }
    }
}

//**************************************************************//

void logparameters(Matrix<Double> &m, std::string loc)
{
    uInt g, p;

    for (g = 0; g < m.nrow(); g++) {
        std::stringstream outmsg;
        outmsg << "Component Flux,X0,Y0,MAJ,MIN/MAJ,PA = ";
        outmsg.precision(8);
        outmsg.setf(ios::fixed);
        outmsg << m(g, 0) << ", ";

        outmsg.precision(3);
        outmsg.setf(ios::fixed);

        for (p = 1; p < m.ncolumn() - 1; p++) {
            outmsg << m(g, p) << ", ";
        }

        outmsg << m(g, p);
        if (loc == "DEBUG") {
            ASKAPLOG_DEBUG_STR(logger, outmsg.str());
        } else if (loc == "INFO") {
            ASKAPLOG_INFO_STR(logger, outmsg.str());
        }
    }
}

//**************************************************************//

bool Fitter::fit(casa::Matrix<casa::Double> pos,
                 casa::Vector<casa::Double> f,
                 casa::Vector<casa::Double> sigma)
{

    itsParams.setBoxFlux(f);
    itsSolution.resize();
    int numLoops = 3;
    itsFitter.setMaxRetries(itsParams.maxRetries());

    itsNDoF = f.size() - itsNumGauss * itsParams.numFreeParam() - 1;

    bool doFit = itsNDoF > 0;

    if (doFit) {

        for (int fitloop = 0; fitloop < numLoops; fitloop++) {
            try {
                itsSolution = itsFitter.fit(pos, f, sigma,
                                            itsParams.itsMaxRMS,
                                            itsParams.itsMaxIter,
                                            itsParams.itsCriterium);
            } catch (AipsError err) {
                std::string message = err.getMesg().chars();
                message = "FIT ERROR: " + message;
                ASKAPLOG_ERROR(logger, message);
            }

            for (unsigned int i = 0; i < itsNumGauss; i++) {
                itsSolution(i, 5) = remainder(itsSolution(i, 5), 2.*M_PI);
            }

//                         ASKAPLOG_INFO_STR(logger,  "Int. Solution #" << fitloop + 1
//                                               << ": chisq=" << itsFitter.chisquared()
//                                               << ": Parameters are:");
//                         logparameters(itsSolution);

            if (!itsFitter.converged()) {
                fitloop = numLoops;
            } else {
                if (!itsParams.negativeFluxPossible()) {
                    // If we don't allow negative fluxes, set all negative components to zero flux
                    for (uint i = 0; i < itsNumGauss; i++) {
                        if (itsSolution(i, 0) < 0) {
                            itsSolution(i, 0) = 0.;
//                                         ASKAPLOG_INFO_STR(logger, "Setting negative component #" << i + 1 << " to zero flux.");
                        }
                    }
                }

                itsFitter.setFirstEstimate(itsSolution);
            }
        }

        for (unsigned int i = 0; i < itsNumGauss; i++) {
            itsSolution(i, 5) = remainder(itsSolution(i, 5), 2.*M_PI);
        }

        itsRedChisq = itsFitter.chisquared() / float(itsNDoF);
        cout.precision(6);

        if (itsFitter.converged()) {
            ASKAPLOG_DEBUG_STR(logger, "Fit converged. Solution Parameters follow: ");
            logparameters(itsSolution);
            ASKAPLOG_DEBUG_STR(logger, "Errors on solution parameters follow: ");
            casa::Matrix<casa::Double> paramErrors = itsFitter.errors();
            logparameters(paramErrors);
        } else {
            ASKAPLOG_DEBUG_STR(logger, "Fit did not converge");
        }

        std::string result;

        if (itsFitter.converged()) {
            result = "Converged";
        } else {
            result = "Failed";
        }

        ASKAPLOG_INFO_STR(logger,
                          "Num Gaussians = " << itsNumGauss <<
                          ", " << result <<
                          ", chisq = " << itsFitter.chisquared() <<
                          ", chisq/nu =  "  << itsRedChisq <<
                          ", dof = " << itsNDoF <<
                          ", RMS = " << itsFitter.RMS());
    } else {
        ASKAPLOG_INFO_STR(logger, "Num Gaussians = " << itsNumGauss <<
                          ": Insufficient degrees of freedom (size=" << f.size() <<
                          ", nfreeParam per Gaussian=" <<  itsParams.numFreeParam() <<
                          ") - not doing fit.");
    }

    return doFit;

}
//**************************************************************//

bool Fitter::passConverged()
{
    return itsFitter.converged() && (itsFitter.chisquared() > 0.);
}

//**************************************************************//

bool Fitter::passChisq()
{
    if (!this->passConverged()) {
        return false;
    }

    if (itsParams.itsChisqConfidence > 0 && itsParams.itsChisqConfidence < 1) {
        if (itsNDoF < 343) {
            float chisqProb =
                analysisutilities::chisqProb(itsNDoF, itsFitter.chisquared());
            return chisqProb > itsParams.itsChisqConfidence;
        } else {
            return (itsRedChisq < 1.2);
        }
    } else {
        return (itsRedChisq < itsParams.itsMaxReducedChisq);
    }
}

//**************************************************************//

bool Fitter::passLocation()
{
    if (!this->passConverged()) {
        return false;
    }

    bool passXLoc = true, passYLoc = true;

    for (unsigned int i = 0; i < itsNumGauss; i++) {
        passXLoc = passXLoc && (itsSolution(i, 1) > itsParams.itsXmin) &&
                   (itsSolution(i, 1) < itsParams.itsXmax);
        passYLoc = passYLoc && (itsSolution(i, 2) > itsParams.itsYmin) &&
                   (itsSolution(i, 2) < itsParams.itsYmax);
    }

    return passXLoc && passYLoc;
}

//**************************************************************//

bool Fitter::passComponentSize()
{
    if (!this->passConverged()) {
        return false;
    }

    bool passSize = true;

    for (unsigned int i = 0; i < itsNumGauss; i++) {
        passSize = passSize && (itsSolution(i, 3) > 0.6 * itsParams.beamSize());
        passSize = passSize &&
                   ((itsSolution(i, 4) * itsSolution(i, 3)) > 0.6 * itsParams.beamSize());
        passSize = passSize && (itsSolution(i, 3) < 1.e30);
    }

    return passSize;
}

//**************************************************************//

bool Fitter::passComponentFlux()
{
    if (!this->passConverged()) {
        return false;
    }

    bool passFlux = true;

    for (unsigned int i = 0; i < itsNumGauss; i++) {
        passFlux = passFlux && (itsSolution(i, 0) > 0.);
        passFlux = passFlux && (itsSolution(i, 0) > 0.5 * itsParams.itsDetectThresh);
    }

    return passFlux;
}

//**************************************************************//

bool Fitter::passPeakFlux()
{
    if (!this->passConverged()) {
        return false;
    }

    bool passPeak = true;

    for (unsigned int i = 0; i < itsNumGauss; i++) {
        passPeak = passPeak && (itsSolution(i, 0) < 2.*itsParams.itsSrcPeak);
    }

    return passPeak;
}

//**************************************************************//

bool Fitter::passIntFlux()
{
    if (!this->passConverged()) {
        return false;
    }

    float intFlux = 0.;

    for (unsigned int i = 0; i < itsNumGauss; i++) {
        Gaussian2D<Double> component(itsSolution(i, 0), itsSolution(i, 1), itsSolution(i, 2),
                                     itsSolution(i, 3), itsSolution(i, 4), itsSolution(i, 5));
        intFlux += component.flux();
    }

    // If fitJustDetection=true, return true (ie. we don't care about the integrated flux
    // If it is false, only return true if the integrated flux is less than 2x the box flux
    return (itsParams.fitJustDetection() || (intFlux < 2.*itsParams.itsBoxFlux));
}

//**************************************************************//

bool Fitter::passSeparation()
{
    if (!this->passConverged()) {
        return false;
    }

    bool passSep = true;

    for (unsigned int i = 0; i < itsNumGauss; i++) {
        for (unsigned int j = i + 1; j < itsNumGauss; j++) {
            float sep = hypot(itsSolution(i, 1) - itsSolution(j, 1) ,
                              itsSolution(i, 2) - itsSolution(j, 2));
            passSep = passSep && (sep > 2.);
        }
    }

    return passSep;
}

//**************************************************************//

bool Fitter::acceptable()
{
    bool passConv = this->passConverged();
    bool passChisq = this->passChisq();
    bool passFlux = this->passComponentFlux();
    bool passLoc = this->passLocation();
    bool passSep = this->passSeparation();
    bool passSize = this->passComponentSize();
    bool passPeak = this->passPeakFlux();
    bool passIntFlux = this->passIntFlux();

    std::stringstream msg;
    if (!(passConv || passChisq || passFlux || passLoc ||
            passSep || passSize || passPeak || passIntFlux)) {
        msg << "Fit failed all criteria";
    } else {
        msg << "Fit failed on criteria: ";
        int ct = 0;
        if (!passConv) {
            msg << "Convergence "; ct++;
        }
        if (!passChisq) {
            msg << (ct++ > 0 ? "| " : "") << "Chisq ";
        }
        if (!passFlux) {
            msg << (ct++ > 0 ? "| " : "") << "Flux ";
        }
        if (!passLoc) {
            msg << (ct++ > 0 ? "| " : "") << "Location ";
        }
        if (!passSep) {
            msg << (ct++ > 0 ? "| " : "") << "Separation ";
        }
        // REMOVE SIZE CHECK //if(!passSize) msg << (ct++>0?"| ":"") << "Size ";
        if (!passPeak) {
            msg << (ct++ > 0 ? "| " : "") << "Peak ";
        }
        if (!passIntFlux) {
            msg << (ct++ > 0 ? "| " : "") << "Integ.Flux ";
        }
    }
//    bool thisFitGood = passConv && passChisq && passLoc && passSep && passSize && passFlux && passPeak && passIntFlux;
    bool thisFitGood = passConv && passChisq && passLoc &&
                       passSep && passFlux && passPeak && passIntFlux;
    if (!thisFitGood) {
        ASKAPLOG_INFO_STR(logger, msg.str());
    }
    return thisFitGood;
}

//**************************************************************//

std::multimap<double, int> Fitter::peakFluxList()
{
    std::multimap<double, int> fitMap;

    for (uint i = 0; i < itsNumGauss; i++) {
        fitMap.insert(std::pair<double, int>(itsSolution(i, 0), i));
    }

    return fitMap;
}

//**************************************************************//

casa::Gaussian2D<casa::Double> Fitter::gaussian(int num)
{
    casa::Gaussian2D<casa::Double>
    gauss(itsSolution(num, 0),
          itsSolution(num, 1), itsSolution(num, 2),
          itsSolution(num, 3), itsSolution(num, 4), itsSolution(num, 5));
    return gauss;
}

//**************************************************************//

}

}

}
