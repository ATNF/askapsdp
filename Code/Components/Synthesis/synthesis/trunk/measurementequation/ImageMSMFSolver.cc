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

#include <measurementequation/ImageMSMFSolver.h>

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

#include <lattices/Lattices/LatticeCleaner.h>
#include <lattices/Lattices/MultiTermLatticeCleaner.h>
#include <lattices/Lattices/ArrayLattice.h>

using namespace casa;
using namespace askap;
using namespace askap::scimath;

#include <iostream>

#include <cmath>
using std::abs;

#include <map>
#include <vector>
#include <string>

using std::map;
using std::vector;
using std::string;

namespace askap
{
  namespace synthesis
  {
    
      
    ImageMSMFSolver::ImageMSMFSolver(const askap::scimath::Params& ip) : 
      ImageCleaningSolver(ip), itsNTaylor(2), itsDoSpeedUp(false), itsSpeedUpFactor(1.)
    {
      itsScales.resize(3);
      itsScales(0)=0;
      itsScales(1)=10;
      itsScales(2)=30;
      itsNPsfTaylor = 2*itsNTaylor-1;
      dbg=True;
    }

    ImageMSMFSolver::ImageMSMFSolver(const askap::scimath::Params& ip,
      const casa::Vector<float>& scales, const int& nterms) : 
          ImageCleaningSolver(ip),itsNTaylor(nterms)
    {
      itsScales.resize(scales.size());
      itsScales=scales;
      itsNPsfTaylor = 2*itsNTaylor-1;
      dbg=True;
    }
    
    Solver::ShPtr ImageMSMFSolver::clone() const
    {
      return Solver::ShPtr(new ImageMSMFSolver(*this));
    }
    
    void ImageMSMFSolver::init()
    {
      resetNormalEquations();
    }

    /// @brief switch the speed up on
    /// @param[in] factor speed up factor
    void ImageMSMFSolver::setSpeedUp(float factor)
    {
      itsDoSpeedUp = true;
      itsSpeedUpFactor = factor;
    }
              
    
// Solve for update simply by scaling the data vector by the diagonal term of the
// normal equations i.e. the residual image
    bool ImageMSMFSolver::solveNormalEquations(askap::scimath::Quality& quality)
    {

      // Solving A^T Q^-1 V = (A^T Q^-1 A) P
     
      // Find all the free parameters beginning with image
      vector<string> names(itsParams->completions("image"));
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
               ASKAPLOG_INFO_STR(logger, "MSMFS minor cycle, processing image "<<tmIt->first);
               // Determine the number of stokes planes and ensuring that all Taylor terms
               // have the same number of polarisations
               ASKAPDEBUGASSERT(tmIt->second != 0);
               // this can be a facet, hence create a helper
               ImageParamsHelper iph(tmIt->first);
               // make it 0-order Taylor term
               iph.makeTaylorTerm(0);
               const casa::IPosition imageShape = itsParams->value(iph.paramName()).shape();               
               const uint nPol = imageShape.nelements()>=3 ? uint(imageShape(2)) : 1;
               ASKAPLOG_INFO_STR(logger, "There are " << nPol << " polarisation planes to solve for." );
               nParameters += imageShape.product(); // add up the number of pixels for zero order
	           // check consistency
	           for (int order=1;order<tmIt->second;++order) {
	                // make the helper a Taylor term of the given order
	                iph.makeTaylorTerm(order);
                    const casa::IPosition thisShape = itsParams->value(iph.paramName()).shape();               
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
	 
	           static bool firstcycle=True;
	
	           // Iterate through Polarisations (former sindex)
	           for (scimath::MultiDimArrayPlaneIter planeIter(imageShape); planeIter.hasMore(); planeIter.next()) {
	                const uint plane = planeIter.sequenceNumber();
	                ASKAPDEBUGASSERT(plane<nPol);
	                ASKAPLOG_INFO_STR(logger, "In Image MSMFSSolver::solveN..E.. : About to iterate for polarisation " 
	                                  << plane<<" tagged as "<<planeIter.tag()<<" in image "<<tmIt->first);
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
          
                    if(firstcycle)  {// Initialize everything only once.
	                   // Initialize the latticecleaners
                       ASKAPLOG_INFO_STR(logger, "Initialising the solver for plane " << plane);
	    
                       itsCleaners[imageTag].reset(new casa::MultiTermLatticeCleaner<float>());
                       ASKAPDEBUGASSERT(itsCleaners[imageTag]);
	    
                       itsCleaners[imageTag]->setcontrol(casa::CleanEnums::MULTISCALE, niter(), gain(), threshold(), 
	                                      fractionalThreshold(), false);
                       itsCleaners[imageTag]->ignoreCenterBox(true);
                       itsCleaners[imageTag]->setscales(itsScales);
                       itsCleaners[imageTag]->setntaylorterms(itsNTaylor);
                       itsCleaners[imageTag]->initialise(planeIter.planeShape()[0],planeIter.planeShape()[1]); // allocates memory once....
	                }
          
                    // Setup the PSFs - all ( 2 x ntaylor - 1 ) of them for the first time.
                    int nOrders = itsNTaylor;
                    casa::Array<float> psfZeroArray(planeIter.planeShape());
                    if (firstcycle) {
                        nOrders = 2*itsNTaylor-1;
                    }
                    // temporary support only homogeneous number of Taylor terms
                    ASKAPCHECK(nOrders == tmIt->second, "Only homogeneous number of Taylor terms are supported");
                    // buffer for the peak of zero-order PSF
                    float zeroPSFPeak = -1;
                    for( int order=0; order < nOrders; ++order) {
                        // make helper to represent the given order
                        iph.makeTaylorTerm(order);
	                    const std::string thisOrderParam = iph.paramName();
                        ASKAPLOG_INFO_STR(logger, "MSMFS solver: processing order "<<order<<" ("<<itsNTaylor<<
                                          " Taylor terms + "<<itsNTaylor-1<<" cross-terms), parameter name: "<<thisOrderParam);
                        ASKAPCHECK(normalEquations().normalMatrixSlice().count(thisOrderParam)>0, "PSF Slice for plane="<<
                                   plane<<" and order="<<order<<" is not present");
                        casa::Vector<double> slice(normalEquations().normalMatrixSlice().find(thisOrderParam)->second);
                        ASKAPCHECK(normalEquations().dataVector(thisOrderParam).size()>0, "Data vector not present for cube plane="<<
                                   plane<<" and order="<<order);
                        casa::Vector<double> dv = normalEquations().dataVector(thisOrderParam);
	   
                        casa::Array<float> psfArray(planeIter.planeShape());
                        casa::convertArray<float, double>(psfArray, planeIter.getPlane(slice));
                        casa::Array<float> dirtyArray(planeIter.planeShape());
                        casa::convertArray<float, double>(dirtyArray, planeIter.getPlane(dv));
                        casa::Array<float> cleanArray(planeIter.planeShape());
                        casa::convertArray<float, double>(cleanArray, 
                                           planeIter.getPlane(itsParams->value(thisOrderParam)));
                        if (order == 0) {
                            zeroPSFPeak = doNormalization(planeIter.getPlaneVector(normdiag),tol(),psfArray,dirtyArray);
                        } else {
                            ASKAPDEBUGASSERT(zeroPSFPeak > 0.);
                            doNormalization(planeIter.getPlaneVector(normdiag),tol(),psfArray,zeroPSFPeak,dirtyArray);
                        }
	   	   
                        ASKAPLOG_INFO_STR(logger, "Preconditioning PSF for plane=" << plane<<
                                          " (tagged as "<<planeIter.tag() << ") and order=" << order <<
                                          " of the image "<<tmIt->first);

                        if (order == 0) {
                            psfZeroArray = psfArray.copy();
                        }
	    	   
                        if( doPreconditioning(psfZeroArray,psfArray) ) {
                           // Write PSFs to disk.
                           ASKAPLOG_INFO_STR(logger, "Exporting preconditioned psfs (to be stored to disk later)");
                           Axes axes(itsParams->axes(thisOrderParam));
                           string psfName="psf."+thisOrderParam;
                           casa::Array<double> aargh(planeIter.planeShape());
                           casa::convertArray<double,float>(aargh,psfArray);
                           const casa::Array<double> & APSF(aargh);
                           if (!itsParams->has(psfName)) {
                               // create an empty parameter with the full shape
                               itsParams->add(psfName, planeIter.shape(), axes);
                           }
                           // insert the slice at the proper place  
                           itsParams->update(psfName, APSF, planeIter.position());                           
	                    }
	   
                        casa::ArrayLattice<float> psf(psfArray);
                        itsCleaners[imageTag]->setpsf(order,psf);
	   
                        // Setup the Residual Images and Model Images  - ( ntaylor ) of them
                        if (order < itsNTaylor) {
                            // Now precondition the residual images
                            doPreconditioning(psfZeroArray,dirtyArray);
		   
                            // We need lattice equivalents. We can use ArrayLattice which involves
                            // no copying
                            casa::ArrayLattice<float> dirty(dirtyArray);
                            casa::ArrayLattice<float> clean(cleanArray);

                            // Send in Dirty images only for ntaylor terms
                            itsCleaners[imageTag]->setresidual(order,dirty);
                            itsCleaners[imageTag]->setmodel(order,clean);
                        }
	   
                    } // end of 'order' loop
	  
                    ASKAPLOG_INFO_STR(logger, "Starting Minor Cycles" );
                    itsCleaners[imageTag]->mtclean();
                    ASKAPLOG_INFO_STR(logger, "Finished Minor Cycles." );
	  
                    // Write the final vector of clean model images into parameters
                    for( int order=0; order < itsNTaylor; ++order) {
                        // make the helper to correspond to the given order
                        iph.makeTaylorTerm(order);
	                    const std::string thisOrderParam = iph.paramName();
                        casa::Array<float> cleanArray(planeIter.planeShape());
                        casa::ArrayLattice<float> clean(cleanArray);
                        ASKAPLOG_INFO_STR(logger, "About to get model for plane="<<plane<<" Taylor order="<<order<<
                                                  " for image "<<tmIt->first);
                        itsCleaners[imageTag]->getmodel(order,clean);
                        casa::Array<double> slice = planeIter.getPlane(itsParams->value(thisOrderParam));
                        casa::convertArray<double, float>(slice, cleanArray);
                    }
               } // end of polarisation (i.e. plane) loop 
	
               // Make sure that the next set of minor cycles does not redo unnecessary things.
               // Also "fix" parameters for order >= itsNTaylor. so that the gridding doesn't get done
               // for these extra terms.

               if (firstcycle) {
                   // Fix the params corresponding to extra Taylor terms.
	               // (MV) probably this part needs another careful look
	               for (int order=0; order<tmIt->second; ++order) {
                        // make the helper to correspond to the given order
                        iph.makeTaylorTerm(order);
	                    const std::string thisOrderParam = iph.paramName();
	                    if (order >= itsNTaylor && itsParams->isFree(thisOrderParam)) {
	                        itsParams->fix(thisOrderParam);
	                    }
	               }
                   firstcycle = False;
               }
           } // try
           catch( const AipsError &x ) {
                  throw AskapError("Failed in the MSMFS Minor Cycle : " + x.getMesg() );
           }
      } // loop: tmIt
      
      ASKAPCHECK(nParameters>0, "No free parameters in ImageMSMFSolver");
      
      quality.setDOF(nParameters);
      quality.setRank(0);
      quality.setCond(0.0);
      quality.setInfo("Multi-Scale Multi-Frequency Clean");
      
      /// Save PSFs and Weights into parameter class (to be exported later)
      saveWeights();
      savePSF();

      return true;
    };
    
  }
}
