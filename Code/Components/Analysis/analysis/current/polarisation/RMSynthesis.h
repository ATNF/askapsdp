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

/* #include <polarisation/PolarisationData.h> */

#include <casa/Arrays/Vector.h>
#include <casa/Complex.h>

#include <Common/ParameterSet.h>

namespace askap {

    namespace analysis {

	class RMSynthesis
	{
	public:
	    RMSynthesis(const LOFAR::ParameterSet &parset);
	    RMSynthesis(const RMSynthesis& other);
	    RMSynthesis& operator= (const RMSynthesis& other);
	    virtual ~RMSynthesis(){};

// **not yet implemented fully**
//	    void calculate(PolarisationData &poldata);
	    void calculate(casa::Vector<float> &lsq, casa::Vector<float> &q, casa::Vector<float> &u, casa::Vector<float> &noise);

	    void fitRMSF();

	    std::string weightType(){return itsWeightType;};
	    unsigned int numPhiChan(){return itsNumPhiChan;};
	    float deltaPhi(){return itsDeltaPhi;};

	    casa::Vector<casa::Complex> fdf(){return itsFaradayDF;};
	    casa::Vector<float> phi(){return itsPhi;};
	    casa::Vector<casa::Complex> rmsf(){return itsRMSF;};
	    casa::Vector<float> phi_rmsf(){return itsPhiForRMSF;};
	    float rmsf_width(){return itsRMSFwidth;};

	    float normalisation(){return itsNormalisation;};

	private:

	    /// @brief Initialise phi and weights based on parset
	    void defineVectors();

	    casa::Vector<float> itsWeights;
	    std::string itsWeightType;
	    float itsNormalisation;
	    
	    unsigned int itsNumPhiChan;
	    float itsDeltaPhi;
	    float itsPhiZero;
	    casa::Vector<float> itsPhi;
	    
	    casa::Vector<casa::Complex> itsFaradayDF;

	    casa::Vector<float> itsPhiDouble;

	    casa::Vector<float> itsPhiForRMSF;
	    casa::Vector<casa::Complex> itsRMSF;

	    float itsRMSFwidth;
	    
	    float itsRefLambdaSquared;
	    float itsRefFrequency;


	};

    }

}


#endif
