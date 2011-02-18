/// @copyright (c) 2007 CSIRO
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

#include <askap_synthesis.h>
#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, ".measurementequation.imageamsmfsolver");

#include <askap/AskapError.h>

#include <casa/aips.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/Arrays/Vector.h>
#include <measurementequation/SynthesisParamsHelper.h>
#include <measurementequation/ImageParamsHelper.h>
#include <utils/MultiDimArrayPlaneIter.h>

#include <deconvolution/DeconvolverMultiTermBasisFunction.h>

#include <lattices/Lattices/LatticeCleaner.h>
#include <lattices/Lattices/MultiTermLatticeCleaner.h>
#include <lattices/Lattices/ArrayLattice.h>

#include <measurementequation/ImageAMSMFSolver.h>

using namespace casa;
using namespace askap;
using namespace askap::scimath;

#include <iostream>

#include <cmath>
using std::abs;

#include <map>
#include <vector>
#include <string>
#include <set>

using std::map;
using std::vector;
using std::string;

namespace askap
{
  namespace synthesis
  {
    
    
    ImageAMSMFSolver::ImageAMSMFSolver() : itsScales(3,0.), itsNumberTaylor(0),
					   itsSolutionType("R5"), itsDecoupleTerms(false)
    {
      ASKAPDEBUGASSERT(itsScales.size() == 3);
      itsScales(1)=10;
      itsScales(2)=30;
      // Now set up controller
      itsControl = boost::shared_ptr<DeconvolverControl<Float> >(new DeconvolverControl<Float>());
      // Now set up monitor
      itsMonitor = boost::shared_ptr<DeconvolverMonitor<Float> >(new DeconvolverMonitor<Float>());
    }
    
    ImageAMSMFSolver::ImageAMSMFSolver(const casa::Vector<float>& scales) : 
      itsScales(scales), itsNumberTaylor(0), 
      itsSolutionType("R5"), itsDecoupleTerms(false)
    {
      // Now set up controller
      itsControl = boost::shared_ptr<DeconvolverControl<Float> >(new DeconvolverControl<Float>());
      // Now set up monitor
      itsMonitor = boost::shared_ptr<DeconvolverMonitor<Float> >(new DeconvolverMonitor<Float>());
    }
    
    Solver::ShPtr ImageAMSMFSolver::clone() const
    {
      return Solver::ShPtr(new ImageAMSMFSolver(*this));
    }
    
    void ImageAMSMFSolver::init()
    {
      resetNormalEquations();
    }
    
    /// @brief Solve for parameters
    /// The solution is constructed from the normal equations
    /// The solution is constructed from the normal equations. The parameters named 
    /// image* are interpreted as images and solved for.
    /// @param[in] ip current model (to be updated)        
    /// @param[in] quality Solution quality information
    bool ImageAMSMFSolver::solveNormalEquations(askap::scimath::Params& ip,askap::scimath::Quality& quality)
    {
      
      // Solving A^T Q^-1 V = (A^T Q^-1 A) P
      
      // Find all the free parameters beginning with image
      vector<string> names(ip.completions("image"));
      for (vector<string>::iterator it = names.begin(); it!=names.end(); ++it) {
	*it = "image" + *it;
      }
      // this should work for faceting as well, taylorMap would contain one element
      // per facet in this case
      std::map<std::string, int> taylorMap;
      SynthesisParamsHelper::listTaylor(names, taylorMap);
      
      uint nParameters=0;
      ASKAPCHECK(taylorMap.size() != 0, "Solver doesn't have any images to solve for");
      for (std::map<std::string, int>::const_iterator tmIt = taylorMap.begin(); 
           tmIt!=taylorMap.end(); ++tmIt) {
	
	// The MSMF Solver expects 2xNTaylor-1 image parameter for each Stokes parameter.
	// 
	// Desired loop structure for multiple stokes and Taylor terms.
	// For image.i loop over Taylor 0,1,2,...
	// For image.q loop over Taylor 0,1,2,...
	// ...
	//
	// stokeslist = ['i','q']
	// ntaylor = 3
	//
	// for ( stokes in stokeslist )
	//    latticecleaner[stokes]->setup();
	//    for ( order in [0:(2*ntaylor-1)-1] )
	//        latticecleaner[stokes]->setpsf(order,psf[order]);
	//        if ( order < ntaylor )
	//           latticecleaner[stokes]->setresidual(order,residual[order]);
	//           latticecleaner[stokes]->setmodel(order,model[order]);
	//    latticecleaner[stokes]->mtclean();
	//    for ( order in [0:ntaylor-1] )
	//        latticecleaner[stokes]->getmodel(order,model[order]);
	//
	
	
	try {
	  ASKAPLOG_INFO_STR(logger, "AMSMFS minor cycle, processing image "<<tmIt->first);
	  // Determine the number of stokes planes and ensuring that all Taylor terms
	  // have the same number of polarisations
	  ASKAPDEBUGASSERT(tmIt->second != 0);
	  // this can be a facet, hence create a helper
	  ImageParamsHelper iph(tmIt->first);
	  // make it 0-order Taylor term
	  if(this->itsNumberTaylor>1) {
	    iph.makeTaylorTerm(0);
	  }
	  const casa::IPosition imageShape = ip.value(iph.paramName()).shape();               
	  const uint nPol = imageShape.nelements()>=3 ? uint(imageShape(2)) : 1;
	  ASKAPLOG_INFO_STR(logger, "There are " << nPol << " polarisation planes to solve for." );
	  nParameters += imageShape.product(); // add up the number of pixels for zero order
	  // check consistency
	  for (uInt order=1;order<uInt(tmIt->second);++order) {
	    // make the helper a Taylor term of the given order
	    iph.makeTaylorTerm(order);
	    const casa::IPosition thisShape = ip.value(iph.paramName()).shape();               
	    const uint thisNPol = thisShape.nelements()>=3 ? uint(thisShape(2)) : 1;
	    ASKAPCHECK(thisNPol == nPol, "Number of polarisations are supposed to be consistent for all Taylor terms, order="<<
		       order<<" has "<<thisNPol<<" polarisation planes");
	    nParameters += thisShape.product(); // add up the number of pixels for this order
	  }
	  
	  // this check is temporary, to avoid unnecessary surprises while further developing the code
	  if (imageShape.nelements()>=4) {
	    ASKAPCHECK(imageShape(3) == 1, "Output cube for MSMFS solver should have just one spectral plane, shape="<<
		       imageShape<<" nPol="<<nPol);
	  }
	  //
	  
	  // as polarisations are not necessarily represented by a different parameter
	  // we have to build a set of parameters which are going to be fixed inside the loop
	  // (or alternatively fix them multiple times, which is also a reasonable solution)
	  std::set<std::string> parametersToBeFixed;
	  
	  // Iterate through Polarisations (former sindex)
	  for (scimath::MultiDimArrayPlaneIter planeIter(imageShape); planeIter.hasMore(); planeIter.next()) {
	    const uint plane = planeIter.sequenceNumber();
	    ASKAPDEBUGASSERT(plane<nPol);
	    std::string tagLogString(planeIter.tag());
	    if (tagLogString.size()) {
	      tagLogString = "tagged as " + tagLogString;
	    } else {
	      tagLogString = "not tagged";
	    }
	    
	    ASKAPLOG_INFO_STR(logger, "Preparing iteration for polarisation " 
			      << plane<<" ("<<tagLogString<<") in image "<<tmIt->first);
	    // make the helper a 0-order Taylor term
	    if(this->itsNumberTaylor>1) {
	      iph.makeTaylorTerm(0);
	    }
	    const std::string zeroOrderParam = iph.paramName();
	    
	    // Setup the normalization vector	  
	    ASKAPLOG_INFO_STR(logger, "Reading the normalization vector from : " << zeroOrderParam);
	    ASKAPCHECK(normalEquations().normalMatrixDiagonal().count(zeroOrderParam)>0, "Diagonal not present");
	    casa::Vector<double> normdiag(normalEquations().normalMatrixDiagonal().find(zeroOrderParam)->second);
	    
	    ASKAPDEBUGASSERT(planeIter.planeShape().nelements()>=2);
	    
	    const double maxDiag = casa::max(planeIter.getPlaneVector(normdiag));
	    ASKAPLOG_INFO_STR(logger, "Maximum of weights = " << maxDiag );
	    
	    // a unique string for every Taylor decomposition (unique for every facet for faceting)
	    const std::string imageTag = tmIt->first + planeIter.tag();
	    
	    // nOrders is the number of free parameters
	    const uInt nOrders = tmIt->second;

	    // nOrders is the total number of free parameters. Initially this is 2 * nTaylor - 1.
	    // This will not work correctly if the number of terms differs between images!
	    const bool firstcycle = !SynthesisParamsHelper::hasValue(itsCleaners,imageTag);          
	    if(itsNumberTaylor==0) {
	      itsNumberTaylor=(nOrders+1)/2;
	      ASKAPLOG_INFO_STR(logger, "There are " << itsNumberTaylor << " Taylor terms");
	      ASKAPLOG_INFO_STR(logger, "There are " << nOrders << " PSFs calculated for this first pass");
	    }
	    else {
	      ASKAPLOG_INFO_STR(decmtbflogger, "There are " << itsNumberTaylor << " Taylor terms");
	    }

	    Vector<Array<Float> > cleanVec(itsNumberTaylor);

	    Vector<Array<Float> > dirtyVec(itsNumberTaylor);
	    Vector<Array<Float> > dirtyLongVec(2*itsNumberTaylor-1);
	    Vector<Array<Float> > psfVec(itsNumberTaylor);
	    Vector<Array<Float> > psfLongVec(2*itsNumberTaylor-1);
	    
	    // check whether a particular tag has been encountered for the first time
	    
	    // Setup the PSFs - all ( 2 x ntaylor - 1 ) of them for the first time.
	    casa::Array<float> psfZeroArray(planeIter.planeShape());
	    
	    // buffer for the peak of zero-order PSF

	    float zeroPSFPeak = -1;
	    uInt limit=this->itsNumberTaylor;
	    if(firstcycle) limit=2*this->itsNumberTaylor-1;
	    for( uInt order=0; order < limit; ++order) {
	      // make helper to represent the given order
	      if(this->itsNumberTaylor>1) {
		iph.makeTaylorTerm(order);
	      }
	      const std::string thisOrderParam = iph.paramName();
	      ASKAPLOG_INFO_STR(logger, "AMSMFS solver: processing order "
				<< order << " (" << itsNumberTaylor <<
				" Taylor terms + " << itsNumberTaylor-1 << " cross-terms), parameter name: " << thisOrderParam);

	      // Always get the PSF for all orders
	      if ((order<this->itsNumberTaylor)||(firstcycle)) {
		ASKAPCHECK(normalEquations().normalMatrixSlice().count(thisOrderParam)>0,
			   "PSF Slice for plane="<<
			   plane<<" and order="<<order<<" is not present");
		casa::Vector<double> slice(normalEquations().normalMatrixSlice().find(thisOrderParam)->second);
		psfLongVec(order).resize(planeIter.planeShape());
		casa::convertArray<float, double>(psfLongVec(order), planeIter.getPlane(slice));
	      }
	      
	      // For the dirty images, we need only the first nTaylor
	      ASKAPCHECK(normalEquations().dataVector(thisOrderParam).size()>0,
			 "Data vector not present for cube plane="<<
			 plane<<" and order="<<order);
	      dirtyLongVec(order).resize(planeIter.planeShape());
	      casa::Vector<double> dv = normalEquations().dataVector(thisOrderParam);
	      casa::convertArray<float, double>(dirtyLongVec(order), planeIter.getPlane(dv));

	      // For the clean images, we need only the first nTaylor
	      if(order<this->itsNumberTaylor) {
		cleanVec(order).resize(planeIter.planeShape());
		casa::convertArray<float, double>(cleanVec(order), 
						  planeIter.getPlane(ip.value(thisOrderParam)));
	      }
	    }

	    // Now precondition the residual images using the zeroth order psf. We need to 
	    // keep a copy of the zeroth PSF to avoid having it overwritten each time.
	    Array<float> psfWorkArray;
	    psfZeroArray=psfLongVec(0).copy();
	    zeroPSFPeak=max(psfZeroArray);

	    casa::Array<float> maskArray(planeIter.planeShape());

	    if(firstcycle) {
	      // For the first cycle we need to precondition and normalise all PSFs and all dirty images
	      for(uInt order=0; order < 2 * itsNumberTaylor - 1; ++order) {
		// Now precondition each PSF by the PSF for term 0.
		ASKAPLOG_INFO_STR(logger, "Preconditioning PSF for plane=" << plane<< "("<<tagLogString<< ") and order=" << order);
		// We need to work with the original preconditioning PSF since it gets overridden
		psfWorkArray = psfZeroArray.copy();
		doPreconditioning(psfWorkArray,psfLongVec(order));
		// Now we can precondition the dirty (residual) array
		psfWorkArray = psfZeroArray.copy();
		if(doPreconditioning(psfWorkArray,dirtyLongVec(order))) {
		  ASKAPLOG_INFO_STR(logger, "Preconditioning dirty image for plane=" << plane<<
				    " ("<<tagLogString<< ") and order=" << order);
		}
		// Normalise. We need to normalise the PSF only. The dirty vector is along for the ride.
		doNormalization(padDiagonal(planeIter.getPlane(normdiag)),tol(),psfLongVec(order),zeroPSFPeak,dirtyLongVec(order),
				boost::shared_ptr<casa::Array<float> >(&maskArray, utility::NullDeleter()));
	      }
	    }
	    else {
	      // For the subsequent cycles cycle we need to precondition and normalise the updated dirty images
	      for(uInt order=0; order < itsNumberTaylor; ++order) {
		// Precondition the dirty (residual) array
		psfWorkArray = psfZeroArray.copy();
		if(doPreconditioning(psfWorkArray,dirtyLongVec(order))) {
		  ASKAPLOG_INFO_STR(logger, "Preconditioning dirty image for plane=" << plane<<
				    " ("<<tagLogString<< ") and order=" << order);
		}
		// Normalise. We need to normalise the PSF only. The dirty vector is along for the ride.
		doNormalization(padDiagonal(planeIter.getPlane(normdiag)),tol(),psfLongVec(order),zeroPSFPeak,dirtyLongVec(order),
				boost::shared_ptr<casa::Array<float> >(&maskArray, utility::NullDeleter()));
	      // Store the new PSF in parameter class to be saved to disk later
	      saveArrayIntoParameter(ip, tmIt->first, planeIter.shape(), "psf.image",
				     unpadImage(psfLongVec(order)), planeIter.position());
	      }
	    }

	    // The deconvolver only needs the first itsNumberTaylor elements so we
	    // copy only the ones we need. Since casa::Array is by reference
	    // the overhead is minimal.
	    for(uInt order=0; order < psfVec.nelements(); ++order) {
	      psfVec(order)=psfLongVec(order);
	      dirtyVec(order)=dirtyLongVec(order);
	    }

	    ASKAPLOG_INFO_STR(logger, "Create or update the deconvolver");

	    // Now that we have all the required images, we can initialise the deconvolver
	    for(uInt order=0; order < itsNumberTaylor; ++order) {

	      // This takes up some memory and we have to ship the residual image out inside
	      // the parameter class. Therefore, we may not need this functionality in the 
	      // production version (or may need to implement it in a different way).
	      saveArrayIntoParameter(ip, tmIt->first, planeIter.shape(), "residual",
				     unpadImage(dirtyVec(order)), planeIter.position());
	    
	      // uncomment the code below to save the mask
	      if(order==0) {
		saveArrayIntoParameter(ip, tmIt->first, planeIter.shape(), "mask",
				       unpadImage(maskArray),
				       planeIter.position());
	      }
	    
	      if(firstcycle)  {// Initialize everything only once.
		ASKAPLOG_INFO_STR(logger, "Initialising the solver for plane " << plane
				  <<" tag "<<imageTag);
		itsCleaners[imageTag].reset(new DeconvolverMultiTermBasisFunction<Float, Complex>(dirtyVec, psfVec, psfLongVec));
		ASKAPDEBUGASSERT(itsCleaners[imageTag]);

		itsCleaners[imageTag]->setMonitor(itsMonitor);
		itsCleaners[imageTag]->setControl(itsControl);
		
		itsBasisFunction->initialise(dirtyVec(0).shape());
		itsCleaners[imageTag]->setBasisFunction(itsBasisFunction);
		itsCleaners[imageTag]->setSolutionType(itsSolutionType);
		itsCleaners[imageTag]->setDecouple(itsDecoupleTerms);
		itsCleaners[imageTag]->setMask(maskArray, order);
	      }
	      else {
		// Update the dirty images
		ASKAPLOG_INFO_STR(logger, "Multi-Term Basis Function deconvolver already exists - update dirty images");
		itsCleaners[imageTag]->updateDirty(dirtyVec);
		ASKAPLOG_INFO_STR(logger, "Successfully updated dirty images");
	      }
	      // Initialise the model
	      Array<float> initialModel(cleanVec(order).nonDegenerate().shape());
	      initialModel.set(0.0);
	      itsCleaners[imageTag]->setBackground(initialModel, order);
		
	      // We have to reset the initial objective function
	      // so that the fractional threshold mechanism will work.
	      itsCleaners[imageTag]->state()->resetInitialObjectiveFunction();
	      // By convention, iterations are counted from scratch each
	      // major cycle
	      itsCleaners[imageTag]->state()->setCurrentIter(0);
	      
	    } // end of 'order' loop
	    
	    ASKAPLOG_INFO_STR(logger, "Starting Minor Cycles" );
	    itsCleaners[imageTag]->deconvolve();
	    ASKAPLOG_INFO_STR(logger, "Finished Minor Cycles." );
	    
	    // Write the final vector of clean model images into parameters
	    for( uInt order=0; order < itsNumberTaylor; ++order) {
	      // make the helper to correspond to the given order
	      if(this->itsNumberTaylor>1) {
		iph.makeTaylorTerm(order);
	      }
	      const std::string thisOrderParam = iph.paramName();
	      ASKAPLOG_INFO_STR(logger, "About to get model for plane="<<plane<<" Taylor order="<<order<<
				" for image "<<tmIt->first);
	      casa::Array<double> slice = planeIter.getPlane(ip.value(thisOrderParam)).nonDegenerate();
	      slice=unpadImage(itsCleaners[imageTag]->model(order)+itsCleaners[imageTag]->background(order));
	    }
	    // add extra parameters (cross-terms) to the to-be-fixed list
	    for (uInt order = itsNumberTaylor; order<uInt(tmIt->second); ++order) {
	      // make the helper to correspond to the given order
	      if(this->itsNumberTaylor>1) {
		iph.makeTaylorTerm(order);
	      }
	      const std::string thisOrderParam = iph.paramName();
	      parametersToBeFixed.insert(thisOrderParam);
	    }
	  } // end of polarisation (i.e. plane) loop 
	  
	    // Make sure that the next set of minor cycles does not redo unnecessary things.
	    // Also "fix" parameters for order >= itsNumberTaylor. so that the gridding doesn't get done
	    // for these extra terms.
	  
	    // Fix the params corresponding to extra Taylor terms.
	    // (MV) probably this part needs another careful look
	  for (std::set<std::string>::const_iterator ci = parametersToBeFixed.begin(); 
	       ci != parametersToBeFixed.end(); ++ci) {
	    if (ip.isFree(*ci)) {
	      ip.fix(*ci);
	    }
	  }
	} // try
	catch( const AipsError &x ) {
	  throw AskapError("Failed in the AMSMFS Minor Cycle : " + x.getMesg() );
	}
      } // loop: tmIt
      
      ASKAPCHECK(nParameters>0, "No free parameters in ImageAMSMFSolver");
      
      quality.setDOF(nParameters);
      quality.setRank(0);
      quality.setCond(0.0);
      quality.setInfo("Multi-Scale Multi-Frequency Clean");
      
      /// Save PSFs and Weights into parameter class (to be exported later)
      saveWeights(ip);
      savePSF(ip);
      
      return true;
    };
    
    void ImageAMSMFSolver::setBasisFunction(BasisFunction<Float>::ShPtr bf) {
      itsBasisFunction=bf;
    }
    
    BasisFunction<Float>::ShPtr ImageAMSMFSolver::basisFunction() {
      return itsBasisFunction;
    }
    
    void ImageAMSMFSolver::configure(const LOFAR::ParameterSet &parset) {
      
      ImageSolver::configure(parset);
      
      if(parset.getString("algorithm")=="AMSMFS") {
	ASKAPASSERT(this->itsMonitor);
	this->itsMonitor->configure(parset);
	ASKAPASSERT(this->itsControl);
	this->itsControl->configure(parset);
	
	// Make the basis function
	std::vector<float> defaultScales(3);
	defaultScales[0]=0.0;
	defaultScales[1]=10.0;
	defaultScales[2]=30.0;
	std::vector<float> scales=parset.getFloatVector("scales", defaultScales);
	itsBasisFunction=BasisFunction<Float>::ShPtr(new MultiScaleBasisFunction<Float>(scales));

	String solutionType=parset.getString("solutiontype", "MAXBASE");
	if(solutionType=="R5") {
	}
	else if(solutionType=="MAXTERM0") {
	}
	else {
	  solutionType="MAXBASE";
	}
	ASKAPLOG_INFO_STR(decmtbflogger, "Solution type = " << solutionType);
	this->itsSolutionType=solutionType;

        this->itsDecoupleTerms=parset.getBool("decouple", "true");
	if(itsDecoupleTerms) {
	  ASKAPLOG_INFO_STR(decmtbflogger, "Decoupling in term using the inverse of the coupling matrix");
	}
      }
    }
  }
}
