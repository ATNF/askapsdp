/// @file
///
/// Contains the calls to the fitting routines, as well as parameters such as number of Gaussians and box widths
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

#ifndef ASKAP_ANALYSIS_FITRESULTS_H_
#define ASKAP_ANALYSIS_FITRESULTS_H_

#include <sourcefitting/Fitter.h>
#include <sourcefitting/Component.h>

#include <scimath/Fitting/FitGaussian.h>
#include <scimath/Functionals/Gaussian2D.h>

#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>

#include <duchamp/fitsHeader.hh>

#include <casa/namespace.h>

#include <Common/ParameterSet.h>

#include <map>
#include <vector>
#include <utility>


namespace askap {

    namespace analysis {

        namespace sourcefitting {

            /// @brief A class to store the results of source fitting.
            /// @details This class contains all information needed to
            /// describe the results of an attempt at gaussian fitting. It records
            /// whether the fit worked, the basic fit parameters and the descriptions
            /// of the fitted components. This is able to be passed over a Blob
            /// stream.
            class FitResults {
                public:
                    /// @brief Default constructor: bad fit, 0 components
                    FitResults() {itsFitIsGood = false; itsNumGauss = 0;}
                    /// @brief Destructor
                    virtual ~FitResults() {};
                    /// @brief Copy constructor
                    FitResults(const FitResults& f);
                    /// @brief Copy function
                    FitResults& operator= (const FitResults& f);

                    /// @brief Store the results from a fit attempt
                    void saveResults(Fitter &fit);

                    /// @brief Whether the fit is good or not
                    bool  isGood() {return itsFitIsGood;};
                    /// @brief The chi-squared value of the fit;
                    float chisq() {return itsChisq;};
                    /// @brief The reduced chi-squared value of the fit: chi-squared divided by the number of degrees of freedom
                    float redchisq() {return itsRedChisq;};
                    /// @brief The RMS deviation for the fit
                    float RMS() {return itsRMS;};
                    /// @brief The number of degrees of freedom for the fit
                    int   ndof() {return itsNumDegOfFreedom;};
                    /// @brief The number of free parameters for the fit
                    int   numFreeParam() {return itsNumFreeParam;};
                    /// @brief The number of Gaussian components used in the fit
                    int   numGauss() {return itsNumGauss;};
                    /// @brief Return the set of Gaussian fits
                    std::vector<casa::Gaussian2D<double> > fitSet() {return itsGaussFitSet;};
                    /// @brief Return a reference to the set of Gaussian fits.
                    std::vector<casa::Gaussian2D<Double> >& fits() {
                        std::vector<casa::Gaussian2D<Double> >& rfit = itsGaussFitSet; return rfit;
                    };
                    /// @brief Return the set of Gaussian fits in SubComponent format
                    std::vector<SubComponent> getCmpntList();

                    /// @brief Return a given Gaussian from the FitSet
                    casa::Gaussian2D<Double> gaussian(int num) {return itsGaussFitSet[num];};

                    /// @brief The number of fitted components
                    int numFits() {return itsGaussFitSet.size();};

                    /// @brief Functions allowing fitting parameters to be passed over LOFAR Blobs
                    /// @name
                    /// @{

                    /// @brief Pass Fitting parameters into a Blob
                    friend LOFAR::BlobOStream& operator<<(LOFAR::BlobOStream &stream, FitResults& par);
                    /// @brief Receive Fitting parameters from a Blob
                    friend LOFAR::BlobIStream& operator>>(LOFAR::BlobIStream &stream, FitResults& par);

                    /// @}


                protected:
                    /// @brief Is the fit good?
                    bool  itsFitIsGood;
                    /// @brief The chi-squared value
                    float itsChisq;
                    /// @brief The reduced chi-squared value
                    float itsRedChisq;
                    /// @brief The RMS of the fit
                    float itsRMS;
                    /// @brief The number of degrees of freedom in the fit
                    int   itsNumDegOfFreedom;
                    /// @brief The number of free parameters in the fit
                    int   itsNumFreeParam;
                    /// @brief the number of Gaussians used in the fit
                    int   itsNumGauss;
                    /// @brief A two-dimensional Gaussian fit to the object.
                    std::vector<casa::Gaussian2D<Double> > itsGaussFitSet;

            };


        }

    }

}

#endif

