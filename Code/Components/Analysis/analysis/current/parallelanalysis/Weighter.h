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
/// @author Matthew Whiting <Matthew.Whiting@csiro.au>
///
#ifndef ASKAP_ANALYSIS_WEIGHTER_H_
#define ASKAP_ANALYSIS_WEIGHTER_H_

#include <askapparallel/AskapParallel.h>
#include <casainterface/CasaInterface.h>
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
	    Weighter(askap::askapparallel::AskapParallel& comms, const LOFAR::ParameterSet &parset);
	    Weighter(const Weighter& other);
	    Weighter& operator= (const Weighter& other);
	    virtual ~Weighter(){};
	
	    void initialise(duchamp::Cube &cube, bool doAllocation=true);
	    float weight(size_t i);
	    void search();
	    void applyCutoff();
	    float cutoff(){return itsWeightCutoff;};

	    bool fileOK(){return (itsImage!="");};
	    bool doApplyCutoff(){return fileOK() && (itsWeightCutoff>0.);};
	    bool isValid(size_t i);
	    bool doScaling(){return fileOK() && itsFlagDoScaling;};
	   
	    bool isValid(){return fileOK() && (doScaling() || doApplyCutoff());};

	protected:
	    void findNorm();
	    void readWeights();

	    askap::askapparallel::AskapParallel *itsComms;
	    std::string itsImage;
	    duchamp::Section itsSection;
	    duchamp::Cube *itsCube;
	    float itsNorm;
	    float itsWeightCutoff;
	    std::string itsCutoffType;
	    bool itsFlagDoScaling;
	    casa::Array<casa::Float> itsWeights;

	};

    }
}


#endif
