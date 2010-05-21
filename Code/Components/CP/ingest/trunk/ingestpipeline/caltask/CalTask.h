/// @file CalTask.h
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
/// @author Ben Humphreys <ben.humphreys@csiro.au>

#ifndef ASKAP_CP_CALTASK_H
#define ASKAP_CP_CALTASK_H

// ASKAPsoft includes
#include "boost/shared_ptr.hpp"
#include "Common/ParameterSet.h"

// CASA includes
#include <casa/complex.h>
#include <casa/Arrays/Matrix.h>

// Local package includes
#include "ingestpipeline/ITask.h"
#include "ingestpipeline/datadef/VisChunk.h"

namespace askap {
namespace cp {

class CalTask : public askap::cp::ITask {
    public:
	/// @brief constructor
	/// @details Initialise calibration task by passing parameters
	/// coded in the parset
	/// @param[in] parset parameters
        explicit CalTask(const LOFAR::ParameterSet& parset);

	/// @brief destructor
        virtual ~CalTask();

	/// @brief main method to apply calibration
	/// @details It modifies the visibility chunk in situ by applying
	/// current calibration
	/// @param[in] chunk shared pointer to visibility chunk
        virtual void process(VisChunk::ShPtr chunk);

    protected:
        /// @brief obtain gain for a given antenna/beam/pol
	/// @details This is a helper method to separate extraction of
	/// the values from the parset file from the actual calculation.
	/// Based on the given number of antenna, beam and polarisation
	/// (X or Y coded as 0 and 1), it returns the value of the 
	/// parallel-hand gain (i.e. Gxx or Gyy)
	/// @param[in] ant 0-based antenna id
	/// @param[in] beam 0-based beam id
	/// @param[in] pol Either 0 for XX or 1 for YY
	casa::Complex getGain(casa::uInt ant, casa::uInt beam, casa::uInt pol) const;

        /// @brief helper method to load complex parameter
	/// @details It reads the value from itsGainsParset and
	/// forms a complex number.
	/// @param[in] name parameter name
	/// @return complex number
	casa::Complex readComplex(const std::string &name) const;

	/// @brief fill Mueller matrix
	/// @details This method forms the measurement equation
	/// defined by Mueller matrix for a given baseline and beams.
	/// The method is implemented in a general way, so it supports
	/// correlations corresponding to a different beam. However, in
	/// practice beam1 and beam2 are likely to be the same most of 
	/// the time.
	/// @param[in] matr Mueler matrix to fill (must already be resized to 4x4)
	/// @param[in] ant1 first antenna id (0-based)
	/// @param[in] ant2 second antenna id (0-based)
	/// @param[in] beam1 beam id at the first antenna (0-based)
	/// @param[in] beam2 beam id at the second antenna (0-based)
	void fillMuellerMatrix(casa::Matrix<casa::Complex> &matr, casa::uInt ant1, 
	         casa::uInt ant2, casa::uInt beam1, casa::uInt beam2) const;

    private:
	/// @brief parset file with configuration parameters
        const LOFAR::ParameterSet itsParset;

	/// @brief parset file with gains
	LOFAR::ParameterSet itsGainsParset;
};

}
}

#endif
