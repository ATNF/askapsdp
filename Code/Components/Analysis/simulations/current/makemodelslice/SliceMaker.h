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
#ifndef ASKAP_SIMULATIONS_SLICEMAKER_H_
#define ASKAP_SIMULATIONS_SLICEMAKER_H_

#include <vector>
#include <string>

#include <analysisparallel/SubimageDef.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <casa/Quanta/Unit.h>
#include <casa/Arrays/IPosition.h>
#include <images/Images/PagedImage.h>
#include <boost/scoped_ptr.hpp>
#include <Common/ParameterSet.h>


namespace askap {

    namespace simulations {



	class SliceMaker
	{
	public:
	    SliceMaker(const LOFAR::ParameterSet& parset);
	    virtual ~SliceMaker(){};

	    void initialise();  // verify chunk list and set up coordsys
	    void createSlice(); // create the output image
	    const void writeChunks(); // write the slice of each individual chunk

	protected:

	    analysisutilities::SubimageDef itsSubimageDef;
	    std::vector<std::string> itsChunkList;
	    unsigned int itsNumChunks;
	    std::string itsModelName;
	    std::string itsSliceName;

	    boost::scoped_ptr< casa::PagedImage<float> > itsSlice;
	    casa::IPosition itsSliceShape;
	    std::vector<int> itsNpix;
	    int itsNchan;
	    unsigned int itsLngAxis;
	    unsigned int itsLatAxis;
	    unsigned int itsSpcAxis;
	    

	    casa::CoordinateSystem itsRefCoordinates;
	    casa::Unit itsRefUnits;
	    casa::IPosition itsRefShape;
	    std::vector<int> itsChanRange;


	};

    }
}

#endif
