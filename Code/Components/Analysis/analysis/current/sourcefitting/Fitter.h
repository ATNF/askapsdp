/// @file
///
/// Contains the calls to the fitting routines
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

#ifndef ASKAP_ANALYSIS_FITTER_H_
#define ASKAP_ANALYSIS_FITTER_H_

#include <sourcefitting/FittingParameters.h>
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
            class FittingParameters; // foreshadowing

            /// @ingroup sourcefitting
            /// @brief A class to manage the 2D profile fitting.
            /// @details The class handles the calling of the fitting
            /// functions, and stores the results using the casa::FitGaussian class
            /// and a casa::Matrix with the best fit. The FittingParameters class
            /// holds the relevant parameters.
            class Fitter {
                public:
                    /// @brief Default constructor
                    Fitter() {};
                    /// @brief Default destructor
                    virtual ~Fitter() {};
                    /// @brief Copy constructor
                    Fitter(const Fitter& f);
                    /// @brief Copy function
                    Fitter& operator= (const Fitter& f);

                    /// @brief Set and return the set of fitting parameters
                    /// @{
                    void setParams(FittingParameters p) {itsParams = p;};
                    FittingParameters params() {return itsParams;};
                    FittingParameters &rparams() {FittingParameters& rfitpars = itsParams; return rfitpars;};
                    ///@}

                    /// @brief Set and return the number of Gaussian components to be fitted.
                    /// @{
                    void setNumGauss(int i) {itsNumGauss = i;};
                    int  numGauss() {return itsNumGauss;};
                    /// @}

                    /// @brief Return the chi-squared value from the fit.
                    float chisq() {return itsFitter.chisquared();};
                    /// @brief Return the reduced chi-squared value from the fit.
                    float redChisq() {return itsRedChisq;};
                    /// @brief Return the RMS of the fit
                    float RMS() {return itsFitter.RMS();};
                    /// @brief Return the number of degrees of freedom of the fit.
                    int ndof() {return itsNDoF;};

                    /// @brief Set the intial estimates for the Gaussian components.
                    void setEstimates(std::vector<SubComponent> cmpntList, duchamp::FitsHeader &head);
                    /// @brief Set the retry factors
                    void setRetries();
                    /// @brief Set the mask values
                    void setMasks();
                    /// @brief Fit components to the data
                    void fit(casa::Matrix<casa::Double> pos, casa::Vector<casa::Double> f,
                             casa::Vector<casa::Double> sigma);

                    /// @brief Functions to test the fit according to various criteria.
                    /// @{

                    /// @brief Has the fit converged?
                    bool passConverged();
                    /// @brief Does the fit have an acceptable chi-squared value?
                    bool passChisq();
                    /// @brief Are the fitted components suitably within the box?
                    bool passLocation();
                    /// @brief Are the component sizes big enough?
                    bool passComponentSize();
                    /// @brief Are the component fluxes OK?
                    bool passComponentFlux();
                    bool passPeakFlux();
                    bool passIntFlux();
                    bool passSeparation();
                    /// @brief Is the fit acceptable overall?
                    bool acceptable();
                    /// @}

                    /// @brief Return an ordered list of peak fluxes
                    std::multimap<double, int> peakFluxList();

                    /// @brief Return a casa::Gaussian2D version of a particular component.
                    casa::Gaussian2D<casa::Double> gaussian(int num);


                protected:
                    /// @brief The set of parameters defining the fits
                    FittingParameters itsParams;

                    /// @brief The number of Gaussian functions to fit.
                    unsigned int itsNumGauss;
                    /// @brief The casa Gaussian Fitter
                    FitGaussian<casa::Double> itsFitter;
                    /// @brief The number of degrees of freedom in the fit
                    int itsNDoF;
                    /// @brief The reduced chi-squared of the fit
                    float itsRedChisq;

                    /// @brief The fitted components
                    casa::Matrix<casa::Double> itsSolution;

            };


        }

    }

}

#endif

