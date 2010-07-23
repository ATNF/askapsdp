/// @file SolverCore.cc
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
#include "SolverCore.h"

// System includes
#include <string>

// ASKAPsoft includes
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <Common/ParameterSet.h>
#include <fitting/INormalEquations.h>
#include <fitting/ImagingNormalEquations.h>
#include <fitting/Params.h>
#include <fitting/Solver.h>
#include <fitting/Quality.h>
#include <measurementequation/ImageSolverFactory.h>
#include <measurementequation/SynthesisParamsHelper.h>
#include <measurementequation/ImageRestoreSolver.h>
#include <measurementequation/IImagePreconditioner.h>
#include <measurementequation/WienerPreconditioner.h>
#include <measurementequation/GaussianTaperPreconditioner.h>
#include <measurementequation/ImageMultiScaleSolver.h>
#include <measurementequation/ImageParamsHelper.h>
#include <casa/OS/Timer.h>

// Local includes
#include "distributedimager/common/IBasicComms.h"
#include "distributedimager/common/DistributedImageSolverFactory.h"

// Using
using namespace askap;
using namespace askap::cp;
using namespace askap::scimath;
using namespace askap::synthesis;

ASKAP_LOGGER(logger, ".SolverCore");

SolverCore::SolverCore(LOFAR::ParameterSet& parset,
        askap::cp::IBasicComms& comms,
        askap::scimath::Params::ShPtr model_p)
: itsParset(parset), itsComms(comms), itsModel(model_p)
{
    const std::string solver_par = itsParset.getString("solver");
    const std::string algorithm_par = itsParset.getString("solver.Clean.algorithm", "MultiScale");
    const std::string distributed_par = itsParset.getString("solver.Clean.distributed", "False");
    const std::string mode = itsParset.getString("mode","Continuum");
    // There is a distributed MultiScale Clean implementation in this processing
    // element, so use it if appropriate
    if (solver_par == "Clean" && algorithm_par == "MultiScale" &&
            distributed_par == "True" && mode == "Continuum") {
        itsSolver = DistributedImageSolverFactory::make(*itsModel, itsParset, itsComms);
    } else {
        itsSolver = ImageSolverFactory::make(*itsModel, itsParset);
    }
}

SolverCore::~SolverCore()
{
}

void SolverCore::solveNE(askap::scimath::INormalEquations::ShPtr ne_p)
{
    casa::Timer timer;
    timer.mark();

    itsSolver->init();
    itsSolver->addNormalEquations(*ne_p);

    ASKAPLOG_INFO_STR(logger, "Solving Normal Equations");
    askap::scimath::Quality q;

    ASKAPDEBUGASSERT(itsModel);
    itsSolver->solveNormalEquations(*itsModel, q);
    ASKAPLOG_DEBUG_STR(logger, "Solved normal equations in "<< timer.real()
            << " seconds ");

    // Extract the largest residual
    const std::vector<std::string> peakParams = itsModel->completions("peak_residual.");

    double peak = peakParams.size() == 0 ? getPeakResidual(ne_p) : -1.;
    for (std::vector<std::string>::const_iterator peakParIt = peakParams.begin();
            peakParIt != peakParams.end(); ++peakParIt) {
        const double tempval = std::abs(itsModel->scalarValue("peak_residual." + *peakParIt));
        if (tempval > peak) {
            peak = tempval;
        }
    }

    if (itsModel->has("peak_residual")) {
        itsModel->update("peak_residual",peak);
    } else {
        itsModel->add("peak_residual",peak);
    }
    itsModel->fix("peak_residual");
}

double SolverCore::getPeakResidual(askap::scimath::INormalEquations::ShPtr ne_p)
{
    // we need a specialized method of the imaging normal equations to get the peak
    // for all images. Multiple images can be represented by a single normal equations class.
    // We could also use the dataVector method of the interface (INormalEquations). However,
    // it is a bit cumbersome to iterate over all parameters. It is probably better to
    // leave this full case for a future as there is no immediate use case.
    boost::shared_ptr<askap::scimath::ImagingNormalEquations> ine =
        boost::dynamic_pointer_cast<askap::scimath::ImagingNormalEquations>(ne_p);
    // we could have returned some special value (e.g. negative), but throw exception for now
    ASKAPCHECK(ine, "Current code to calculate peak residuals works for imaging-specific normal equations only");
    double peak = -1.;
    const std::map< std::string, casa::Vector<double> >& dataVector =
        ine->dataVector();
    const std::map< std::string, casa::Vector<double> >& diag =
        ine->normalMatrixDiagonal();
    for (std::map< std::string, casa::Vector<double> >::const_iterator ci =
            dataVector.begin(); ci != dataVector.end(); ++ci) {
        if (ci->first.find("image") == 0) {
            // this is an image
            ASKAPASSERT(ci->second.nelements() != 0);
            std::map< std::string, casa::Vector<double> >::const_iterator diagIt =
                diag.find(ci->first);
            ASKAPDEBUGASSERT(diagIt != diag.end());
            const double maxDiag = casa::max(diagIt->second);
            // hard coded at this stage
            const double cutoff = 1e-2 * maxDiag;
            ASKAPDEBUGASSERT(diagIt->second.nelements() == ci->second.nelements());
            for (casa::uInt elem = 0; elem < diagIt->second.nelements(); ++elem) {
                const double thisDiagElement = std::abs(diagIt->second[elem]);
                if (thisDiagElement > cutoff) {
                    const double tempPeak = ci->second[elem] / thisDiagElement;
                    if (tempPeak > peak) {
                        peak = tempPeak;
                    }
                }
            }
        }
    }
    return peak;
}

void SolverCore::writeModel(const std::string &postfix)
{
    ASKAPCHECK(itsModel, "itsModel is not correctly initialized");
    ASKAPCHECK(itsSolver, "itsSolver is not correctly initialized");

    SynthesisParamsHelper::setUpImageHandler(itsParset);

    ASKAPLOG_INFO_STR(logger, "Writing out results as images");
    vector<string> resultimages = itsModel->names();
    for (vector<string>::const_iterator it=resultimages.begin(); it
            !=resultimages.end(); it++) {
        if ((it->find("image") == 0) || (it->find("psf") == 0) ||
                (it->find("weights") == 0) || (it->find("mask") == 0) ||
                (it->find("residual")==0)) {
            ASKAPLOG_INFO_STR(logger, "Saving " << *it << " with name " << *it+postfix );
            SynthesisParamsHelper::saveImageParameter(*itsModel, *it, *it+postfix);
        }
    }

    bool restore = itsParset.getBool("restore", false);
    if (restore && postfix == "") {
        ASKAPLOG_INFO_STR(logger, "Writing out restored images as CASA images");

        ASKAPDEBUGASSERT(itsModel);
        boost::shared_ptr<ImageRestoreSolver> ir = ImageRestoreSolver::createSolver(itsParset.makeSubset("restore."),
                *itsModel);
        ASKAPDEBUGASSERT(ir);
        ASKAPDEBUGASSERT(itsSolver);
        // configure restore solver the same way as normal imaging solver
        boost::shared_ptr<ImageSolver> template_solver = boost::dynamic_pointer_cast<ImageSolver>(itsSolver);
        ASKAPDEBUGASSERT(template_solver);
        ImageSolverFactory::configurePreconditioners(itsParset,ir);
        ir->configureSolver(*template_solver);
        ir->copyNormalEquations(*template_solver);

        Quality q;
        ir->solveNormalEquations(*itsModel, q);
        ASKAPDEBUGASSERT(itsModel);
        // merged image should be a fixed parameter without facet suffixes
        resultimages=itsModel->fixedNames();
        for (vector<string>::const_iterator ci=resultimages.begin(); ci!=resultimages.end(); ++ci) {
            const ImageParamsHelper iph(*ci);
            if (!iph.isFacet() && (ci->find("image") == 0)) {
                ASKAPLOG_INFO_STR(logger, "Saving restored image " << *ci << " with name "
                        << *ci+string(".restored") );
                SynthesisParamsHelper::saveImageParameter(*itsModel, *ci,*ci+string(".restored"));
            }
        }
    }
}

