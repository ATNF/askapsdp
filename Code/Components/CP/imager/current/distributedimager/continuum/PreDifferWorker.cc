/// @file PreDifferWorker.cc
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
#include "PreDifferWorker.h"

// System includes
#include <string>
#include <sstream>
#include <stdexcept>

// ASKAPsoft includes
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <Common/ParameterSet.h>
#include <fitting/Equation.h>
#include <fitting/INormalEquations.h>
#include <fitting/ImagingNormalEquations.h>
#include <fitting/Params.h>
#include <gridding/IVisGridder.h>
#include <gridding/VisGridderFactory.h>
#include <measurementequation/ImageFFTEquation.h>
#include <dataaccess/TableConstDataSource.h>
#include <dataaccess/TableDataSource.h>
#include <dataaccess/IDataIterator.h>
#include <dataaccess/IDataIterator.h>
#include <dataaccess/SharedIter.h>
#include <dataaccess/ParsetInterface.h>
#include <casa/OS/Timer.h>

// Local package includes
#include "distributedimager/common/IBasicComms.h"
#include "distributedimager/continuum/ReductionLogic.h"
#include "messages/UpdateModel.h"
#include "messages/PreDifferResponse.h"
#include "messages/PreDifferRequest.h"

using namespace askap::cp;
using namespace askap::scimath;
using namespace askap::synthesis;

ASKAP_LOGGER(logger, ".PreDifferWorker");

PreDifferWorker::PreDifferWorker(LOFAR::ParameterSet& parset,
        askap::cp::IBasicComms& comms)
: itsParset(parset), itsComms(comms)
{
    itsGridder_p = VisGridderFactory::make(itsParset);
}

PreDifferWorker::~PreDifferWorker()
{
    itsGridder_p.reset();
}

askap::scimath::INormalEquations::ShPtr PreDifferWorker::calcNE(askap::scimath::Params::ShPtr notused)
{
    // Assert PreConditions
    ASKAPCHECK(itsGridder_p, "itsGridder_p is not correctly initialized");

    // Receive the model
    UpdateModel updatemsg;
    itsComms.receiveMessageBroadcast(updatemsg, itsMaster);
    askap::scimath::Params::ShPtr model_p = updatemsg.get_model();


    // Pointer to measurement equation
    askap::scimath::Equation::ShPtr equation_p;

    // Normal equations which will be accumulated into
    // until all workunits have been received by this worker.
    itsNormalEquation_p = ImagingNormalEquations::ShPtr(new ImagingNormalEquations(*model_p));

    // This count tracks how many work units have had their results
    // merged into the normal equations pointed to be itsNormalEquation_p
    int count = 0;

    while (1) {
        // Ask the master for a workunit
        PreDifferResponse response;
        response.set_payloadType(PreDifferResponse::READY);
        itsComms.sendMessage(response, itsMaster);

        // Receive the workunit from the master
        PreDifferRequest request;
        itsComms.receiveMessage(request, itsMaster);

        if (request.get_payloadType() == PreDifferRequest::FINALIZE) {
            // Indicates all workunits have been assigned already
            break;
        }

        const std::string ms = request.get_dataset();
        ASKAPLOG_INFO_STR(logger, "Calculating normal equations for " << ms );

        casa::Timer timer;
        timer.mark();

        // Setup the DataSource
        bool useMemoryBuffers = itsParset.getBool("memorybuffers", false);
        if (useMemoryBuffers) {
            ASKAPLOG_INFO_STR(logger, "Scratch data will be held in memory" );
        } else {
            ASKAPLOG_INFO_STR(logger, "Scratch data will be written to the subtable of the original dataset" );
        }

        std::string colName = itsParset.getString("datacolumn", "DATA");
        TableDataSource ds(ms, (useMemoryBuffers ? TableDataSource::MEMORY_BUFFERS : TableDataSource::DEFAULT),
                colName);
        IDataSelectorPtr sel = ds.createSelector();
        sel << itsParset;
        IDataConverterPtr conv = ds.createConverter();
        conv->setFrequencyFrame(casa::MFrequency::Ref(casa::MFrequency::TOPO),
                "Hz");
        conv->setDirectionFrame(casa::MDirection::Ref(casa::MDirection::J2000));
        IDataSharedIter it = ds.createIterator(sel, conv);

        // Calc NE
        ASKAPCHECK(model_p, "model_p is not correctly initialized");
        ASKAPCHECK(itsNormalEquation_p, "ne_p is not correctly initialized");

        equation_p = askap::scimath::Equation::ShPtr(new ImageFFTEquation(*model_p, it, itsGridder_p));
        ASKAPCHECK(equation_p, "equation_p is not correctly initialized");
        equation_p->calcEquations(*itsNormalEquation_p);

        ASKAPLOG_DEBUG_STR(logger, "Calculated normal equations for "<< ms << " in "
                << timer.real() << " seconds ");

        equation_p.reset();

        count++;
    }

    // Even if the count is zero, must report the empty object
    reduceNE(itsNormalEquation_p, count);

    // Cleanup
    itsNormalEquation_p.reset();
    model_p.reset();

    // Return a null shared pointer, the worker does not
    // need to store the Normal Equations since they
    // have been sent to the master.
    return askap::scimath::INormalEquations::ShPtr();
}

void PreDifferWorker::reduceNE(askap::scimath::INormalEquations::ShPtr ne_p, int count)
{
    ReductionLogic rlogic(itsComms.getId(), itsComms.getNumNodes());
    const int accumulatorStep = rlogic.getAccumulatorStep();
    const int id = itsComms.getId();

    if (id % accumulatorStep == 0) {
        // Accumulator + worker
        int accumulatedCount = count;

        // Determine how many workers this accumulator is responsible for,
        // not counting itself, hence the subtraction of one.
        const int responsible = rlogic.responsible();

        ASKAPLOG_DEBUG_STR(logger, "Accumulator @" << id << " waiting for " << responsible
                << " workers to report normal equations");

        // Receive and merge normal equations for all workers this accumulator
        // is responsible for.
        for (int i = 0; i < responsible; ++i) {
            int source;
            PreDifferResponse response;
            itsComms.receiveMessageAnySrc(response, source);
            ASKAPCHECK(response.get_payloadType() == PreDifferResponse::RESULT,
                    "Expected only RESULT payloads at this time");

            int recvcount = response.get_count();
            askap::scimath::INormalEquations::ShPtr recv_ne_p = response.get_normalEquations();

            ASKAPLOG_DEBUG_STR(logger, "Accumulator @" << id << " received NE from " << source);

            // If the recvcount is zero, this indicates a null normal equation.
            // This occurs when a worker doesn't get any work assigned to it, but
            // the accumulator still needs know this.
            if (recvcount > 0) {
                ne_p->merge(*recv_ne_p);
                accumulatedCount += recvcount;
            }
        }

        // Finally send NE to the master
        PreDifferResponse response;
        response.set_payloadType(PreDifferResponse::RESULT);
        response.set_count(accumulatedCount);
        response.set_normalEquations(ne_p);
        itsComms.sendMessage(response, itsMaster);
    } else {
        // Worker only
        int accumulator = id - (id % accumulatorStep);

        // Send NE to the accumulator
        PreDifferResponse response;
        response.set_payloadType(PreDifferResponse::RESULT);
        response.set_count(count);
        response.set_normalEquations(ne_p);
        itsComms.sendMessage(response, accumulator);
    }
}
