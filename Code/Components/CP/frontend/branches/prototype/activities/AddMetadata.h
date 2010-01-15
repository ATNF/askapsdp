/// @file AddMetadata.h
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

#ifndef ASKAP_CP_ADDMETADATA_H
#define ASKAP_CP_ADDMETADATA_H

// System includes
#include <string>

// ASKAPsoft includes
#include "Ice/Ice.h"
#include "Common/ParameterSet.h"

// Local package includes
#include "activities/Activity.h"
#include "activities/InputPort.h"
#include "activities/OutputPort.h"
#include "streams/Visibilities.h"
#include "streams/Metadata.h"

namespace askap {
    namespace cp {

        class AddMetadata : public Activity
        {
            public:

                /// @brief Constructor.
                AddMetadata(const Ice::CommunicatorPtr ic,
                        const Ice::ObjectAdapterPtr adapter,
                        const LOFAR::ParameterSet& parset);

                /// @brief Destructor.
                virtual ~AddMetadata();

                virtual void attachInputPort(int port, const std::string& topic);
                virtual void attachOutputPort(int port, const std::string& topic);

                virtual void detachInputPort(int port);
                virtual void detachOutputPort(int port);

            protected:
                void run(void);

            private:
                // Ice Communicator
                const Ice::CommunicatorPtr itsComm;

                // Parameters
                LOFAR::ParameterSet itsParset;

                // Input Ports
                askap::cp::InputPort<askap::cp::frontend::Metadata,
                    askap::cp::frontend::IMetadataStream> itsInPort0;

                askap::cp::InputPort<askap::cp::frontend::Visibilities,
                    askap::cp::frontend::IVisStream> itsInPort1;

                // Output Ports
                askap::cp::OutputPort<askap::cp::frontend::Visibilities,
                    askap::cp::frontend::IVisStreamPrx> itsOutPort0;
        };
    };
};

#endif
