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

namespace askap {

    namespace analysis {

	class RMSynthesis
	{
	public:
	    RMSynthesis(const LOFAR::ParameterSet &parset);
	    RMSynthesis(const RMSynthesis& other);
	    RMSynthesis& operator= (const RMSynthesis& other);
	    virtual ~RMSynthesis();

	    void initialise(PolarisationData &poldata);


	private:

	    PolarisationData itsPolData;
	    casa::Array<float> itsWeights;
	    
	    int itsNumPhiChan;
	    float itsDeltaPhi;
	    float itsPhiRef;
	    casa::Array<float> itsPhi;
	    
	    casa::Array<casa::Complex> itsFaradayDF;

	    casa::Array<float> itsPhiDouble;

	    casa::Array<casa::Complex> itsRMSF;

	    float itsRMSFwidth;
	    
	    float itsRefLambdaSquared;


	};

    }

}


#endif
