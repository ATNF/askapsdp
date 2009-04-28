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

    // Pointer to normal equation
    askap::scimath::INormalEquations::ShPtr ne_p;

    // Pointer to measurement equation
    askap::scimath::Equation::ShPtr equation_p;

    while (1) {
        std::string ms = m_comms.receiveString(cg_master);
        if (ms == "") {
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
        ne_p = ImagingNormalEquations::ShPtr(new ImagingNormalEquations(*model_p));
        ASKAPCHECK(ne_p, "ne_p is not correctly initialized");

        equation_p = askap::scimath::Equation::ShPtr(new ImageFFTEquation(*model_p, it, m_gridder_p));
        ASKAPCHECK(equation_p, "equation_p is not correctly initialized");
        equation_p->calcEquations(*ne_p);

        ASKAPLOG_INFO_STR(logger, "Calculated normal equations for "<< ms << " in "
                << timer.real() << " seconds ");

        // Send NE to the master
        m_comms.sendNE(ne_p, cg_master);
    }

    // Cleanup
    equation_p.reset();
    ne_p.reset();
    model_p.reset();

    // Return a null shared pointer, the worker does not
    // need to store the Normal Equations since they
    // have been sent to the master.
    return askap::scimath::INormalEquations::ShPtr();
}
