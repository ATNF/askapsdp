/// @file SolverMaster.cc
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
#include "SolverMaster.h"

// System includes
#include <string>

// ASKAPsoft includes
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
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
#include <casa/OS/Timer.h>

// Local includes
#include <distributedimager/MPIBasicComms.h>
#include <distributedimager/DistributedImageSolverFactory.h>

// Using
using namespace askap;
using namespace askap::cp;
using namespace askap::scimath;
using namespace askap::synthesis;

ASKAP_LOGGER(logger, ".SolverMaster");

SolverMaster::SolverMaster(LOFAR::ACC::APS::ParameterSet& parset,
        askap::cp::MPIBasicComms& comms,
        askap::scimath::Params::ShPtr model_p)
: itsParset(parset), itsComms(comms), itsModel(model_p)
{
    setupRestoreBeam();

    const std::string solver_par = itsParset.getString("solver");
    const std::string algorithm_par = itsParset.getString("solver.Clean.algorithm", "MultiScale");
    const std::string distributed_par = itsParset.getString("solver.Clean.distributed", "False");
    // There is a distributed MultiScale Clean implementation in this processing
    // element, so use it if appropriate
    if (solver_par == "Clean" && algorithm_par == "MultiScale" &&
            distributed_par == "True") {
        itsSolver = DistributedImageSolverFactory::make(*itsModel, itsParset, itsComms);
    } else {
        itsSolver = ImageSolverFactory::make(*itsModel, itsParset);
    }
}

SolverMaster::~SolverMaster()
{
}

void SolverMaster::solveNE(askap::scimath::INormalEquations::ShPtr ne_p)
{
    casa::Timer timer;
    timer.mark();

    itsSolver->init();
    itsSolver->setParameters(*itsModel);
    itsSolver->addNormalEquations(*ne_p);

    ASKAPLOG_INFO_STR(logger, "Solving Normal Equations");
    askap::scimath::Quality q;

    itsSolver->solveNormalEquations(q);
    *itsModel = itsSolver->parameters();
    ASKAPLOG_INFO_STR(logger, "Solved normal equations in "<< timer.real()
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

double SolverMaster::getPeakResidual(askap::scimath::INormalEquations::ShPtr ne_p)
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

void SolverMaster::setupRestoreBeam(void)
{
    bool restore = itsParset.getBool("restore", false);

    if (restore) {
        itsQbeam.resize(3);
        std::vector<std::string> beam = itsParset.getStringVector("restore.beam");
        ASKAPCHECK(beam.size() == 3, "Need three elements for beam");
        for (int i = 0; i < 3; ++i) {
            casa::Quantity::read(itsQbeam(i), beam[i]);
        }
    }
}

void SolverMaster::writeModel(const std::string &postfix)
{
    ASKAPCHECK(itsModel, "itsModel is not correctly initialized");
    ASKAPCHECK(itsSolver, "itsSolver is not correctly initialized");

    ASKAPLOG_INFO_STR(logger, "Writing out results as CASA images");
    vector<string> resultimages = itsModel->names();
    for (vector<string>::const_iterator it=resultimages.begin(); it
            !=resultimages.end(); it++) {
        if ((it->find("image") == 0) || (it->find("psf") == 0) ||
                (it->find("weights") == 0) || (it->find("mask") == 0) ||
                (it->find("residual")==0)) {
            ASKAPLOG_INFO_STR(logger, "Saving " << *it << " with name " << *it+postfix );
            SynthesisParamsHelper::saveAsCasaImage(*itsModel, *it, *it+postfix);
        }
    }

    bool restore = itsParset.getBool("restore", false);
    if (restore && postfix == "") {
        ASKAPLOG_INFO_STR(logger, "Writing out restored images as CASA images");
        ImageRestoreSolver ir(*itsModel, itsQbeam);
        ir.setThreshold(itsSolver->threshold());
        ir.setVerbose(itsSolver->verbose());
        /// @todo Fix copying of preconditioners
        // Check for preconditioners. Same code as in ImageSolverFactory.
        // Will be neater if the RestoreSolver is also created in the ImageSolverFactory.
        const std::vector<std::string> preconditioners= itsParset.getStringVector("preconditioner.Names",std::vector<std::string>());
        if(preconditioners.size()) {
            for (vector<string>::const_iterator pc = preconditioners.begin(); pc != preconditioners.end(); ++pc) {
                if( (*pc)=="Wiener" ) {
                    float noisepower = itsParset.getFloat("preconditioner.Wiener.noisepower",0.0);
                    ir.addPreconditioner(IImagePreconditioner::ShPtr(new WienerPreconditioner(noisepower)));
                }
                if ((*pc) == "GaussianTaper") {
                    // at this stage we have to define tapers in uv-cells, rather than in klambda
                    // because the physical cell size is unknown to solver factory. 
                    // Theoretically we could parse the parameters here and extract the cell size and
                    // shape, but it can be defined separately for each image. We need to find
                    // the way of dealing with this complication.
                    ASKAPCHECK(itsParset.isDefined("preconditioner.GaussianTaper"),
                            "preconditioner.GaussianTaper showwing the taper size should be defined to use GaussianTaper");
                    const vector<double> taper = SynthesisParamsHelper::convertQuantity(
                            itsParset.getStringVector("preconditioner.GaussianTaper"),"rad");
                    ASKAPCHECK((taper.size() == 3) || (taper.size() == 1),
                            "preconditioner.GaussianTaper can have either single element or "
                            " a vector of 3 elements. You supplied a vector of "<<taper.size()<<" elements");
                    ASKAPCHECK(itsParset.isDefined("Images.shape") && itsParset.isDefined("Images.cellsize"),
                            "Imager.shape and Imager.cellsize should be defined to convert the taper fwhm specified in "
                            "angular units in the image plane into uv cells");
                    const std::vector<double> cellsize = SynthesisParamsHelper::convertQuantity(
                            itsParset.getStringVector("Images.cellsize"),"rad");
                    const std::vector<int> shape = itsParset.getInt32Vector("Images.shape");
                    ASKAPCHECK((cellsize.size() == 2) && (shape.size() == 2),
                            "Images.cellsize and Images.shape parameters should have exactly two values");
                    // factors which appear in nominator are effectively half sizes in radians
                    const double xFactor = cellsize[0]*double(shape[0])/2.;
                    const double yFactor = cellsize[1]*double(shape[1])/2.;

                    if (taper.size() == 3) {

                        ASKAPDEBUGASSERT((taper[0]!=0) && (taper[1]!=0));
                        ir.addPreconditioner(IImagePreconditioner::ShPtr(new GaussianTaperPreconditioner(
                                        xFactor/taper[0],yFactor/taper[1],taper[2])));
                    } else {
                        ASKAPDEBUGASSERT(taper[0]!=0);
                        if (std::abs(xFactor-yFactor)<4e-15) {
                            // the image is square, can use the short cut
                            ir.addPreconditioner(IImagePreconditioner::ShPtr(new GaussianTaperPreconditioner(xFactor/taper[0])));
                        } else {
                            // the image is rectangular. Although the gaussian taper is symmetric in
                            // angular coordinates, it will be elongated along the vertical axis in 
                            // the uv-coordinates.
                            ir.addPreconditioner(IImagePreconditioner::ShPtr(new GaussianTaperPreconditioner(xFactor/taper[0],
                                            yFactor/taper[0],0.)));
                        }
                    }
                }
            }
        } else {
            ir.addPreconditioner(IImagePreconditioner::ShPtr(new WienerPreconditioner()));
        }
        ir.copyNormalEquations(*itsSolver);
        Quality q;
        ir.solveNormalEquations(q);
        ASKAPDEBUGASSERT(itsModel);
        *itsModel = ir.parameters();
        resultimages=itsModel->completions("image");
        for (vector<string>::iterator it=resultimages.begin(); it
                !=resultimages.end(); it++)
        {
            string imageName("image"+(*it)+postfix);
            ASKAPLOG_INFO_STR(logger, "Saving restored image " << imageName << " with name "
                    << imageName+string(".restored") );
            SynthesisParamsHelper::saveAsCasaImage(*itsModel, "image"+(*it),
                    imageName+string(".restored"));
        }
    }
}

