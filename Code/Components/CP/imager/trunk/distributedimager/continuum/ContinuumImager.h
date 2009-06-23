/// @file ContinuumImager.h
///
/// @copyright (c) 2009 CSIRO
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

#ifndef ASKAP_CP_CONTINUUMIMAGER_H
#define ASKAP_CP_CONTINUUMIMAGER_H

// ASKAPsoft includes
#include <APS/ParameterSet.h>
#include <fitting/Params.h>

// Local package includes
#include "distributedimager/common/MPIBasicComms.h"

namespace askap {
    namespace cp {

        /// @brief Main class for the Distributed imager.
        class ContinuumImager
        {
            public:
                /// @brief Construct a Distributed Imager.
                /// 
                /// @param[in]  parset  the parameter set containing
                ///                     the configuration.
                /// @param[in]  comms   an instance of IBasicComms.
                ContinuumImager(LOFAR::ACC::APS::ParameterSet& parset,
                        askap::cp::MPIBasicComms& comms);

                /// @brief Destructor.
                ~ContinuumImager();

                /// @brief Run the distrbuted imager.
                void run(void);

            private:

                // Returns true if the caller is the master process,
                // else false.
                bool isMaster(void);

                // Id of the master process
                static const int itsMaster = 0;

                // Parameter set
                LOFAR::ACC::APS::ParameterSet& itsParset;

                // Communications class
                askap::cp::MPIBasicComms& itsComms;

                // Model
                askap::scimath::Params::ShPtr itsModel;

                // No support for assignment
                ContinuumImager& operator=(const ContinuumImager& rhs);

                // No support for copy constructor
                ContinuumImager(const ContinuumImager& src);
        };

    };
};

#endif
