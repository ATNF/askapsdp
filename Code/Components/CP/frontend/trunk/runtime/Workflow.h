/// @file Workflow.h
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

#ifndef ASKAP_CP_WORKFLOW_H
#define ASKAP_CP_WORKFLOW_H

// ASKAPsoft includes
#include "Ice/Ice.h"
#include "APS/ParameterSet.h"

namespace askap {
    namespace cp {

        class Workflow
        {
            public:
                /// @brief Constructor.
                Workflow(const Ice::CommunicatorPtr& ic,
                        const Ice::ObjectAdapterPtr& adapter,
                        const LOFAR::ACC::APS::ParameterSet& parset,
                        const std::string& runtimeName);

                /// @brief Destructor.
                virtual ~Workflow();

                virtual void start(void);
                virtual void stop(void);

            private:
                // Ice Communicator
                Ice::CommunicatorPtr itsComm;

                // Ice Adapter
                Ice::ObjectAdapterPtr itsAdapter;

                // Parameter set describing the workflow
                LOFAR::ACC::APS::ParameterSet itsParset;

                // Node name, used to determine which parts of workflow
                // within this runtime
                const std::string itsRuntimeName;

                // No support for assignment
                Workflow& operator=(const Workflow& rhs);

                // No support for copy constructor
                Workflow(const Workflow& src);
        };
    };
};

#endif
