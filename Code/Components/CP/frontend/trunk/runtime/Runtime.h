/// @file Runtime.h
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

#ifndef ASKAP_CP_RUNTIME_H
#define ASKAP_CP_RUNTIME_H

// ASKAPsoft includes
#include "Ice/Ice.h"
#include "APS/ParameterSet.h"
#include "boost/shared_ptr.hpp"
#include "runtime/Frontend.h"
#include "runtime/Workflow.h"

namespace askap {
    namespace cp {

        class Runtime : public askap::cp::frontend::IFrontend
        {
            public:
                /// @brief Constructor.
                Runtime(const LOFAR::ACC::APS::ParameterSet& parset);

                /// @brief Destructor.
                virtual ~Runtime();

                void run(void);
                Ice::CommunicatorPtr initIce(const LOFAR::ACC::APS::ParameterSet& parset);
                Ice::ObjectAdapterPtr createAdapter(const LOFAR::ACC::APS::ParameterSet& parset,
                        Ice::CommunicatorPtr& ic);

                // Ice "Frontend" interfaces
                void startWorkflow(const askap::cp::frontend::WorkflowDesc& wfDesc, const Ice::Current& cur);
                void stopWorkflow(const Ice::Current& cur);
                void shutdown(const Ice::Current& cur);

            private:
                LOFAR::ACC::APS::ParameterSet itsParset;

                Ice::CommunicatorPtr itsComm;
                Ice::ObjectAdapterPtr itsAdapter;

                boost::shared_ptr<Workflow> itsWorkflow;

                // No support for assignment
                Runtime& operator=(const Runtime& rhs);

                // No support for copy constructor
                Runtime(const Runtime& src);
        };
    };
};

#endif
