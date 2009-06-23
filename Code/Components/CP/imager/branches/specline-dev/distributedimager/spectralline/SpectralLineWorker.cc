/// @file SpectralLineWorker.cc
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
#include "SpectralLineWorker.h"

// System includes
#include <string>
#include <sstream>
#include <stdexcept>
#include <vector>

// ASKAPsoft includes
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <fitting/Equation.h>
#include <fitting/INormalEquations.h>
#include <fitting/ImagingNormalEquations.h>
#include <fitting/Params.h>
#include <gridding/IVisGridder.h>
#include <gridding/VisGridderFactory.h>
#include <measurementequation/SynthesisParamsHelper.h>
#include <measurementequation/ImageFFTEquation.h>
#include <measurementequation/SynthesisParamsHelper.h>
#include <dataaccess/IConstDataSource.h>
#include <dataaccess/TableConstDataSource.h>
#include <dataaccess/IConstDataIterator.h>
#include <dataaccess/IDataConverter.h>
#include <dataaccess/IDataSelector.h>
#include <dataaccess/IDataIterator.h>
#include <dataaccess/SharedIter.h>
#include <APS/ParameterSet.h>
#include <APS/Exceptions.h>
#include <casa/OS/Timer.h>

// Local includes
#include "distributedimager/common/IBasicComms.h"
#include "distributedimager/common/SolverCore.h"
#include "messages/SpectralLineWorkUnit.h"
#include "messages/SpectralLineWorkRequest.h"

using namespace askap::cp;
using namespace askap;
using namespace askap::scimath;
using namespace askap::synthesis;
using namespace LOFAR::ACC::APS;

ASKAP_LOGGER(logger, ".SpectralLineWorker");

SpectralLineWorker::SpectralLineWorker(LOFAR::ACC::APS::ParameterSet& parset,
        askap::cp::IBasicComms& comms)
: itsParset(parset), itsComms(comms)
{
    itsGridder_p = VisGridderFactory::make(itsParset);
}

SpectralLineWorker::~SpectralLineWorker()
{
    itsGridder_p.reset();
}

void SpectralLineWorker::run(void)
{
    while(1) {
        // Send a request for work
        SpectralLineWorkRequest wrequest;
        itsComms.sendMessage(wrequest, itsMaster);

        // Get a workunit
        SpectralLineWorkUnit wu;
        itsComms.receiveMessage(wu, itsMaster);

        if (wu.get_payloadType() == SpectralLineWorkUnit::DONE) {
            // Indicates all workunits have been assigned already
            ASKAPLOG_INFO_STR(logger, "Received DONE signal");
            break;
        }

        const std::string ms = wu.get_dataset();
        ASKAPLOG_INFO_STR(logger, "Received Work Unit for dataset " << ms );
        processWorkUnit(wu);
    }

}

void SpectralLineWorker::processWorkUnit(const SpectralLineWorkUnit& wu)
{
    const std::string colName = itsParset.getString("datacolumn", "DATA");
    const std::string ms = wu.get_dataset();

    TableDataSource ds(ms, TableDataSource::DEFAULT, colName);
    IDataSelectorPtr sel = ds.createSelector();
    IDataConverterPtr conv = ds.createConverter();

    conv->setFrequencyFrame(casa::MFrequency::Ref(casa::MFrequency::TOPO), "Hz");
    conv->setDirectionFrame(casa::MDirection::Ref(casa::MDirection::J2000));
    IDataSharedIter it = ds.createIterator(sel, conv);

    const int nChannels = it->nChannel();
    const std::string imagename = itsParset.getString("Images.name");

    for (int i = 0; i < nChannels; ++i) {
        processChannel(ds, imagename, i, wu.get_channelOffset());
    }

}

void SpectralLineWorker::processChannel(askap::synthesis::TableDataSource& ds,
        const std::string& imagename, int channel, int channelOffset)
{
    ASKAPLOG_INFO_STR(logger, "Processing channel " << (channel + channelOffset + 1));

    askap::scimath::Params::ShPtr model_p(new Params());
    setupImage(model_p, (channelOffset + channel + 1));

    casa::Timer timer;
    timer.mark();

    // Setup data iterator
    IDataSelectorPtr sel = ds.createSelector();
    sel->chooseChannels(1, channel);
    IDataConverterPtr conv = ds.createConverter();
    conv->setFrequencyFrame(casa::MFrequency::Ref(casa::MFrequency::TOPO), "Hz");
    conv->setDirectionFrame(casa::MDirection::Ref(casa::MDirection::J2000));
    IDataSharedIter it = ds.createIterator(sel, conv);

    // Setup normal equations
    askap::scimath::INormalEquations::ShPtr ne_p
        = ImagingNormalEquations::ShPtr(new ImagingNormalEquations(*model_p));

    // Setup measurement equations
    askap::scimath::Equation::ShPtr equation_p
        = askap::scimath::Equation::ShPtr(new ImageFFTEquation(*model_p, it, itsGridder_p));

    // Calc NE
    ASKAPLOG_INFO_STR(logger, "Calculating normal equations for channel " << (channel + channelOffset + 1));
    ASKAPCHECK(model_p, "model_p is not correctly initialized");
    ASKAPCHECK(ne_p, "ne_p is not correctly initialized");
    ASKAPCHECK(equation_p, "equation_p is not correctly initialized");
    equation_p->calcEquations(*ne_p);

    ASKAPLOG_INFO_STR(logger, "Calculated normal equations for channel "
            << (channel + channelOffset + 1) << " in "
            << timer.real() << " seconds ");

    equation_p.reset();

    // Solve NE
    SolverCore solverCore(itsParset, itsComms, model_p);
    solverCore.solveNE(ne_p);

    // Write image
    solverCore.writeModel("");
}

void SpectralLineWorker::setupImage(const askap::scimath::Params::ShPtr& params, int actualChannel)
{
    try {
        ParameterSet parset = itsParset.makeSubset("Images.");

        const int nfacets = parset.getInt32("nfacets", 1);
        const std::string nameParam = parset.getString("name");
        const std::vector<std::string> direction = parset.getStringVector("direction");
        const std::vector<std::string> cellsize = parset.getStringVector("cellsize");
        const std::vector<int> shape = parset.getInt32Vector("shape");
        const std::vector<double> freq = parset.getDoubleVector("frequency");
        const int nchan = 1;

        ASKAPCHECK(nfacets > 0, "Number of facets is supposed to be a positive number, you gave "<<nfacets);
        ASKAPCHECK(shape.size() >= 2, "Image is supposed to be at least two dimensional. "<<
                "check shape parameter, you gave "<<shape);

        // Add suffix to the image name to indicate channel number
        std::stringstream name;
        name << nameParam << "_ch" << actualChannel;

        if (nfacets == 1) {
            ASKAPLOG_INFO_STR(logger, "Setting up new empty image "<< name.str() );
            SynthesisParamsHelper::add(*params, name.str(), direction, cellsize, shape, freq[0], freq[1], nchan);
        } else {
            // this is a multi-facet case
            ASKAPLOG_INFO_STR(logger, "Setting up "<<nfacets<<" x "<<nfacets<<
                    " new empty facets for image "<< name.str() );
            const int facetstep = parset.getInt32("facetstep",casa::min(shape[0], shape[1]));
            ASKAPCHECK(facetstep > 0, "facetstep parameter is supposed to be positive, you have "<<facetstep);
            ASKAPLOG_INFO_STR(logger, "Facet centers will be "<< facetstep <<
                    " pixels apart, each facet size will be "<< shape[0] << " x " << shape[1]);
            SynthesisParamsHelper::add(*params, name.str(), direction, cellsize, shape, freq[0], freq[1], nchan, nfacets, facetstep);
        }

    } catch (const LOFAR::ACC::APS::APSException &ex) {
        throw AskapError(ex.what());
    }
}
