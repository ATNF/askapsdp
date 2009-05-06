/// @file SolverWorker.h
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

#ifndef ASKAP_CP_SOLVERWORKER_H
#define ASKAP_CP_SOLVERWORKER_H

// System includes
#include <string>

// ASKAPsoft includes
#include <APS/ParameterSet.h>
#include <fitting/INormalEquations.h>
#include <fitting/Params.h>

// Local includes
#include "distributedimager/ISolverTask.h"
#include "distributedimager/MPIBasicComms.h"
#include "distributedimager/SolverTaskComms.h"

namespace askap {
    namespace cp {

        class SolverWorker : public ISolverTask
        {
            public:
                SolverWorker(LOFAR::ACC::APS::ParameterSet& parset,
                        askap::cp::MPIBasicComms& comms,
                        askap::scimath::Params::ShPtr model_p);

                virtual ~SolverWorker();

                virtual void solveNE(askap::scimath::INormalEquations::ShPtr);

                virtual void writeModel(const std::string& postfix);

            private:
                // No support for assignment
                SolverWorker& operator=(const SolverWorker& rhs);

                // No support for copy constructor
                SolverWorker(const SolverWorker& src);

                // Parameter set
                LOFAR::ACC::APS::ParameterSet& itsParset;

                // Communications class
                askap::cp::SolverTaskComms itsComms;
        };

    };
};

#endif
