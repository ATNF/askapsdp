/// @file
///
/// XXX Notes on program XXX
///
/// @copyright (c) 2010 CSIRO
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
#ifndef ASKAP_ANALYSIS_SUBTHRESH_H
#define ASKAP_ANALYSIS_SUBTHRESH_H

#include <askap_analysis.h>
#include <sourcefitting/RadioSource.h>
#include <sourcefitting/Component.h>

#include <duchamp/Cubes/cubes.hh>
#include <casa/Arrays/Slicer.h>

namespace askap {
	
	namespace analysis {
		
		namespace sourcefitting {
		
			class SubThresholder {
			public:
				SubThresholder(){};
				virtual ~SubThresholder(){};
				SubThresholder(const SubThresholder &s);
				SubThresholder& operator=(const SubThresholder &s);
				
				void define(RadioSource &r, float *array=0);
				std::vector<SubComponent> find();
				
				void saveArray(float *array, long *dim);
				
				
			protected:
				SubComponent itsFirstGuess;
				float *itsFluxArray;
				duchamp::Image *itsImage;
				long *itsDim;
				casa::Slicer itsSourceBox;
				int itsNumThresholds;
				float itsBaseThreshold;
				float itsThreshIncrement;
				float itsPeakFlux;
				int itsSourceSize;
				
				};
		
		}
		
	}
	
}


#endif
