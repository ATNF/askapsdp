/// @file MockMetadataSource.h
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

#ifndef ASKAP_CP_MOCKMETADATASOURCE_H
#define ASKAP_CP_MOCKMETADATASOURCE_H

// ASKAPsoft includes
#include "boost/shared_ptr.hpp"

// CP Ice interfaces
#include "CommonTypes.h"
#include "TypedValues.h"

// Local package includes
#include "ingestpipeline/sourcetask/IMetadataSource.h"
#include "ingestpipeline/sourcetask/test/DequeWrapper.h"

namespace askap {
    namespace cp {

        class MockMetadataSource : public askap::cp::IMetadataSource
        {
            public:
                MockMetadataSource();
                ~MockMetadataSource();

                void add(boost::shared_ptr<askap::interfaces::TimeTaggedTypedValueMap> obj);

                boost::shared_ptr<askap::interfaces::TimeTaggedTypedValueMap> next(void);

                // Shared pointer definition
                typedef boost::shared_ptr<MockMetadataSource> ShPtr;

            private:
                DequeWrapper< askap::interfaces::TimeTaggedTypedValueMap > itsBuffer;
        };

    };
};

#endif
