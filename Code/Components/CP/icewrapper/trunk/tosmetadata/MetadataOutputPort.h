/// @file MetadataOutputPort.h
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

// System includes
#include <string>

// ASKAPsoft includes
#include "Ice/Ice.h"
#include "boost/scoped_ptr.hpp"

// CP ice interfaces
#include "TypedValues.h"

// Local package includes
#include "iceutils/OutputPort.h"

#ifndef ASKAP_CP_METADATAOUTPUTPORT_H
#define ASKAP_CP_METADATAOUTPUTPORT_H

namespace askap {
    namespace cp {

        class MetadataOutputPort
        {
            public:
                MetadataOutputPort(const std::string& locatorHost,
                        const std::string& locatorPort,
                        const std::string& topicManager,
                        const std::string& topic);

                ~MetadataOutputPort();

                void send(const askap::interfaces::TimeTaggedTypedValueMap& message);

            private:
                typedef OutputPort<askap::interfaces::TimeTaggedTypedValueMap,
                        askap::interfaces::datapublisher::ITimeTaggedTypedValueMapPublisherPrx>
                            OutputPortType;

                boost::scoped_ptr<OutputPortType> itsOutputPort;

                askap::interfaces::datapublisher::ITimeTaggedTypedValueMapPublisherPrx itsProxy;
        };

    };
};

#endif
