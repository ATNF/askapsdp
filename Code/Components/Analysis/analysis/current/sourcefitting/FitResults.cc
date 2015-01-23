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

#include <sourcefitting/FitResults.h>
#include <sourcefitting/Fitter.h>
#include <sourcefitting/Component.h>
#include <analysisutilities/AnalysisUtilities.h>

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

void FitResults::saveResults(Fitter &fit)
{

    itsFitIsGood = true;
    itsFlagFitIsGuess = false;
    itsChisq = fit.chisq();
    itsRedChisq = fit.redChisq();
    itsRMS = fit.RMS();
    itsNumDegOfFreedom = fit.ndof();
    itsNumFreeParam = fit.params().numFreeParam();
    itsNumGauss = fit.numGauss();
    itsNumPix = itsNumDegOfFreedom + itsNumGauss * itsNumFreeParam + 1;
    // Make a map so that we can output the fitted components in order of peak flux
    std::multimap<double, int> fitMap = fit.peakFluxList();
    // Need to use reverse_iterator so that brightest component's listed first
    std::multimap<double, int>::reverse_iterator rfit = fitMap.rbegin();

    for (; rfit != fitMap.rend(); rfit++) {
        itsGaussFitSet.push_back(fit.gaussian(rfit->second));
    }
}
//**************************************************************//

void FitResults::saveGuess(std::vector<SubComponent> cmpntList)
{

    itsFitIsGood = false;
    itsFlagFitIsGuess = true;
    itsChisq = 999.;
    itsRedChisq = 999.;
    itsRMS = 0.;
    itsNumDegOfFreedom = 0;
    itsNumFreeParam = 0;
    itsNumGauss = cmpntList.size();
    itsNumPix = 0;
    // Make a map so that we can output the fitted components in order of peak flux
    std::multimap<double, int> fitMap;
    for (unsigned int i = 0; i < itsNumGauss; i++) {
        fitMap.insert(std::pair<double, int>(cmpntList[i].peak(), i));
    }
    // Need to use reverse_iterator so that brightest component's listed first
    std::multimap<double, int>::reverse_iterator rfit = fitMap.rbegin();

    for (; rfit != fitMap.rend(); rfit++) {
        itsGaussFitSet.push_back(cmpntList[rfit->second].asGauss());
    }
}

//**************************************************************//

std::vector<SubComponent> FitResults::getCmpntList()
{
    std::vector<SubComponent> output(itsGaussFitSet.size());
    std::vector<casa::Gaussian2D<Double> >::iterator gauss = itsGaussFitSet.begin();
    int comp = 0;

    for (; gauss < itsGaussFitSet.end(); gauss++) {
        output[comp].setX(gauss->xCenter());
        output[comp].setY(gauss->yCenter());
        output[comp].setPeak(gauss->height());
        output[comp].setMajor(gauss->majorAxis());
        output[comp].setMinor(gauss->minorAxis());
        output[comp].setPA(gauss->PA());
        comp++;
    }

    return output;
}


void FitResults::logIt(std::string loc)
{
    std::vector<casa::Gaussian2D<Double> >::iterator gauss;
    for (gauss = itsGaussFitSet.begin(); gauss < itsGaussFitSet.end(); gauss++) {
        std::stringstream outmsg;
        outmsg << "Component Flux,X0,Y0,MAJ,MIN,PA = ";
        outmsg.precision(8);
        outmsg.setf(ios::fixed);
        outmsg << gauss->height() << ", ";
        outmsg.precision(3);
        outmsg.setf(ios::fixed);
        outmsg << gauss->xCenter() << ", " << gauss->yCenter() << ", "
               << gauss->majorAxis() << ", " << gauss->minorAxis() << ", " << gauss->PA();
        if (loc == "DEBUG") {
            ASKAPLOG_DEBUG_STR(logger, outmsg.str());
        } else if (loc == "INFO") {
            ASKAPLOG_INFO_STR(logger, outmsg.str());
        }

    }
}



}

}

}
