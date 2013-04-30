/// @file
///
/// Control class to run the calculation of the variable (box) threshold
///
/// @copyright (c) 2011 CSIRO
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
#ifndef ASKAP_ANALYSIS_VAR_THRESH_H_
#define ASKAP_ANALYSIS_VAR_THRESH_H_
#include <Common/ParameterSet.h>
#include <duchamp/Cubes/cubes.hh>
#include <string>

namespace askap {

    namespace analysis {

	class VariableThresholder
	{
	public:
	    VariableThresholder(){};
	    VariableThresholder(const LOFAR::ParameterSet &parset);
	    VariableThresholder(const VariableThresholder& other);
	    VariableThresholder& operator= (const VariableThresholder& other);
	    virtual ~VariableThresholder(){};

	    void initialise(duchamp::Cube &cube);
	    void threshold();
	    void search();

	    
	protected:
	    void fixName(std::string name, bool flag, std::string suffix);

	    /// Should we use robust (ie. median-based) statistics
	    bool itsFlagRobustStats;
	    float itsSNRthreshold;
	    std::string itsSearchType;
	    /// The half-box-width used for the sliding-box calculations
	    int itsBoxSize;

	    std::string itsInputImage;

	    /// Whether to write a casa image containing the S/N ratio
	    bool itsFlagWriteSNRimage;
	    /// Name of S/N image to be written
	    std::string itsSNRimageName;
		
	    /// Whether to write a casa image containing the Detection threshold
	    bool itsFlagWriteThresholdImage;
	    /// Name of Threshold image to be written
	    std::string itsThresholdImageName;

	    /// Whether to write a casa image containing the image noise
	    bool itsFlagWriteNoiseImage;
	    /// Name of Noise image to be written
	    std::string itsNoiseImageName;

	    duchamp::Cube *itsCube;

	};

    }

}




#endif
