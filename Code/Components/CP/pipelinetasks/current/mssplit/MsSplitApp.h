/// @file MsSplitApp.h
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

#ifndef ASKAP_CP_MSSPLITAPP_H
#define ASKAP_CP_MSSPLITAPP_H

// Package level header file
#include "askap_pipelinetasks.h"

// System includes
#include <string>
#include <set>
#include <utility>
#include <stdint.h>

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "askap/AskapLogging.h"
#include "askap/Application.h"
#include "askap/AskapUtil.h"
#include "boost/shared_ptr.hpp"
#include "Common/ParameterSet.h"
#include "casa/aips.h"
#include "ms/MeasurementSets/MeasurementSet.h"

namespace askap {
namespace cp {
namespace pipelinetasks {

class MsSplitApp : public askap::Application {
    public:
        virtual int run(int argc, char* argv[]);

    private:

        static boost::shared_ptr<casa::MeasurementSet> create(
            const std::string& filename, casa::uInt bucketSize,
            casa::uInt tileNcorr, casa::uInt tileNchan);

        static void copyAntenna(const casa::MeasurementSet& source, casa::MeasurementSet& dest);

        static void copyDataDescription(const casa::MeasurementSet& source, casa::MeasurementSet& dest);

        static void copyFeed(const casa::MeasurementSet& source, casa::MeasurementSet& dest);

        static void copyField(const casa::MeasurementSet& source, casa::MeasurementSet& dest);

        static void copyObservation(const casa::MeasurementSet& source, casa::MeasurementSet& dest);

        static void copyPointing(const casa::MeasurementSet& source, casa::MeasurementSet& dest);

        static void copyPolarization(const casa::MeasurementSet& source, casa::MeasurementSet& dest);

        static void splitSpectralWindow(const casa::MeasurementSet& source,
                                 casa::MeasurementSet& dest,
                                 const uint32_t startChan,
                                 const uint32_t endChan,
                                 const uint32_t width);

        void splitMainTable(const casa::MeasurementSet& source,
                            casa::MeasurementSet& dest,
                            const uint32_t startChan,
                            const uint32_t endChan,
                            const uint32_t width);

        int split(const std::string& invis, const std::string& outvis,
                  const uint32_t startChan,
                  const uint32_t endChan,
                  const uint32_t width,
                  const LOFAR::ParameterSet& parset);

        // Returns true if row filtering is enabled, otherwise false.
        bool rowFiltersExist() const;

        // Returns true if the the row should be filtered (i.e excluded), otherwise
        // true.
        bool rowIsFiltered(uint32_t scanid, uint32_t feed1, uint32_t feed2) const;

        /// Set of beam IDs to include in the new measurement set, or empty
        /// if all beams are to be included
        std::set<uint32_t> itsBeams;

        /// Set of scan IDs to include in the new measurement set, or empty
        /// if all scans are to be included
        std::set<uint32_t> itsScans;
};

}
}
}
#endif
