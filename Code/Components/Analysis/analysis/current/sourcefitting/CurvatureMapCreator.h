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
#ifndef ASKAP_ANALYSIS_CURVATURE_MAP_H_
#define ASKAP_ANALYSIS_CURVATURE_MAP_H_
#include <askapparallel/AskapParallel.h>
#include <Common/ParameterSet.h>
#include <string>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/IPosition.h>
#include <outputs/ImageWriter.h>
#include <analysisparallel/SubimageDef.h>

#include <duchamp/Cubes/cubes.hh>

namespace askap {

    namespace analysis {

	class CurvatureMapCreator
	{
	public:
	    CurvatureMapCreator(){};
	    CurvatureMapCreator(askap::askapparallel::AskapParallel &comms, const LOFAR::ParameterSet &parset);
	    CurvatureMapCreator(const CurvatureMapCreator& other);
	    CurvatureMapCreator& operator= (const CurvatureMapCreator& other);
	    virtual ~CurvatureMapCreator(){};

	    void setCube(duchamp::Cube &cube){itsCube = &cube;};
	    void initialise(duchamp::Cube &cube, analysisutilities::SubimageDef &subdef);
	    void calculate();
	    void maskBorders();
	    void write();

	    float sigmaCurv(){return itsSigmaCurv;};

	protected:
	    void findSigma();

	    askap::askapparallel::AskapParallel *itsComms;
	    LOFAR::ParameterSet itsParset;
	    duchamp::Cube *itsCube;
	    analysisutilities::SubimageDef *itsSubimageDef;
	    std::string itsFilename;
	    casa::Array<float> itsArray;
	    casa::IPosition itsShape;
	    casa::IPosition itsLocation;
	    float itsSigmaCurv;
	};

    }

}

#endif
