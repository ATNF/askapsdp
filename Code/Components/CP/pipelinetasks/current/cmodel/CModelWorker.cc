/// @file CModelWorker.cc
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

// Include own header file first
#include "cmodel/CModelWorker.h"

// Include package level header file
#include "askap_pipelinetasks.h"

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "boost/scoped_ptr.hpp"
#include "Common/ParameterSet.h"

// Casacore includes
#include "casa/aipstype.h"
#include "images/Images/TempImage.h"
#include "images/Images/PagedImage.h"

// Local package includes
#include "cmodel/MPIBasicComms.h"
#include "cmodel/CasaComponentImager.h"
#include "cmodel/ImageFactory.h"

// Using
using namespace askap;
using namespace askap::cp::pipelinetasks;
using namespace casa;

ASKAP_LOGGER(logger, ".CModelWorker");

CModelWorker::CModelWorker(MPIBasicComms& comms)
    : itsComms(comms)
{
}

void CModelWorker::run(void)
{
    // Parameter set
    LOFAR::ParameterSet parset;

    // Obtain the parset via broadcast
    itsComms.broadcastParset(parset, 0);

    // Create a TempImage and the component imager
    casa::TempImage<casa::Float> image = ImageFactory::createTempImage(parset);
    CasaComponentImager imager(parset);

    // Signal master ready, receive and image components until the master
    // signals completion by sending an empty list
    std::vector<askap::cp::skymodelservice::Component> list;
    do {
        itsComms.signalReady(0);
        list = itsComms.receiveComponents(0);

        ASKAPLOG_DEBUG_STR(logger, "Imaging list of " << list.size() << " components");
        imager.projectComponents(list, image);
    } while (!list.empty());

    ASKAPLOG_DEBUG_STR(logger, "Beginning reduction");
    itsComms.sumImages(image, 0);
    ASKAPLOG_DEBUG_STR(logger, "Reduction complete");

}
