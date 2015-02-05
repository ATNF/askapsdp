/// @file
///
/// Perform Rotation Measure Synthesis and parameterise
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
#ifndef ASKAP_ANALYSIS_RM_SYNTHESIS_H_
#define ASKAP_ANALYSIS_RM_SYNTHESIS_H_

// #include <polarisation/PolarisationData.h>

#include <casa/Arrays/Vector.h>
#include <casa/BasicSL/Complex.h>

#include <Common/ParameterSet.h>

namespace askap {

namespace analysis {

/// The default type of weighting, if not specified in the parset.
const std::string defaultWeight = "variance";

class RMSynthesis {
    public:
        /// @details Initialises the Farady Depth arrays (phi,
        /// phiForRMSF) according to the parset specification (which
        /// gives the number of phi channels, their spacing and the
        /// centre RM), and sets the FDF and RMSF arrays to zero.
        explicit RMSynthesis(const LOFAR::ParameterSet &parset);
        virtual ~RMSynthesis() {};

// **not yet implemented fully**
//     /// @details Takes the PolarisationData object, which
//     /// contains the I,Q,U spectra and the QU noise spectrum,
//     /// and the lambda-squared array, and calls the main
//     /// calculate function on those arrays to perform RM
//     /// synthesis.
//      void calculate(PolarisationData &poldata);

        /// @details Takes the lambda-squared array and corresponding
        /// Q &U spectra and QU noise spectrum, and defines the
        /// weights, the normalisation and the reference
        /// lambda-squared value. It then performs RM Synthesis,
        /// creating the FDF and RMSF arrays. Also calls the fitRMSF
        /// function to obtain the FWHM of the main RMSF lobe.
        void calculate(const casa::Vector<float> &lsq,
                       const casa::Vector<float> &q,
                       const casa::Vector<float> &u,
                       const casa::Vector<float> &noise);

        /// Fit to the RM Spread Function. Find extent of peak of RMSF
        /// by starting at peak and finding where slope changes -
        /// ie. go left, find where slope become negative. go right,
        /// find where slope become positive
        ///
        /// To that range alone, fit a Gaussian - fitGaussian should be
        /// fine.
        ///
        /// Records the FWHM of the fitted Gaussian
        void fitRMSF();

        const std::string weightType() {return itsWeightType;};
        const unsigned int numPhiChan() {return itsNumPhiChan;};
        const float deltaPhi() {return itsDeltaPhi;};

        const casa::Vector<casa::Complex> &fdf() {return itsFaradayDF;};
        const casa::Vector<float> &phi() {return itsPhi;};
        const casa::Vector<casa::Complex> &rmsf() {return itsRMSF;};
        const casa::Vector<float> &phi_rmsf() {return itsPhiForRMSF;};
        const float rmsf_width() {return itsRMSFwidth;};
        const float refLambdaSq() {return itsRefLambdaSquared;};

        const float normalisation() {return itsNormalisation;};
        const float fdf_noise() {return itsFDFnoise;};

        const unsigned int numFreqChan() {return itsWeights.size();};
        const float lsqVariance() {return itsLambdaSquaredVariance;};

    private:

        /// @brief Initialise phi and weights based on parset
        void defineVectors();

        casa::Vector<float> itsWeights;
        std::string itsWeightType;
        float itsNormalisation;
        float itsLambdaSquaredVariance;

        unsigned int itsNumPhiChan;
        float itsDeltaPhi;
        float itsPhiZero;
        casa::Vector<float> itsPhi;

        casa::Vector<casa::Complex> itsFaradayDF;

/// The average of the provided noise spectrum, scaled by sqrt(num_freq_chan)
        float itsFDFnoise;

        casa::Vector<float> itsPhiDouble;

        casa::Vector<float> itsPhiForRMSF;
        casa::Vector<casa::Complex> itsRMSF;

        float itsRMSFwidth;

        float itsRefLambdaSquared;


};

}

}


#endif
