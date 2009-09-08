/// @file AdminInterface.h
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

#ifndef ASKAP_CP_ADMININTERFACE_H
#define ASKAP_CP_ADMININTERFACE_H

// ASKAPsoft includes
#include "Ice/Ice.h"

// Interface includes
#include "manager/Component.h"

namespace askap {
    namespace cp {
        class AdminInterface : public askap::interfaces::component::IComponent
        {
            public:
                /// @brief Constructor.
                AdminInterface(const Ice::CommunicatorPtr ic);

                /// @brief Destructor.
                virtual ~AdminInterface();

                void run(void);
                Ice::ObjectAdapterPtr createAdapter(void);

                // Ice "IComponent" interfaces
                void startup(const askap::interfaces::ParameterMap& params, const Ice::Current& cur);
                void shutdown(const Ice::Current& cur);
                void activate(const Ice::Current& cur);
                void deactivate(const Ice::Current& cur);
                askap::interfaces::component::ComponentTestResultSeq selfTest(const Ice::Current& cur);
                askap::interfaces::component::ComponentState getState(const Ice::Current& cur);

            private:
                Ice::CommunicatorPtr itsComm;

                Ice::ObjectAdapterPtr itsAdapter;

                askap::interfaces::component::ComponentState itsState;

                // No support for assignment
                AdminInterface& operator=(const AdminInterface& rhs);

                // No support for copy constructor
                AdminInterface(const AdminInterface& src);
        };
    };
};

#endif
