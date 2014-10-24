/// @file
///
/// Handle the parameterisation of objects that require reading from a file on disk
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
#ifndef ASKAP_OBJECT_PARAMER_H_
#define ASKAP_OBJECT_PARAMER_H_

#include <askapparallel/AskapParallel.h>
#include <parallelanalysis/DuchampParallel.h>
#include <duchamp/Detection/Detection.hh>
#include <vector>

namespace askap {

    namespace analysis {

	class ObjectParameteriser
	{
	public:
	    ObjectParameteriser(askap::askapparallel::AskapParallel& comms);
	    ObjectParameteriser(const ObjectParameteriser& other);
	    ObjectParameteriser& operator= (const ObjectParameteriser& other);
	    virtual ~ObjectParameteriser();

	    /// @brief Initialise
	    void initialise(DuchampParallel *dp);

	    /// @brief Master sends list to workers, who fill out itsInputList
	    void distribute(); 
	    void parameterise();
	    void gather();

	protected:

	    askap::askapparallel::AskapParallel *itsComms;
	    DuchampParallel *itsDP;
	    std::vector<sourcefitting::RadioSource> itsInputList;
	    std::vector<sourcefitting::RadioSource> itsOutputList;

	};

    }

}


#endif
