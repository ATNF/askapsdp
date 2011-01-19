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
#ifndef ASKAP_ANALYSIS_WEIGHTER_H_
#define ASKAP_ANALYSIS_WEIGHTER_H_

#include <askapparallel/AskapParallel.h>
#include <analysisutilities/CasaImageUtil.h>
#include <duchamp/Utils/Section.hh>
#include <casa/aipstype.h>
#include <casa/Arrays/Vector.h>

#include <vector>
#include <string>

namespace askap {
    namespace analysis {

      /// @brief A simple class to get the relative weight of a given pixel

      class Weighter
      {

      public:
	Weighter(askap::mwbase::AskapParallel& comms);
	virtual ~Weighter(){};
	
	void initialise(std::string &weightsImage, duchamp::Section &section);
	void findNorm();
	void readWeights();
	float weight(int i){return sqrt(itsWeights(i)/itsNorm);};

      protected:
	askap::mwbase::AskapParallel& itsComms;
	std::string itsImage;
	duchamp::Section itsSection;
	float itsNorm;
	casa::Vector<casa::Double> itsWeights;

      };

    }
}


#endif
