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

            FitResults::FitResults(const FitResults& f)
            {
                operator=(f);
            }

            //**************************************************************//

            FitResults& FitResults::operator= (const FitResults& f)
            {
                if (this == &f) return *this;

                this->itsFitIsGood = f.itsFitIsGood;
                this->itsChisq = f.itsChisq;
                this->itsRedChisq = f.itsRedChisq;
                this->itsRMS = f.itsRMS;
                this->itsNumDegOfFreedom = f.itsNumDegOfFreedom;
                this->itsNumFreeParam = f.itsNumFreeParam;
                this->itsNumGauss = f.itsNumGauss;
                this->itsGaussFitSet = f.itsGaussFitSet;
                return *this;
            }

            //**************************************************************//

            void FitResults::saveResults(Fitter &fit)
            {
                this->itsFitIsGood = true;
                this->itsChisq = fit.chisq();
                this->itsRedChisq = fit.redChisq();
                this->itsRMS = fit.RMS();
                this->itsNumDegOfFreedom = fit.ndof();
                this->itsNumFreeParam = fit.params().numFreeParam();
                this->itsNumGauss = fit.numGauss();
                // Make a map so that we can output the fitted components in order of peak flux
                std::multimap<double, int> fitMap = fit.peakFluxList();
                // Need to use reverse_iterator so that brightest component's listed first
                std::multimap<double, int>::reverse_iterator rfit = fitMap.rbegin();

                for (; rfit != fitMap.rend(); rfit++)
                    this->itsGaussFitSet.push_back(fit.gaussian(rfit->second));
            }

            //**************************************************************//

	  std::vector<SubComponent> FitResults::getCmpntList()
	   {
	     std::vector<SubComponent> output(this->itsGaussFitSet.size());
	     std::vector<casa::Gaussian2D<Double> >::iterator gauss = this->itsGaussFitSet.begin();
	     int comp=0;
	     for(;gauss<this->itsGaussFitSet.end();gauss++){
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


        }

    }

}
