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
#include <duchamp/PixelMap/Object2D.hh>
#include <casa/Arrays/Slicer.h>
#include <casa/Arrays/Vector.h>

namespace askap {
	
	namespace analysis {
		
		namespace sourcefitting {
		
			class SubThresholder {
			public:
//				SubThresholder(){itsFluxArray=0; itsDim=0;};
				SubThresholder(){};
				virtual ~SubThresholder();
				SubThresholder(const SubThresholder &s);
				SubThresholder& operator=(const SubThresholder &s);
				
//				void define(RadioSource *r, float *array=0);
				void define(RadioSource *r, casa::Vector<casa::Double> &array);
				void define(RadioSource *r, casa::Vector<float> &array);
				void define(RadioSource *r);
				std::vector<SubComponent> find();
				
//				void saveArray(float *array, std::vector<long> &dim);
				void saveArray(casa::Vector<casa::Double> &array);
				void saveArray(casa::Vector<float> &array);
				//								void saveArray(float *array, long *dim);
				void keepObject(PixelInfo::Object2D &obj);

				
			protected:
				SubComponent itsFirstGuess;
//				float *itsFluxArray;
//				long *itsDim;
				casa::Vector<float> itsFluxArray;
				casa::Vector<long> itsDim;
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
