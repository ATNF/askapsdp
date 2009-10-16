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
       
    /// @brief obtain single polarisation slice for a given image
    /// @details This is a helper method extracting a single polarisation
    /// slice from a given image parameter
    /// @param[in] paramName image parameter
    /// @param[in] pol required polarisation plane
    /// @return slice array
    casa::Array<double> ImageMSMFSolver::polSlice(const std::string &paramName, int pol) const
    {
       ASKAPDEBUGASSERT(itsParams);
       casa::Array<double> img = itsParams->value(paramName);
       const casa::IPosition shape = img.shape();
       const int nPol = shape.nelements()>=3 ? shape(2) : 1;
       ASKAPDEBUGASSERT(pol<nPol);
       if (nPol == 1) {
           return img;
       }
       casa::IPosition blc(shape.nelements(),0);
       blc(2) = pol;
       casa::IPosition trc(shape);
       for (size_t pos = 0; pos<shape.nelements(); ++pos) {
            trc(pos) -= 1;
            ASKAPDEBUGASSERT(trc(pos)>=0);
       }
       trc(2)=pol;
       return img(blc,trc);
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
      // this will not work with faceting, need to think how to fix it
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
               const casa::IPosition imageShape = itsParams->value(ImageParamsHelper(tmIt->first,0).paramName()).shape();               
               const int nPol = imageShape.nelements()>=3 ? imageShape(2) : 1;
               ASKAPLOG_INFO_STR(logger, "There are " << nPol << " polarisation planes to solve for." );
               nParameters += imageShape.product(); // add up the number of pixels for zero order
	           // check consistency
	           for (int order=1;order<tmIt->second;++order) {
                    const casa::IPosition thisShape = itsParams->value(ImageParamsHelper(tmIt->first,order).paramName()).shape();               
                    const int thisNPol = thisShape.nelements()>=3 ? thisShape(2) : 1;
	                ASKAPCHECK(thisNPol == nPol, "Number of polarisations are supposed to be consistent for all Taylor terms, order="<<
	                           order<<" has "<<thisNPol<<" polarisation planes");
                    nParameters += thisShape.product(); // add up the number of pixels for this order
	           }
	 
	           static bool firstcycle=True;
	
	           // Iterate through Polarisations (former sindex)
	           for (int pol=0;pol<nPol;++pol) {
	                ASKAPLOG_INFO_STR(logger, "In Image MSMFSSolver::solveN..E.. : About to iterate for polarisation " 
	                                  << pol<<" in image "<<tmIt->first);
	                const std::string zeroOrderParam = ImageParamsHelper(tmIt->first,0).paramName();
	                
                    // Setup the normalization vector	  
                    ASKAPLOG_INFO_STR(logger, "Reading the normalization vector from : " << zeroOrderParam);
                    ASKAPCHECK(normalEquations().normalMatrixDiagonal().count(zeroOrderParam)>0, "Diagonal not present");
                    const casa::Vector<double>& normdiag(normalEquations().normalMatrixDiagonal().find(zeroOrderParam)->second);
                    const casa::IPosition vecShape(1, polSlice(zeroOrderParam,pol).nelements());
                    const casa::IPosition valShape(polSlice(zeroOrderParam, pol).shape());

                    ASKAPDEBUGASSERT(valShape.nelements()>=2);
	  
                    double maxDiag(casa::max(normdiag));
                    ASKAPLOG_INFO_STR(logger, "Maximum of weights = " << maxDiag );
          
                    if(firstcycle)  {// Initialize everything only once.
	                   // Initialize the latticecleaners
                       ASKAPLOG_INFO_STR(logger, "Initialising the solver for polarisation " << pol);
	    
                       itsCleaners[pol].reset(new casa::MultiTermLatticeCleaner<float>());
                       ASKAPDEBUGASSERT(itsCleaners[pol]);
	    
                       itsCleaners[pol]->setcontrol(casa::CleanEnums::MULTISCALE, niter(), gain(), threshold(), 
	                                      fractionalThreshold(), false);
                       itsCleaners[pol]->ignoreCenterBox(true);
                       itsCleaners[pol]->setscales(itsScales);
                       itsCleaners[pol]->setntaylorterms(itsNTaylor);
                       itsCleaners[pol]->initialise(valShape[0],valShape[1]); // allocates memory once....
	                }
          
                    // Setup the PSFs - all ( 2 x ntaylor - 1 ) of them for the first time.
                    int nOrders = itsNTaylor;
                    casa::Array<float> psfZeroArray(valShape);
                    if (firstcycle) {
                        nOrders = 2*itsNTaylor-1;
                    }
                    // temporary support only homogeneous number of Taylor terms
                    ASKAPASSERT(nOrders == tmIt->second);
                    // buffer for the peak of zero-order PSF
                    float zeroPSFPeak = -1;
                    for( int order=0; order < nOrders; ++order) {
	                    std::string thisOrderParam = ImageParamsHelper(tmIt->first, order).paramName();
                        ASKAPLOG_INFO_STR(logger, "MSMFS solver: processing order "<<order<<" ("<<itsNTaylor<<
                                          " Taylor terms + "<<itsNTaylor-1<<" cross-terms), parameter name: "<<thisOrderParam);
                        ASKAPCHECK(normalEquations().normalMatrixSlice().count(thisOrderParam)>0, "PSF Slice for pol="<<pol<<
                                   " and order="<<order<<" is not present");
                        const casa::Vector<double>& slice(normalEquations().normalMatrixSlice().find(thisOrderParam)->second);
                        ASKAPCHECK(normalEquations().dataVector(thisOrderParam).size()>0, "Data vector not present for pol="<<
                                   pol<<" and order="<<order);
                        const casa::Vector<double>& dv = normalEquations().dataVector(thisOrderParam);
	   
                        casa::Array<float> psfArray(valShape);
                        casa::convertArray<float, double>(psfArray, slice.reform(valShape));
                        casa::Array<float> dirtyArray(valShape);
                        casa::convertArray<float, double>(dirtyArray, dv.reform(valShape));
                        casa::Array<float> cleanArray(valShape);
                        casa::convertArray<float, double>(cleanArray, polSlice(thisOrderParam,pol));
                        if (order == 0) {
                            zeroPSFPeak = doNormalization(normdiag,tol(),psfArray,dirtyArray);
                        } else {
                            ASKAPDEBUGASSERT(zeroPSFPeak > 0.);
                            doNormalization(normdiag,tol(),psfArray,zeroPSFPeak,dirtyArray);
                        }
	   	   
                        ASKAPLOG_INFO_STR(logger, "Preconditioning PSF for pol=" << pol << " and order=" << order );

                        if (order == 0) {
                            psfZeroArray = psfArray.copy();
                        }
	    	   
                        if( doPreconditioning(psfZeroArray,psfArray) ) {
                           // Write PSFs to disk.
                           ASKAPLOG_INFO_STR(logger, "Exporting preconditioned psfs (to be stored to disk later)");
                           Axes axes(itsParams->axes(thisOrderParam));
                           string psfName="psf."+thisOrderParam;
                           casa::Array<double> aargh(valShape);
                           casa::convertArray<double,float>(aargh,psfArray);
                           const casa::Array<double> & APSF(aargh);
                           if (!itsParams->has(psfName)) {
                               itsParams->add(psfName, APSF, axes);
                           } else {
                               itsParams->update(psfName, APSF);
                           }
	                    }
	   
                        casa::ArrayLattice<float> psf(psfArray);
                        itsCleaners[pol]->setpsf(order,psf);
	   
                        // Setup the Residual Images and Model Images  - ( ntaylor ) of them
                        if (order < itsNTaylor) {
                            // Now precondition the residual images
                            doPreconditioning(psfZeroArray,dirtyArray);
		   
                            // We need lattice equivalents. We can use ArrayLattice which involves
                            // no copying
                            casa::ArrayLattice<float> dirty(dirtyArray);
                            casa::ArrayLattice<float> clean(cleanArray);

                            // Send in Dirty images only for ntaylor terms
                            itsCleaners[pol]->setresidual(order,dirty);
                            itsCleaners[pol]->setmodel(order,clean);
                        }
	   
                    } // end of 'order' loop
	  
                    ASKAPLOG_INFO_STR(logger, "Starting Minor Cycles" );
                    itsCleaners[pol]->mtclean();
                    ASKAPLOG_INFO_STR(logger, "Finished Minor Cycles." );
	  
                    // Write the final vector of clean model images into parameters
                    for( int order=0; order < itsNTaylor; ++order) {
	                    const std::string thisOrderParam = ImageParamsHelper(tmIt->first, order).paramName();
                        const casa::IPosition valShape(polSlice(thisOrderParam,pol).shape());
                        casa::Array<float> cleanArray(valShape);
                        casa::ArrayLattice<float> clean(cleanArray);
                        ASKAPLOG_INFO_STR(logger, "About to get model" );
                        itsCleaners[pol]->getmodel(order,clean);
                        casa::Array<double> slice = polSlice(thisOrderParam,pol);
                        casa::convertArray<double, float>(slice, cleanArray);
                    }
               } // end of polarisation loop 
	
               // Make sure that the next set of minor cycles does not redo unnecessary things.
               // Also "fix" parameters for order >= itsNTaylor. so that the gridding doesn't get done
               // for these extra terms.

               if (firstcycle) {
                   // Fix the params corresponding to extra Taylor terms.
	               // (MV) probably this part needs another careful look
	               for (int order=0; order<tmIt->second; ++order) {
	                    const std::string thisOrderParam = ImageParamsHelper(tmIt->first, order).paramName();
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
