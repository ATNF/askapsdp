/// @file MSSink.h
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

#ifndef ASKAP_CP_MSSINK_H
#define ASKAP_CP_MSSINK_H

// ASKAPsoft includes
#include "boost/shared_ptr.hpp"
#include "boost/scoped_ptr.hpp"
#include "Common/ParameterSet.h"
#include "ms/MeasurementSets/MeasurementSet.h"
#include "casa/Quanta.h"

// Local package includes
#include "ingestpipeline/ITask.h"
#include "ingestpipeline/datadef/VisChunk.h"
#include "ingestutils/IConfiguration.h"

namespace askap {
namespace cp {

/// @brief A sink task for the central processor ingest pipeline which writes
/// the data out to a measurement set.
class MSSink : public askap::cp::ITask {
    public:
        /// @brief Constructor.
        /// @param[in] parset   the parameter set used to configure this task.
        MSSink(const LOFAR::ParameterSet& parset);

        /// @brief Destructor.
        virtual ~MSSink();

        /// @brief Writes out the data in the VisChunk parameter to the 
        /// measurement set.
        ///
        /// @param[in,out] chunk    the instance of VisChunk to write out. Note
        ///                         the VisChunk pointed to by "chunk" nor the pointer
        ///                         itself are modified by this function.
        virtual void process(VisChunk::ShPtr chunk);

    private:
        // Initialises the ANTENNA table
        void initAntennas(void);

        // Initialises the FEED table
        void initFeeds(void);

        // Initialises the  SPECTRAL WINDOW table
        void initSpws(void);

        // Initialises the FIELDS table
        void initFields(void);

        // Initialises the OBSERVATION table
        void initObs(void);

        // Create the measurement set
        void create(void);

        // Parameter set
        const LOFAR::ParameterSet itsParset;

        // Configuration wrapper (around parset)
        boost::scoped_ptr<IConfiguration> itsConfig;

        // Measurement set
        boost::scoped_ptr<casa::MeasurementSet> itsMs;
};

}
}

#endif
