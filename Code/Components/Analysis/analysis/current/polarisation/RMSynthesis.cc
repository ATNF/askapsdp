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
#include <complex>
#include <casa/BasicSL/Complex.h>
#include <scimath/Fitting/FitGaussian.h>

#include <Common/ParameterSet.h>

///@brief Where the log messages go.
ASKAP_LOGGER(logger, ".rmsynthesis");

namespace askap {

    namespace analysis {


	RMSynthesis::RMSynthesis(const LOFAR::ParameterSet &parset)
	{
	    /// @details Initialises the Farady Depth array (phi) according to the parset specification (which gives the number of phi channels, their spacing and the centre RM.

	    this->itsNumPhiChan = parset.getUint("numPhiChan",0);
	    ASKAPASSERT(this->itsNumPhiChan>0);
	    this->itsDeltaPhi = parset.getFloat("deltaPhi",0.);
	    ASKAPASSERT(this->itsDeltaPhi>0.);
	    this->itsPhiZero = parset.getFloat("phiZero",0.);

	    std::string defaultWeight="variance";
	    this->itsWeightType = parset.getString("weightType",defaultWeight);
	    if(this->itsWeightType != "uniform" && this->itsWeightType != "variance"){
		ASKAPLOG_WARN_STR(logger, "RMSynthesis: weightType must be either 'uniform' or 'variance' (you have " << this->itsWeightType<<"). Setting to " << defaultWeight);
		this->itsWeightType = defaultWeight;
	    }

	    this->defineVectors();
	}

	void RMSynthesis::defineVectors()
	{
	    casa::IPosition shape(1,this->itsNumPhiChan);
	    this->itsPhi = casa::Vector<float>(shape,0.);
	    casa::indgen<float>(this->itsPhi, -0.5*this->itsNumPhiChan*this->itsDeltaPhi, this->itsDeltaPhi);
//	    ASKAPLOG_DEBUG_STR(logger, "Phi array = " << this->itsPhi);
	    casa::IPosition bigshape(1,2*this->itsNumPhiChan);
	    this->itsPhiForRMSF = casa::Vector<float>(bigshape,0.);
	    casa::indgen<float>(this->itsPhiForRMSF, -1.*this->itsNumPhiChan*this->itsDeltaPhi, this->itsDeltaPhi);

	    this->itsFaradayDF = casa::Vector<casa::Complex>(shape,0.);

	    this->itsRMSF = casa::Vector<casa::Complex>(bigshape,0.);

	    
	}

	RMSynthesis::RMSynthesis(const RMSynthesis& other)
	{
	    this->operator=(other);
	}

	RMSynthesis& RMSynthesis::operator= (const RMSynthesis& other)
	{
	    if(this==&other) return *this;
	    this->itsWeights = other.itsWeights;
	    this->itsWeightType = other.itsWeightType;
	    this->itsNormalisation = other.itsNormalisation;
	    this->itsNumPhiChan = other.itsNumPhiChan;
	    this->itsDeltaPhi = other.itsDeltaPhi;
	    this->itsPhiZero = other.itsPhiZero;
	    this->itsPhi = other.itsPhi;
	    this->itsFaradayDF = other.itsFaradayDF;
	    this->itsFDFnoise = other.itsFDFnoise;
	    this->itsPhiDouble = other.itsPhiDouble;
	    this->itsPhiForRMSF = other.itsPhiForRMSF;
	    this->itsRMSF = other.itsRMSF;
	    this->itsRMSFwidth = other.itsRMSFwidth;
	    this->itsRefLambdaSquared = other.itsRefLambdaSquared;
	    return *this;

	}

// ** PolarisationData not yet implemented fully **
	// void RMSynthesis::calculate(PolarisationData &poldata)
	// {
	//     /// @details Takes the PolarisationData object, which
	//     /// contains the I,Q,U spectra and the QU noise spectrum,
	//     /// and the lambda-squared array, and calls the main
	//     /// calculate function on those arrays to perform RM
	//     /// synthesis.

	//     casa::Vector<float> lsq = poldata.l2();
	//     // q = Q/Imod, u = U/Imod, p = q + iu

	//     casa::Vector<float> q=poldata.Q().spectrum() / poldata.Imod();
	//     casa::Vector<float> u=poldata.U().spectrum() / poldata.Imod();
	    
	//     this->calculate(lsq,q,u,poldata.noise());

	// }

	void RMSynthesis::calculate(casa::Vector<float> &lsq, casa::Vector<float> &q, casa::Vector<float> &u, casa::Vector<float> &noise)
	{
	    /// @details Takes the lambda-squared array and
	    /// corresponding Q &U spectra and QU noise spectrum, and
	    /// defines the weights, the normalisation and the
	    /// reference lambda-squared value. It then performs RM
	    /// Synthesis, creating the FDF and RMSF arrays. Also
	    /// calls the fitRMSF function to obtain the FWHM of the
	    /// main RMSF lobe.

	    // p = q + iu
	    casa::Vector<casa::Complex> p = casa::makeComplex(q,u);

	    if(this->itsWeightType == "variance")
		this->itsWeights = casa::pow(noise,-2.);
	    else
		this->itsWeights = casa::Vector<float>(noise.size(),1.);

	    this->itsFDFnoise = casa::mean(noise)/sqrt(noise.size());

	    // K = \sum(w_i)^-1
	    this->itsNormalisation = 1./casa::sum(this->itsWeights);

	    // \lambda^2_0 = K * \sum(w_i*\lambda^2_i)
	    this->itsRefLambdaSquared = this->itsNormalisation * casa::sum(this->itsWeights * lsq);

	    // Compute FDF
	    for(size_t j=0;j<this->itsNumPhiChan;j++){
		casa::Vector<float> phase = -2.F* this->itsPhi[j]*(lsq-this->itsRefLambdaSquared);
		casa::Vector<casa::Complex> sampling = casa::makeComplex(this->itsWeights * cos(phase),this->itsWeights * sin(phase));
		this->itsFaradayDF[j] = this->itsNormalisation * casa::sum( p * sampling );
	    }

	    // Compute RMSF
	    for(size_t j=0;j<2.*this->itsNumPhiChan;j++){
		casa::Vector<float> phase = -2.F* this->itsPhiForRMSF[j]*(lsq-this->itsRefLambdaSquared);
		casa::Vector<casa::Complex> sampling = casa::makeComplex(this->itsWeights * cos(phase),this->itsWeights * sin(phase));
		this->itsRMSF[j] = this->itsNormalisation * casa::sum(sampling);
	    }

	    this->fitRMSF();

	}

	void RMSynthesis::fitRMSF()
	{
// ** not yet implemented **

	    // find extent of peak of RMSF by starting at peak and finding where slope changes
	    //   - ie. go left, find where slope become negative. go right, find where slope become positive
	    //
	    // to that range alone, fit a Gaussian - fitGaussian should be fine.
	    //
	    // report the FWHM of the fitted Gaussian

	    casa::Vector<float> rmsf_p = casa::amplitude(this->itsRMSF);
	    float minRMSF,maxRMSF;
	    casa::IPosition locMin,locMax;
	    casa::minMax<float>(minRMSF,maxRMSF,locMin,locMax,rmsf_p);

	    // move left
	    float ref=maxRMSF, val=maxRMSF;
	    int limitLower=locMax[0];
	    ASKAPLOG_DEBUG_STR(logger, "lower:  " << ref << " " << val << " " << limitLower);
	    do{
		ref=val;
		val=rmsf_p[--limitLower];
		ASKAPLOG_DEBUG_STR(logger, "lower:  " << ref << " " << val << " " << limitLower);
	    } while((ref-val)>0);
	    limitLower++;
	    val=maxRMSF;
	    int limitUpper=locMax[0];
	    ASKAPLOG_DEBUG_STR(logger, "upper:  " << ref << " " << val << " " << limitUpper);
	    do{
		ref=val;
		val=rmsf_p[++limitUpper];
		ASKAPLOG_DEBUG_STR(logger, "upper:  " << ref << " " << val << " " << limitUpper);
	    } while((ref-val)>0);
	    limitUpper--;

	    ASKAPLOG_DEBUG_STR(logger, "Fitting to peak of RMSF between phi channels " << limitLower << " and " << limitUpper);

	    int size=limitUpper-limitLower+1;
	    casa::Matrix<casa::Double> pos(size,1);
	    casa::Vector<casa::Double> f(size);
	    casa::Vector<casa::Double> sigma(size);

	    for(int i=limitLower; i<=limitUpper;i++) {
		pos.row(i-limitLower) = this->itsPhiForRMSF[i];
		f(i-limitLower) = rmsf_p[i];
		sigma(i-limitLower) = 1.;
	    }

	    ASKAPLOG_DEBUG_STR(logger, "RMSF fit: pos = " << pos);
	    ASKAPLOG_DEBUG_STR(logger, "RMSF fit: f = " << f);

	    casa::FitGaussian<casa::Double> fitter;
	    fitter.setDimensions(1);
	    fitter.setNumGaussians(1);
	    casa::Matrix<casa::Double> estimate;
	    estimate.resize(1,3);
	    estimate(0,0) = 1.;
	    estimate(0,1) = this->itsNumPhiChan-limitLower;
	    estimate(0,2) = this->itsDeltaPhi;
	    fitter.setFirstEstimate(estimate);
	    casa::Matrix<casa::Double> solution= fitter.fit(pos, f, sigma);
		
	    if(!fitter.converged())
		ASKAPLOG_WARN_STR(logger, "RMSF fit did not converge!");
	    else{
		this->itsRMSFwidth = solution(0,2);
	    }
	

	}

    }

}
