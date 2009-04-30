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

using namespace askap::cp;
using namespace askap::scimath;
using namespace askap::synthesis;

ASKAP_LOGGER(logger, ".PreDifferWorker");

PreDifferWorker::PreDifferWorker(LOFAR::ACC::APS::ParameterSet& parset,
        askap::cp::IBasicComms& comms) : m_parset(parset), m_comms(comms)
{
    m_gridder_p = VisGridderFactory::make(m_parset);
}

PreDifferWorker::~PreDifferWorker()
{
    m_gridder_p.reset();
}

askap::scimath::INormalEquations::ShPtr PreDifferWorker::calcNE(askap::scimath::Params::ShPtr notused)
{
    // Assert PreConditions
    ASKAPCHECK(m_gridder_p, "m_gridder_p is not correctly initialized");

    // Receive the model
    askap::scimath::Params::ShPtr model_p = m_comms.receiveModel();

    // Pointer to measurement equation
    askap::scimath::Equation::ShPtr equation_p;

    // Normal equations which will be accumulated into
    // until all workunits have been received by this worker.
    m_ne_p = ImagingNormalEquations::ShPtr(new ImagingNormalEquations(*model_p));

    // This count tracks how many work units have had their results
    // merged into the normal equations pointed to be m_ne_p
    int count = 0;

    while (1) {
        // Ask the master for a workunit
        m_comms.sendString("next", cg_master);
        std::string ms = m_comms.receiveString(cg_master);
        if (ms == "") {
            // Indicates all workunits have been assigned already
            break;
        }

        casa::Timer timer;
        timer.mark();

        ASKAPLOG_INFO_STR(logger, "Calculating normal equations for " << ms );

        // Setup the DataSource
        bool useMemoryBuffers = m_parset.getBool("memorybuffers", false);
        if (useMemoryBuffers) {
            ASKAPLOG_INFO_STR(logger, "Scratch data will be held in memory" );
        } else {
            ASKAPLOG_INFO_STR(logger, "Scratch data will be written to the subtable of the original dataset" );
        }

        std::string colName = m_parset.getString("datacolumn", "DATA");
        TableDataSource ds(ms, (useMemoryBuffers ? TableDataSource::MEMORY_BUFFERS : TableDataSource::DEFAULT),
                colName);
        IDataSelectorPtr sel = ds.createSelector();
        sel << m_parset;
        IDataConverterPtr conv = ds.createConverter();
        conv->setFrequencyFrame(casa::MFrequency::Ref(casa::MFrequency::TOPO),
                "Hz");
        conv->setDirectionFrame(casa::MDirection::Ref(casa::MDirection::J2000));
        IDataSharedIter it = ds.createIterator(sel, conv);

        // Calc NE
        ASKAPCHECK(model_p, "model_p is not correctly initialized");
        ASKAPCHECK(m_ne_p, "ne_p is not correctly initialized");

        equation_p = askap::scimath::Equation::ShPtr(new ImageFFTEquation(*model_p, it, m_gridder_p));
        ASKAPCHECK(equation_p, "equation_p is not correctly initialized");
        equation_p->calcEquations(*m_ne_p);

        ASKAPLOG_INFO_STR(logger, "Calculated normal equations for "<< ms << " in "
                << timer.real() << " seconds ");

        equation_p.reset();

        count++;
    }

    // Even if the count is zero, must report the empty object
    reduceNE(m_ne_p, count);

    // Cleanup
    m_ne_p.reset();
    model_p.reset();

    // Return a null shared pointer, the worker does not
    // need to store the Normal Equations since they
    // have been sent to the master.
    return askap::scimath::INormalEquations::ShPtr();
}

void PreDifferWorker::reduceNE(askap::scimath::INormalEquations::ShPtr ne_p, int count)
{
    // Specify how large the batch for the reduction is. If this is set to 16, and
    // there are 256 processes (numNodes) then the reduction is 256->16>1.
    // TODO: Make this something better than a magic number. Ideally this
    // code would do a multi level graph reduction, whereas now it just does a
    // single level reduction.
    const int accumulatorStep = 16;

    const int id = m_comms.getId();
    const int numNodes = m_comms.getNumNodes();

    if (id % accumulatorStep == 0) {
        // Accumulator + worker
        int accumulatedCount = count;

        // Determine how many workers this accumulator is responsible for,
        // not counting itself, hence the subtraction of one.
        int responsible;
        if ((id + accumulatorStep) > numNodes) {
            responsible = numNodes - id - 1;
        } else {
            responsible = accumulatorStep - 1;
        }

        ASKAPLOG_INFO_STR(logger, "Accumulator @" << id << " waiting for " << responsible
                << " workers to report normal equations");

        // Receive and merge normal equations for all workers this accumulator
        // is responsible for.
        for (int i = 0; i < responsible; ++i) {
            int source;
            int recvcount;
            askap::scimath::INormalEquations::ShPtr recv_ne_p = m_comms.receiveNE(source, recvcount);
            ASKAPLOG_INFO_STR(logger, "Accumulator @" << id << " received NE from " << source);

            // If the recvcount is zero, this indicates a null normal equation.
            // This occurs when a worker doesn't get any work assigned to it, but
            // the accumulator still needs know this.
            if (recvcount > 0) {
                ne_p->merge(*recv_ne_p);
                accumulatedCount += recvcount;
            }
        }

        // Finally send NE to the master
        m_comms.sendNE(m_ne_p, cg_master, accumulatedCount);

    } else {
        // Worker only
        int accumulator = id - (id % accumulatorStep);

        // Send NE to the accumulator
        m_comms.sendNE(m_ne_p, accumulator, count);
    }
}
