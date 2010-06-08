/// @file ObsService.h
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

#ifndef ASKAP_CP_OBSSERVICE_H
#define ASKAP_CP_OBSSERVICE_H

// ASKAPsoft includes
#include "Ice/Ice.h"

// ICE Interface includes
#include "CP.h"

namespace askap {
namespace cp {

/// @brief This class implements the Central Processor observation service.
class ObsService : public askap::interfaces::cp::ICPObsService {
    public:
        /// @brief Constructor.
        ObsService(const Ice::CommunicatorPtr ic);

        /// @brief Destructor.
        virtual ~ObsService();

        // Ice "IComponent" interfaces
        void startObs(const long sbid, const askap::interfaces::ParameterMap& parmap,
                      const Ice::Current& cur);
        void abortObs(const Ice::Current& cur);

    private:
        // ICE communicator
        Ice::CommunicatorPtr itsComm;

        // No support for assignment
        ObsService& operator=(const ObsService& rhs);

        // No support for copy constructor
        ObsService(const ObsService& src);
};

}
}

#endif
