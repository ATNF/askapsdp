/// @file SpectralLineWorker.h
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

#ifndef ASKAP_CP_SPECTRALLINEWORKER_H
#define ASKAP_CP_SPECTRALLINEWORKER_H

// System includes

// ASKAPsoft includes
#include <Common/ParameterSet.h>
#include <fitting/INormalEquations.h>
#include <fitting/Params.h>
#include <dataaccess/TableDataSource.h>
#include <gridding/IVisGridder.h>

// Local includes
#include "distributedimager/common/IBasicComms.h"
#include "messages/SpectralLineWorkUnit.h"

namespace askap {
    namespace cp {

        class SpectralLineWorker
        {
            public:
                SpectralLineWorker(LOFAR::ParameterSet& parset,
                        askap::cp::IBasicComms& comms);
                ~SpectralLineWorker();

                void run(void);


            private:
                // Process a workunit
                void processWorkUnit(const SpectralLineWorkUnit& wu);

                // For a given workunit, just process a single channel
                void processChannel(askap::synthesis::TableDataSource& ds,
                        const std::string& imagename, unsigned int localChannel,
                        unsigned int globalChannel);

                // Setup the image specified in itsParset and add it to the Params instance.
                void setupImage(const askap::scimath::Params::ShPtr& params, int actualChannel);

                // Parameter set
                LOFAR::ParameterSet& itsParset;

                // Communications class
                askap::cp::IBasicComms& itsComms;

                // Pointer to the gridder
                askap::synthesis::IVisGridder::ShPtr itsGridder_p;

                // No support for assignment
                SpectralLineWorker& operator=(const SpectralLineWorker& rhs);

                // No support for copy constructor
                SpectralLineWorker(const SpectralLineWorker& src);

                // ID of the master process
                static const int itsMaster = 0;
        };

    };
};

#endif
