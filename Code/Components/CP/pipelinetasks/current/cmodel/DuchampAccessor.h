/// @file DuchampAccessor.h
///
/// @copyright (c) 2011 CSIRO
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

#ifndef ASKAP_CP_PIPELINETASKS_DUCHAMPACCESSOR_H
#define ASKAP_CP_PIPELINETASKS_DUCHAMPACCESSOR_H

// System includes
#include <string>
#include <fstream>

// ASKAPsoft includes
#include "casa/aipstype.h"
#include "components/ComponentModels/ComponentList.h"
#include "components/ComponentModels/SkyComponent.h"

// Local package includes
#include "cmodel/IGlobalSkyModel.h"

namespace askap {
namespace cp {
namespace pipelinetasks {

/// @brief An object providing access to a sky model contained in a duchamp
/// output ASCII text file.
class DuchampAccessor : IGlobalSkyModel {
    public:
        // Constructor
        DuchampAccessor(const std::string& filename);

        // Destructor
        ~DuchampAccessor();

        // Conesearch (or filter)
        virtual casa::ComponentList coneSearch(const casa::Double ra,
                const casa::Double dec,
                const casa::Double searchRadius);

    private:
        casa::SkyComponent createComponent(const std::string& line);

        std::ifstream itsFile;
};

}
}
}

#endif
