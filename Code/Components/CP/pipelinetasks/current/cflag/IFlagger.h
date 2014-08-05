/// @file IFlagger.h
///
/// @copyright (c) 2012 CSIRO
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

#ifndef ASKAP_CP_PIPELINETASKS_IFLAGGER_H
#define ASKAP_CP_PIPELINETASKS_IFLAGGER_H

// ASKAPsoft includes
#include "ms/MeasurementSets/MSColumns.h"
#include "casa/aipstype.h"

// Local package includes
#include "cflag/FlaggingStats.h"

namespace askap {
namespace cp {
namespace pipelinetasks {

/// @brief An interface for classes that perform flagging on a per row basis.
class IFlagger {
    public:

        /// Destructor
        virtual ~IFlagger();

        /// Perform flagging (if necessary) for the row with index "row".
        ///
        /// @param[in,out] msc  the masurement set columns that contain the data
        ///                     and flagging arrays
        /// @param[in] row      the (zero-based) index number for the row in
        ///                     msc to be processed.
        /// @param[in] dryRun   if true the measurement set will not be modified,
        ///                     however statistics will be calculated indicating
        ///                     what flagging would have been done.
        virtual void processRow(casa::MSColumns& msc, const casa::uInt pass,
                                const casa::uInt row, const bool dryRun) = 0;

        /// Returns flagging statistics
        virtual FlaggingStats stats(void) const = 0;

        /// Functions associated with multiple passees
        /// @param[in] pass     number of passes over the data already performed
        virtual casa::Bool processingRequired(const casa::uInt pass) = 0;

};

}
}
}

#endif
