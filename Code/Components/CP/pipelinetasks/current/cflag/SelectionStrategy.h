/// @file SelectionStrategy.h
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

#ifndef ASKAP_CP_PIPELINETASKS_SELECTIONFLAGGER_H
#define ASKAP_CP_PIPELINETASKS_SELECTIONFLAGGER_H

// ASKAPsoft includes
#include "Common/ParameterSet.h"
#include "casa/aipstype.h"
#include "ms/MeasurementSets/MeasurementSet.h"
#include "ms/MeasurementSets/MSColumns.h"
#include "ms/MeasurementSets/MSSelection.h"

// Local package includes
#include "cflag/IFlagStrategy.h"

namespace askap {
namespace cp {
namespace pipelinetasks {

/// @brief TODO: Write documentation...
class SelectionStrategy : public IFlagStrategy {
    public:

        /// @brief Constructor
        SelectionStrategy(const LOFAR::ParameterSet& parset,
                          const casa::MeasurementSet& ms);

        virtual void processRow(casa::MSColumns& msc, const casa::uInt row);

    private:
        bool checkBaseline(casa::MSColumns& msc, const casa::uInt row);
        bool checkField(casa::MSColumns& msc, const casa::uInt row);
        bool checkTimerange(casa::MSColumns& msc, const casa::uInt row);
        bool checkScan(casa::MSColumns& msc, const casa::uInt row);
        bool checkFeed(casa::MSColumns& msc, const casa::uInt row);
        bool checkUVRange(casa::MSColumns& msc, const casa::uInt row);
        bool checkAutocorr(casa::MSColumns& msc, const casa::uInt row);

        bool checkChannel(casa::MSColumns& msc, const casa::uInt row);

        void flagRow(casa::MSColumns& msc, const casa::uInt row);

        void flagDetailed(casa::MSColumns& msc, const casa::uInt row,
                          const casa::Int chan, const casa::Int corr);

        casa::MSSelection itsSelection;

};

}
}
}

#endif
