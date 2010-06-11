/// @file SpectralLineImager.cc
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

// Include own header file first
#include "SpectralLineImager.h"

// Include package level header file
#include <askap_imager.h>

// System includes
#include <string>

// ASKAPsoft includes
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <Common/ParameterSet.h>

// Local includes
#include "distributedimager/common/IBasicComms.h"
#include "distributedimager/spectralline/SpectralLineMaster.h"
#include "distributedimager/spectralline/SpectralLineWorker.h"

ASKAP_LOGGER(logger, ".SpectralLineImager");

using namespace askap::cp;
using namespace askap;

SpectralLineImager::SpectralLineImager(LOFAR::ParameterSet& parset,
        askap::cp::MPIBasicComms& comms) : itsParset(parset), itsComms(comms)
{
    if (isMaster()) {
        ASKAPLOG_INFO_STR(logger, "ASKAP Distributed Spectral Line Imager - " << ASKAP_PACKAGE_VERSION);
    }
}

SpectralLineImager::~SpectralLineImager()
{
}

void SpectralLineImager::run(void)
{
    if (isMaster()) {
        SpectralLineMaster master(itsParset, itsComms);
        master.run();
    } else {
        SpectralLineWorker worker(itsParset, itsComms);
        worker.run();
    }
}

bool SpectralLineImager::isMaster(void)
{
    return (itsComms.getId() == itsMaster) ? true : false;
}
