/// @file
///
/// XXX Notes on program XXX
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
/// @author XXX XXX <XXX.XXX@csiro.au>
///
#ifndef ASKAP_ANALYSIS_EXTRACTION_MOMENT_MAP_H_
#define ASKAP_ANALYSIS_EXTRACTION_MOMENT_MAP_H_
#include <askap_analysis.h>
#include <extraction/SourceDataExtractor.h>
#include <Common/ParameterSet.h>

namespace askap {

    namespace analysis { 


	class MomentMapExtractor : public SourceDataExtractor
	{
	public:
	    MomentMapExtractor(){};
	    MomentMapExtractor(const MomentMapExtractor& other);
	    MomentMapExtractor(const LOFAR::ParameterSet& parset);
	    MomentMapExtractor& operator= (const MomentMapExtractor& other);
	    virtual ~MomentMapExtractor(){};

	    void extract();
	    void writeImage();

	protected:
	    void defineSlicer();
	    void initialiseArray();

	    /// What sort of cutout to do - full field or box around the source?
	    std::string itsSpatialMethod;
	    
	    /// For the box method, how many pixels to pad around the source?
	    unsigned int itsPadSize;

	};

    }

}

#endif
