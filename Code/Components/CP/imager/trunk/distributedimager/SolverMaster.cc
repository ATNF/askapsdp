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

using namespace askap;
using namespace askap::cp;
using namespace askap::scimath;
using namespace askap::synthesis;

ASKAP_LOGGER(logger, ".SolverMaster");

SolverMaster::SolverMaster(LOFAR::ACC::APS::ParameterSet& parset,
        askap::cp::IBasicComms& comms,
        askap::scimath::Params::ShPtr model_p)
: m_parset(parset), m_comms(comms), m_model_p(model_p)
{
    setupRestoreBeam();
    m_solver_p = ImageSolverFactory::make(*m_model_p, m_parset);
}

SolverMaster::~SolverMaster()
{
}

void SolverMaster::solveNE(askap::scimath::INormalEquations::ShPtr ne_p)
{
    casa::Timer timer;
    timer.mark();

    m_solver_p->init();
    m_solver_p->setParameters(*m_model_p);
    m_solver_p->addNormalEquations(*ne_p);

    ASKAPLOG_INFO_STR(logger, "Solving Normal Equations");
    askap::scimath::Quality q;

    const std::string solver_par = m_parset.getString("solver");
    const std::string algorithm_par = m_parset.getString("solver.Clean.algorithm", "MultiScale");
    const std::string distributed_par = m_parset.getString("solver.Clean.distributed", "False");

    // There is a distributed MultiScale Clean implementation in this processing
    // element, so use it if appropriate
    if (solver_par == "Clean" && algorithm_par == "MultiScale" &&
            distributed_par == "True") {
        distributedMSClean(m_solver_p, q);
    } else {
        m_solver_p->solveNormalEquations(q);
        *m_model_p = m_solver_p->parameters();
    }
    ASKAPLOG_INFO_STR(logger, "Solved normal equations in "<< timer.real()
            << " seconds ");

    // Extract the largest residual
    const std::vector<std::string> peakParams = m_model_p->completions("peak_residual.");

    double peak = peakParams.size() == 0 ? getPeakResidual(ne_p) : -1.;
    for (std::vector<std::string>::const_iterator peakParIt = peakParams.begin();
            peakParIt != peakParams.end(); ++peakParIt) {
        const double tempval = std::abs(m_model_p->scalarValue("peak_residual." + *peakParIt));
        if (tempval > peak) {
            peak = tempval;
        }
    }

    if (m_model_p->has("peak_residual")) {
        m_model_p->update("peak_residual",peak);
    } else {
        m_model_p->add("peak_residual",peak);
    }
    m_model_p->fix("peak_residual");
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
    bool restore = m_parset.getBool("restore", false);

    if (restore) {
        m_Qbeam.resize(3);
        std::vector<std::string> beam = m_parset.getStringVector("restore.beam");
        ASKAPCHECK(beam.size() == 3, "Need three elements for beam");
        for (int i = 0; i < 3; ++i) {
            casa::Quantity::read(m_Qbeam(i), beam[i]);
        }
    }
}

void SolverMaster::writeModel(const std::string &postfix)
{
    ASKAPCHECK(m_model_p, "m_model_p is not correctly initialized");
    ASKAPCHECK(m_solver_p, "m_solver_p is not correctly initialized");

    ASKAPLOG_INFO_STR(logger, "Writing out results as CASA images");
    vector<string> resultimages = m_model_p->names();
    for (vector<string>::const_iterator it=resultimages.begin(); it
            !=resultimages.end(); it++) {
        if ((it->find("image") == 0) || (it->find("psf") == 0) ||
                (it->find("weights") == 0) || (it->find("mask") == 0) ||
                (it->find("residual")==0)) {
            ASKAPLOG_INFO_STR(logger, "Saving " << *it << " with name " << *it+postfix );
            SynthesisParamsHelper::saveAsCasaImage(*m_model_p, *it, *it+postfix);
        }
    }

    bool restore = m_parset.getBool("restore", false);
    if (restore && postfix == "") {
        ASKAPLOG_INFO_STR(logger, "Writing out restored images as CASA images");
        ImageRestoreSolver ir(*m_model_p, m_Qbeam);
        ir.setThreshold(m_solver_p->threshold());
        ir.setVerbose(m_solver_p->verbose());
        /// @todo Fix copying of preconditioners
        // Check for preconditioners. Same code as in ImageSolverFactory.
        // Will be neater if the RestoreSolver is also created in the ImageSolverFactory.
        const std::vector<std::string> preconditioners= m_parset.getStringVector("preconditioner.Names",std::vector<std::string>());
        if(preconditioners.size()) {
            for (vector<string>::const_iterator pc = preconditioners.begin(); pc != preconditioners.end(); ++pc) {
                if( (*pc)=="Wiener" ) {
                    float noisepower = m_parset.getFloat("preconditioner.Wiener.noisepower",0.0);
                    ir.addPreconditioner(IImagePreconditioner::ShPtr(new WienerPreconditioner(noisepower)));
                }
                if ((*pc) == "GaussianTaper") {
                    // at this stage we have to define tapers in uv-cells, rather than in klambda
                    // because the physical cell size is unknown to solver factory. 
                    // Theoretically we could parse the parameters here and extract the cell size and
                    // shape, but it can be defined separately for each image. We need to find
                    // the way of dealing with this complication.
                    ASKAPCHECK(m_parset.isDefined("preconditioner.GaussianTaper"),
                            "preconditioner.GaussianTaper showwing the taper size should be defined to use GaussianTaper");
                    const vector<double> taper = SynthesisParamsHelper::convertQuantity(
                            m_parset.getStringVector("preconditioner.GaussianTaper"),"rad");
                    ASKAPCHECK((taper.size() == 3) || (taper.size() == 1),
                            "preconditioner.GaussianTaper can have either single element or "
                            " a vector of 3 elements. You supplied a vector of "<<taper.size()<<" elements");
                    ASKAPCHECK(m_parset.isDefined("Images.shape") && m_parset.isDefined("Images.cellsize"),
                            "Imager.shape and Imager.cellsize should be defined to convert the taper fwhm specified in "
                            "angular units in the image plane into uv cells");
                    const std::vector<double> cellsize = SynthesisParamsHelper::convertQuantity(
                            m_parset.getStringVector("Images.cellsize"),"rad");
                    const std::vector<int> shape = m_parset.getInt32Vector("Images.shape");
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
        ir.copyNormalEquations(*m_solver_p);
        Quality q;
        ir.solveNormalEquations(q);
        ASKAPDEBUGASSERT(m_model_p);
        *m_model_p = ir.parameters();
        resultimages=m_model_p->completions("image");
        for (vector<string>::iterator it=resultimages.begin(); it
                !=resultimages.end(); it++)
        {
            string imageName("image"+(*it)+postfix);
            ASKAPLOG_INFO_STR(logger, "Saving restored image " << imageName << " with name "
                    << imageName+string(".restored") );
            SynthesisParamsHelper::saveAsCasaImage(*m_model_p, "image"+(*it),
                    imageName+string(".restored"));
        }
    }
}

void SolverMaster::distributedMSClean(askap::scimath::Solver::ShPtr solver_p, Quality &quality)
{
    ImageMultiScaleSolver *mssolver_p = dynamic_cast<ImageMultiScaleSolver *>(solver_p.get());
    Params &params = *m_model_p;

    // Build scales vector
    std::vector<float> defaultScales(3);
    defaultScales[0] = 0.0;
    defaultScales[1] = 10.0;
    defaultScales[2] = 30.0;

    std::vector<float> scales =
        m_parset.getFloatVector("solver.Clean.scales", defaultScales);

    // Solving A^T Q^-1 V = (A^T Q^-1 A) P
    uint nParameters=0;

    // Find all the free parameters beginning with image
    std::vector<std::string> names(params.completions("image"));
    std::map<std::string, uint> indices;

    for (std::vector<std::string>::const_iterator  it=names.begin();it!=names.end();it++)
    {
        std::string name="image"+*it;
        if(params.isFree(name)) {
            indices[name]=nParameters;
            nParameters+=params.value(name).nelements();
        }
    }
    ASKAPCHECK(nParameters>0, "No free parameters in ImageMultiScaleSolver");

    for (std::map<std::string, uint>::const_iterator indit=indices.begin(); indit!=indices.end(); ++indit)
    {
        // Axes are dof, dof for each parameter
        const casa::IPosition vecShape(1, params.value(indit->first).nelements());
        const casa::IPosition valShape(params.value(indit->first).shape());

        const scimath::ImagingNormalEquations &normalEquations = mssolver_p->normalEquations();

        ASKAPCHECK(normalEquations.normalMatrixDiagonal().count(indit->first)>0, "Diagonal not present");
        const casa::Vector<double>& diag(normalEquations.normalMatrixDiagonal().find(indit->first)->second);
        ASKAPCHECK(normalEquations.dataVector(indit->first).size()>0, "Data vector not present");
        const casa::Vector<double>& dv = normalEquations.dataVector(indit->first);
        ASKAPCHECK(normalEquations.normalMatrixSlice().count(indit->first)>0, "PSF Slice not present");
        const casa::Vector<double>& slice(normalEquations.normalMatrixSlice().find(indit->first)->second);

        casa::Array<float> dirtyArray(valShape);
        casa::convertArray<float, double>(dirtyArray, dv.reform(valShape));
        casa::Array<float> psfArray(valShape);
        casa::convertArray<float, double>(psfArray, slice.reform(valShape));
        casa::Array<float> cleanArray(valShape);
        casa::convertArray<float, double>(cleanArray, params.value(indit->first));
        casa::Array<float> maskArray(valShape);

        // Normalize
        mssolver_p->doNormalization(diag,mssolver_p->tol(),psfArray,dirtyArray,
                boost::shared_ptr<casa::Array<float> >(&maskArray, utility::NullDeleter()));

        // Precondition the PSF and DIRTY images before solving.
        if(mssolver_p->doPreconditioning(psfArray,dirtyArray)) {
            // Save the new PSFs to disk
            Axes axes(params.axes(indit->first));
            std::string psfName="psf."+(indit->first);
            casa::Array<double> anothertemp(valShape);
            casa::convertArray<double,float>(anothertemp,psfArray);
            const casa::Array<double> & APSF(anothertemp);
            if (!params.has(psfName)) {
                params.add(psfName, APSF, axes);
            } else {
                params.update(psfName, APSF);
            }
        } // if there was preconditioning
        ASKAPLOG_INFO_STR(logger, "Peak data vector flux (derivative) "<<max(dirtyArray));

        // Create Lattices and use iterators to get the patches. The
        // PSF center must be extracted.
        casa::ArrayLattice<float> dirtyLattice(dirtyArray);
        casa::ArrayLattice<float> psfLattice(psfArray);
        casa::ArrayLattice<float> cleanLattice(cleanArray);
        casa::ArrayLattice<float> maskLattice(maskArray);

        // Save the residual image.
        // This takes up some memory and we have to ship the residual image out inside
        // the parameter class. Therefore, we may not need this functionality in the
        // production version (or may need to implement it in a different way).
        {
           Axes axes(m_model_p->axes(indit->first));
           ASKAPDEBUGASSERT(indit->first.find("image")==0);
           ASKAPCHECK(indit->first.size()>5,
                   "Image parameter name should have something appended to word image")
           const string residName="residual"+indit->first.substr(5);
           casa::Array<double> anothertemp(valShape);
           casa::convertArray<double,float>(anothertemp,dirtyArray);
           const casa::Array<double> & AResidual(anothertemp);
           if (!m_model_p->has(residName)) {
               m_model_p->add(residName, AResidual, axes);
           } else {
               m_model_p->update(residName, AResidual);
           }
        }

        //////////////////////////////////////////
        // Send work to SolverWorker
        //////////////////////////////////////////

        // Get the dimension of the image, first checking it is the same size
        // as the clean image and mask and ensuring it is square.
        ASKAPCHECK(dirtyLattice.shape() == cleanLattice.shape(),
                "Dimensions of dirty and clean differ");
        ASKAPCHECK(dirtyLattice.shape() == maskLattice.shape(),
                "Dimensions of dirty and mask differ");

        casa::IPosition dirtyShape = dirtyLattice.shape();
        ASKAPCHECK(dirtyShape.nelements() != 2,
                "Dirty image has more than two dimensions");

        const int size_x = dirtyShape(0);
        const int size_y = dirtyShape(1);

        ASKAPCHECK(size_y == size_x, "Only square images are supported");

        // Get and check patch size
        const int c_patchSize = m_parset.getInt32("solver.Clean.patchsize", 512);

        ASKAPCHECK(size_x >= c_patchSize,
                "Image size must be >= patch size");

        ASKAPCHECK(size_x % c_patchSize == 0,
                "Image size must be a multiple of patch size");

        // Use an iterator to get at the patches
        casa::IPosition patchShape(2, c_patchSize, c_patchSize);

        const int c_blc = (size_x / 2) - (c_patchSize / 2);
        const int c_trc = (size_x / 2) + (c_patchSize / 2) - 1;

        // Cut ouf the PSF center
        casa::IPosition blc(4, c_blc, c_blc, 0, 0);
        casa::IPosition trc(4, c_trc, c_trc, 0, 0);
        casa::LCBox centerBox(blc, trc, psfLattice.shape());
        casa::SubLattice<float> psfCenter(psfLattice, centerBox, false);

        // Dirty image iterator
        casa::LatticeStepper dstepper(dirtyLattice.shape(), patchShape);
        casa::RO_LatticeIterator<float> diterator(dirtyLattice, dstepper);

        // Mask iterator
        casa::LatticeStepper maskstepper(maskLattice.shape(), patchShape);
        casa::RO_LatticeIterator<float> maskiterator(maskLattice, maskstepper);

        // Model iterator
        casa::LatticeStepper mstepper(cleanLattice.shape(), patchShape);
        casa::LatticeIterator<float> miterator(cleanLattice, mstepper);

        // Now iterate through and send the patches to cleaner PEs
        int patchid = 0;
        for (diterator.reset(), miterator.reset(); !diterator.atEnd(); diterator++, miterator++, maskiterator++, patchid++) {
            const casa::Array<float> dirtyPatch = diterator.cursor();
            const casa::Array<float> maskPatch = maskiterator.cursor();
            boost::shared_ptr< casa::Array<float> > cleanPatch(new casa::Array<float>(miterator.rwCursor()));

            // TODO: Waiting for a string is a dumb way for the worker to indicate
            // it wants more work to do. Need a MUCH better way of doing this. Some
            // sort of command message incorporating this plus the "no more workunits"
            // message (below) could be developed.
            int source;
            while (m_comms.receiveStringAny(source) != "next") {
                ASKAPLOG_INFO_STR(logger, "Got CleanResponse - Still work to do");
                processCleanResponse();
            }

            m_comms.sendString("ok", source);

            ASKAPLOG_INFO_STR(logger, "Master is allocating CleanRequest " << patchid
                    << " to worker " << source);

            // Put workunit on the workq, need to put it on the workq
            // before sending the request to avoid race conditions.
            CleanerWork work;
            work.patchid = patchid;
            work.model = cleanPatch;
            work.done = false;
            work.strengthOptimum = 0.0;

            m_cleanworkq.push_back(work);

            // To avoid const issue
            double threshold = mssolver_p->threshold().getValue();
            std::string thresholdUnits = mssolver_p->threshold().getUnit();
            m_comms.sendCleanRequest(patchid,
                    dirtyPatch,
                    psfCenter.get(),
                    maskPatch,
                    *(cleanPatch), 
                    threshold,
                    thresholdUnits,
                    mssolver_p->fractionalThreshold(),
                    scales,
                    mssolver_p->niter(),
                    mssolver_p->gain(),
                    source);
        }

        while (outstanding())
        {
                ASKAPLOG_INFO_STR(logger, "Waiting for outstanding CleanRequests");
                processCleanResponse();
        }
                ASKAPLOG_INFO_STR(logger, "No more outstanding CleanRequests");

        // Send each process an empty string to indicate
        // there are no more workunits on offer (TODO: Need to find
        // a better way of doing this)
        for (int dest = 1; dest < m_comms.getNumNodes(); ++dest) {
            ASKAPLOG_INFO_STR(logger, "Finishing up for  worker " << dest);
            std::string msg = m_comms.receiveString(dest);
            if (msg == "response") {
                // ignore
                --dest;
                continue;
            }
            ASKAPLOG_INFO_STR(logger, "Read from " << dest << " the message: " << msg);
            ASKAPCHECK(msg == "next", "Expected message: next");
            m_comms.sendString("", dest);
        }


        // Check if all patches have been cleaned and determine strengthOptimum.
        double strengthOptimum = 0.0;
        std::vector<CleanerWork>::iterator it;
        for (it = m_cleanworkq.begin() ; it < m_cleanworkq.end(); ++it) {
            if (it->done == false) {
                ASKAPTHROW (std::runtime_error,
                        "All CleanRequests should have been completed. Still waiting for patchid "
                        << it->patchid);
            }

            // Checking all patches get the highest absolute strengthOptimum
            if (fabs(it->strengthOptimum) > fabs(strengthOptimum)) {
                strengthOptimum = it->strengthOptimum;
            }
        }
        m_cleanworkq.clear();
        ASKAPLOG_INFO_STR(logger, "All results have been received. Continuing...");

        /////////////////////////////////////////////////////////////
        // At this point the remote aspects of the Clean are finished
        /////////////////////////////////////////////////////////////

        ASKAPLOG_INFO_STR(logger, "Peak flux of the clean image " << max(cleanArray));

        const std::string peakResParam = std::string("peak_residual.") + indit->first;
        if (params.has(peakResParam)) {
            params.update(peakResParam, strengthOptimum);
        } else {
            params.add(peakResParam, strengthOptimum);
        }
        params.fix(peakResParam);

        casa::convertArray<double, float>(params.value(indit->first), cleanArray);

    }

    quality.setDOF(nParameters);
    quality.setRank(0);
    quality.setCond(0.0);
    quality.setInfo("Multiscale Clean");

    /// Save the PSF and Weight
    mssolver_p->saveWeights();
    mssolver_p->savePSF();

   *m_model_p = mssolver_p->parameters();

    return;
}

void SolverMaster::processCleanResponse(void)
{
    int patchid;
    casa::Array<float> patch;
    double strengthOptimum;
    m_comms.recvCleanResponse(patchid, patch, strengthOptimum);

    m_cleanworkq[patchid].model->assign(patch);
    m_cleanworkq[patchid].done = true;
    m_cleanworkq[patchid].strengthOptimum = strengthOptimum;
    ASKAPLOG_INFO_STR(logger, "Received CleanResponse for patchid " << patchid);
}

bool SolverMaster::outstanding(void)
{
    std::vector<CleanerWork>::iterator it;
    for (it = m_cleanworkq.begin() ; it < m_cleanworkq.end(); ++it) {
        ASKAPLOG_INFO_STR(logger, "Patchid " << it->patchid << " status: " << it->done);
        if (it->done != true) {
            return true;
        }
    }

    return false;
}
