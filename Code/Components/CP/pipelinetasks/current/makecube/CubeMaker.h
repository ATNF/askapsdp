/// @file CubeMaker.h
///
/// Class to run the creation of a new cube
///
/// @copyright (c) 2013 CSIRO
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
#ifndef ASKAP_CP_PIPELINETASKS_CUBEMAKER_H
#define ASKAP_CP_PIPELINETASKS_CUBEMAKER_H

#include <Common/ParameterSet.h>
#include <casa/Arrays/IPosition.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <images/Images/PagedImage.h>
#include <casa/Quanta/Unit.h>
#include <vector>
#include <string>

namespace askap {
    namespace cp {
	namespace pipelinetasks {

	    class CubeMaker
	    {
	    public:
		CubeMaker(const LOFAR::ParameterSet &parset);
		virtual ~CubeMaker();

		void initialise();
		void createCube();
		void setImageInfo();
		void writeSlices();

	    protected:
		void getReferenceData();
		void getSecondCoordinates();
		void setRestFreq(casa::CoordinateSystem &csys);
		bool writeSlice(size_t i);

		std::string itsInputNamePattern;
		std::vector<std::string> itsInputNames;
		double itsRestFrequency;
		
		std::string itsBeamReference;
		size_t itsBeamImageNum;

		int itsNumChan;
		casa::IPosition itsRefShape;
		casa::CoordinateSystem itsRefCoordinates;
		casa::CoordinateSystem itsSecondCoordinates;
		casa::Unit itsRefUnits;

		std::string itsCubeName;
		casa::PagedImage<float> *itsCube;
	    };


	}
    }
}




#endif

