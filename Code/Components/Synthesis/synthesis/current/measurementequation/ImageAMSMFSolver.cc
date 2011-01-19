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
ASKAP_LOGGER(logger, ".measurementequation");

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
    
    
    ImageAMSMFSolver::ImageAMSMFSolver() : 
      itsScales(3,0.)
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
      itsScales(scales)
    {
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
	  ASKAPLOG_INFO_STR(logger, " AMSMFS minor cycle, processing image "<<tmIt->first);
	  // Determine the number of stokes planes and ensuring that all Taylor terms
	  // have the same number of polarisations
	  ASKAPDEBUGASSERT(tmIt->second != 0);
	  // this can be a facet, hence create a helper
	  ImageParamsHelper iph(tmIt->first);
	  // make it 0-order Taylor term
	  iph.makeTaylorTerm(0);
	  const casa::IPosition imageShape = ip.value(iph.paramName()).shape();               
	  const uint nPol = imageShape.nelements()>=3 ? uint(imageShape(2)) : 1;
	  ASKAPLOG_INFO_STR(logger, "There are " << nPol << " polarisation planes to solve for." );
	  nParameters += imageShape.product(); // add up the number of pixels for zero order
	  // check consistency
	  for (int order=1;order<tmIt->second;++order) {
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
	    iph.makeTaylorTerm(0);	                
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
	    
	    // number of terms is the number of free parameters
	    const int nOrders = tmIt->second;
	    
	    Vector<Array<Float> > dirtyVec(nOrders);
	    Vector<Array<Float> > psfVec(nOrders);
	    Vector<Array<Float> > cleanVec(nOrders);
	    Vector<Array<Float> > psfShortVec((nOrders+1)/2);
	    
	    // check whether a particular tag has been encountered for the first time
	    
	    // Setup the PSFs - all ( 2 x ntaylor - 1 ) of them for the first time.
	    casa::Array<float> psfZeroArray(planeIter.planeShape());
	    
	    // buffer for the peak of zero-order PSF
	    int nTaylor = nOrders;

	    float zeroPSFPeak = -1;
	    for( int order=0; order < nOrders; ++order) {
	      // make helper to represent the given order
	      iph.makeTaylorTerm(order);
	      const std::string thisOrderParam = iph.paramName();
	      ASKAPLOG_INFO_STR(logger, "AMSMFS solver: processing order " << order << " (" << nTaylor <<
				" Taylor terms + " << nTaylor-1 << " cross-terms), parameter name: " << thisOrderParam);
	      ASKAPCHECK(normalEquations().normalMatrixSlice().count(thisOrderParam)>0, "PSF Slice for plane="<<
			 plane<<" and order="<<order<<" is not present");
	      casa::Vector<double> slice(normalEquations().normalMatrixSlice().find(thisOrderParam)->second);
	      ASKAPCHECK(normalEquations().dataVector(thisOrderParam).size()>0, "Data vector not present for cube plane="<<
			 plane<<" and order="<<order);
	      casa::Vector<double> dv = normalEquations().dataVector(thisOrderParam);
	      
	      psfVec(order).resize(planeIter.planeShape());
	      casa::convertArray<float, double>(psfVec(order), planeIter.getPlane(slice));
	      dirtyVec(order).resize(planeIter.planeShape());
	      casa::convertArray<float, double>(dirtyVec(order), planeIter.getPlane(dv));
	      cleanVec(order).resize(planeIter.planeShape());
	      casa::convertArray<float, double>(cleanVec(order), 
						planeIter.getPlane(ip.value(thisOrderParam)));
	      
	      if (order == 0) {
		psfZeroArray = psfVec(order).copy();
	      }
	      
	      if( doPreconditioning(psfZeroArray,psfVec(order)) ) {
		// Write PSFs to disk.
		ASKAPLOG_INFO_STR(logger, "Exporting preconditioned psfs (to be stored to disk later)");
		Axes axes(ip.axes(thisOrderParam));
		const std::string psfName="psf."+thisOrderParam;
		casa::Array<double> aargh(planeIter.planeShape());
		casa::convertArray<double,float>(aargh,psfVec(order));
		const casa::Array<double> & APSF(aargh);
		if (!ip.has(psfName)) {
		  // create an empty parameter with the full shape
		  ip.add(psfName, planeIter.shape(), axes);
		}
		// insert the slice at the proper place  
		ip.update(psfName, APSF, planeIter.position());                           
	      }
	      
	      ASKAPLOG_INFO_STR(logger, "Preconditioning PSF for plane=" << plane<<
				" ("<<tagLogString<< ") and order=" << order <<
				" parameter name "<<thisOrderParam);
	      
	      if (order == 0) {
		zeroPSFPeak = doNormalization(planeIter.getPlaneVector(normdiag),tol(),psfVec(order),dirtyVec(order));
	      } else {
		ASKAPDEBUGASSERT(zeroPSFPeak > 0.);
		doNormalization(planeIter.getPlaneVector(normdiag),tol(),psfVec(order),zeroPSFPeak,dirtyVec(order));
	      }

	    }
	    // Now that we have all the required images, we can initialise the deconvolver
	    for(int order=0; order < nTaylor; ++order) {
	      // Now precondition the residual images
	      doPreconditioning(psfZeroArray,dirtyVec(order));
	      psfShortVec(order)=psfVec(order);
	    }

	    for(int order=0; order < nTaylor; ++order) {
	      const bool firstcycle = !SynthesisParamsHelper::hasValue(itsCleaners,imageTag);          
	      if(firstcycle)  {// Initialize everything only once.
		// have to remove cross-terms from nTaylor
		ASKAPCHECK(nTaylor % 2 == 1, "Number of free Taylor terms is supposed to be odd at first cycle, you have "<<
			   nTaylor);
		nTaylor = (nTaylor + 1) / 2; // cross-terms are now excluded           
		// Initialize the latticecleaners
		ASKAPLOG_INFO_STR(logger, "Initialising the solver for plane " << plane<<" tag "<<imageTag);
		itsCleaners[imageTag].reset(new DeconvolverMultiTermBasisFunction<Float, Complex>(dirtyVec, psfShortVec));
		ASKAPDEBUGASSERT(itsCleaners[imageTag]);
		// We need to initialise the term-term coupling matrix
		itsCleaners[imageTag]->calculateTermCoupling(psfVec);
	      }
	      else {
		// Update the dirty images
		itsCleaners[imageTag]->updateDirty(dirtyVec(order), order);
	      }
	      // Initialise the model
	      itsCleaners[imageTag]->setModel(cleanVec(order),order);
	    } // end of 'order' loop
	    
	    ASKAPLOG_INFO_STR(logger, "Starting Minor Cycles" );
	    itsCleaners[imageTag]->deconvolve();
	    ASKAPLOG_INFO_STR(logger, "Finished Minor Cycles." );
	    
	    // Write the final vector of clean model images into parameters
	    for( int order=0; order < nTaylor; ++order) {
	      // make the helper to correspond to the given order
	      iph.makeTaylorTerm(order);
	      const std::string thisOrderParam = iph.paramName();
	      casa::Array<float> cleanArray(planeIter.planeShape());
	      ASKAPLOG_INFO_STR(logger, "About to get model for plane="<<plane<<" Taylor order="<<order<<
				" for image "<<tmIt->first);
	      casa::Array<double> slice = planeIter.getPlane(ip.value(thisOrderParam));
	      casa::convertArray<double, float>(slice, cleanArray);
	    }
	    // add extra parameters (cross-terms) to the to-be-fixed list
	    for (int order = nTaylor; order<tmIt->second; ++order) {
	      // make the helper to correspond to the given order
	      iph.makeTaylorTerm(order);
	      const std::string thisOrderParam = iph.paramName();
	      parametersToBeFixed.insert(thisOrderParam);
	    }
	  } // end of polarisation (i.e. plane) loop 
	  
	    // Make sure that the next set of minor cycles does not redo unnecessary things.
	    // Also "fix" parameters for order >= nTaylor. so that the gridding doesn't get done
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
	  throw AskapError("Failed in the MSMFS Minor Cycle : " + x.getMesg() );
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
	itsUseCrossTerms=parset.getBool("usecrossterms", true);
      }
    }
  }
}
