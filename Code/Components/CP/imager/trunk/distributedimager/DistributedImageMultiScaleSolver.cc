/// @file DistributedImageMultiScaleSolver.h
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

// Include own header file first
#include <distributedimager/DistributedImageMultiScaleSolver.h>

// System includes
#include <iostream>
#include <cmath>
#include <map>
#include <vector>
#include <string>

// ASKAPsoft includes
#include <askap_imager.h>
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <askap/AskapUtil.h>
#include <casa/aips.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/Arrays/Vector.h>
#include <lattices/Lattices/LatticeCleaner.h>
#include <lattices/Lattices/ArrayLattice.h>

// Local includes
#include <distributedimager/IBasicComms.h>

// Using
using namespace casa;
using namespace askap;
using namespace askap::scimath;
using namespace askap::cp;

ASKAP_LOGGER(logger, ".DistributedImageMultiScaleSolver");

DistributedImageMultiScaleSolver::DistributedImageMultiScaleSolver(const askap::scimath::Params& ip,
        const LOFAR::ACC::APS::ParameterSet& parset,
        askap::cp::IBasicComms& comms)
    : ImageCleaningSolver(ip), m_parset(parset), m_comms(comms) 
{
    itsScales.resize(3);
    itsScales(0) = 0;
    itsScales(1) = 10;
    itsScales(2) = 30;
}

DistributedImageMultiScaleSolver::DistributedImageMultiScaleSolver(const askap::scimath::Params& ip,
        const casa::Vector<float>& scales,
        const LOFAR::ACC::APS::ParameterSet& parset,
        askap::cp::IBasicComms& comms)
    : ImageCleaningSolver(ip), m_parset(parset), m_comms(comms)
{
    itsScales.resize(scales.size());
    itsScales = scales;
}

void DistributedImageMultiScaleSolver::init()
{
    resetNormalEquations();
}

// Solve for update simply by scaling the data vector by the diagonal term of the
// normal equations i.e. the residual image
bool DistributedImageMultiScaleSolver::solveNormalEquations(askap::scimath::Quality& quality)
{
    // Solving A^T Q^-1 V = (A^T Q^-1 A) P
    uint nParameters=0;

    // Find all the free parameters beginning with image
    std::vector<std::string> names(itsParams->completions("image"));
    std::map<std::string, uint> indices;

    for (std::vector<std::string>::const_iterator  it=names.begin();it!=names.end();it++)
    {
        std::string name="image"+*it;
        if(itsParams->isFree(name)) {
            indices[name]=nParameters;
            nParameters+=itsParams->value(name).nelements();
        }
    }
    ASKAPCHECK(nParameters>0, "No free parameters in ImageMultiScaleSolver");

    for (std::map<std::string, uint>::const_iterator indit=indices.begin(); indit!=indices.end(); ++indit)
    {
        // Axes are dof, dof for each parameter
        const casa::IPosition vecShape(1, itsParams->value(indit->first).nelements());
        const casa::IPosition valShape(itsParams->value(indit->first).shape());

        ASKAPCHECK(normalEquations().normalMatrixDiagonal().count(indit->first)>0, "Diagonal not present");
        const casa::Vector<double>& diag(normalEquations().normalMatrixDiagonal().find(indit->first)->second);
        ASKAPCHECK(normalEquations().dataVector(indit->first).size()>0, "Data vector not present");
        const casa::Vector<double>& dv = normalEquations().dataVector(indit->first);
        ASKAPCHECK(normalEquations().normalMatrixSlice().count(indit->first)>0, "PSF Slice not present");
        const casa::Vector<double>& slice(normalEquations().normalMatrixSlice().find(indit->first)->second);

        casa::Array<float> dirtyArray(valShape);
        casa::convertArray<float, double>(dirtyArray, dv.reform(valShape));
        casa::Array<float> psfArray(valShape);
        casa::convertArray<float, double>(psfArray, slice.reform(valShape));
        casa::Array<float> cleanArray(valShape);
        casa::convertArray<float, double>(cleanArray, itsParams->value(indit->first));
        casa::Array<float> maskArray(valShape);

        // Normalize
        doNormalization(diag,tol(),psfArray,dirtyArray,
                boost::shared_ptr<casa::Array<float> >(&maskArray, utility::NullDeleter()));

        // Precondition the PSF and DIRTY images before solving.
        if(doPreconditioning(psfArray,dirtyArray)) {
            // Save the new PSFs to disk
            Axes axes(itsParams->axes(indit->first));
            std::string psfName="psf."+(indit->first);
            casa::Array<double> anothertemp(valShape);
            casa::convertArray<double,float>(anothertemp,psfArray);
            const casa::Array<double> & APSF(anothertemp);
            if (!itsParams->has(psfName)) {
                itsParams->add(psfName, APSF, axes);
            } else {
                itsParams->update(psfName, APSF);
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
           Axes axes(itsParams->axes(indit->first));
           ASKAPDEBUGASSERT(indit->first.find("image")==0);
           ASKAPCHECK(indit->first.size()>5,
                   "Image parameter name should have something appended to word image")
           const string residName="residual"+indit->first.substr(5);
           casa::Array<double> anothertemp(valShape);
           casa::convertArray<double,float>(anothertemp,dirtyArray);
           const casa::Array<double> & AResidual(anothertemp);
           if (!itsParams->has(residName)) {
               itsParams->add(residName, AResidual, axes);
           } else {
               itsParams->update(residName, AResidual);
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

            std::vector<float> stdscales;
            for (unsigned int i = 0; i < itsScales.size(); ++i) {
                stdscales.push_back(itsScales[i]);
            }
            m_comms.sendCleanRequest(patchid,
                    dirtyPatch,
                    psfCenter.get(),
                    maskPatch,
                    *(cleanPatch),
                    threshold().getValue(),
                    threshold().getUnit(),
                    fractionalThreshold(),
                    stdscales,
                    niter(),
                    gain(),
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
        if (itsParams->has(peakResParam)) {
            itsParams->update(peakResParam, strengthOptimum);
        } else {
            itsParams->add(peakResParam, strengthOptimum);
        }
        itsParams->fix(peakResParam);

        casa::convertArray<double, float>(itsParams->value(indit->first), cleanArray);

    }

    quality.setDOF(nParameters);
    quality.setRank(0);
    quality.setCond(0.0);
    quality.setInfo("Multiscale Clean");

    /// Save the PSF and Weight
    saveWeights();
    savePSF();

    return true;
};

Solver::ShPtr DistributedImageMultiScaleSolver::clone() const
{
    return Solver::ShPtr(new DistributedImageMultiScaleSolver(*this));
}


void DistributedImageMultiScaleSolver::processCleanResponse(void)
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

bool DistributedImageMultiScaleSolver::outstanding(void)
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

