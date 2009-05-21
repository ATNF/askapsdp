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

#include <measurementequation/ImageMultiScaleSolver.h>

#include <askap_synthesis.h>
#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, ".measurementequation");

#include <askap/AskapError.h>

// need it just for null deleter
#include <askap/AskapUtil.h>

#include <utils/MultiDimArrayPlaneIter.h>

#include <casa/aips.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/Arrays/Vector.h>

#include <lattices/Lattices/LatticeCleaner.h>
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
    
      
    ImageMultiScaleSolver::ImageMultiScaleSolver(const askap::scimath::Params& ip) : 
          ImageCleaningSolver(ip) 
    {
      itsScales.resize(3);
      itsScales(0)=0;
      itsScales(1)=10;
      itsScales(2)=30;
    }

    ImageMultiScaleSolver::ImageMultiScaleSolver(const askap::scimath::Params& ip,
      const casa::Vector<float>& scales) : 
          ImageCleaningSolver(ip)
    {
      itsScales.resize(scales.size());
      itsScales=scales;
    }
    
    void ImageMultiScaleSolver::init()
    {
      resetNormalEquations();
    }
    
// Solve for update simply by scaling the data vector by the diagonal term of the
// normal equations i.e. the residual image
    bool ImageMultiScaleSolver::solveNormalEquations(askap::scimath::Quality& quality)
    {

// Solving A^T Q^-1 V = (A^T Q^-1 A) P
      uint nParameters=0;

// Find all the free parameters beginning with image
      vector<string> names(itsParams->completions("image"));
      map<string, uint> indices;
      
      for (vector<string>::const_iterator  it=names.begin();it!=names.end();it++)
      {
        string name="image"+*it;
        if(itsParams->isFree(name)) {
          indices[name]=nParameters;
          nParameters+=itsParams->value(name).nelements();
        }
      }
      ASKAPCHECK(nParameters>0, "No free parameters in ImageMultiScaleSolver");
      
      for (map<string, uint>::const_iterator indit=indices.begin();indit!=indices.end();++indit)
      {
// Axes are dof, dof for each parameter
        //const casa::IPosition vecShape(1, itsParams->value(indit->first).nelements());
        scimath::MultiDimArrayPlaneIter planeIter(itsParams->value(indit->first).shape());
        
        ASKAPCHECK(normalEquations().normalMatrixDiagonal().count(indit->first)>0, "Diagonal not present");
        casa::Vector<double> diag(normalEquations().normalMatrixDiagonal().find(indit->first)->second);
        ASKAPCHECK(normalEquations().dataVector(indit->first).size()>0, "Data vector not present");
        casa::Vector<double> dv = normalEquations().dataVector(indit->first);
        ASKAPCHECK(normalEquations().normalMatrixSlice().count(indit->first)>0, "PSF Slice not present");
        casa::Vector<double> slice(normalEquations().normalMatrixSlice().find(indit->first)->second);
        
        if (planeIter.tag()!="") {
            // it is not a single plane case, there is something to report
            ASKAPLOG_INFO_STR(logger, "Processing plane "<<planeIter.sequenceNumber()<<
                                      " tagged as "<<planeIter.tag());
		}
		
        casa::Array<float> dirtyArray(planeIter.planeShape());
        casa::convertArray<float, double>(dirtyArray, planeIter.getPlane(dv));
        casa::Array<float> psfArray(planeIter.planeShape());
        casa::convertArray<float, double>(psfArray, planeIter.getPlane(slice));
        casa::Array<float> cleanArray(planeIter.planeShape());
        casa::convertArray<float, double>(cleanArray, planeIter.getPlane(itsParams->value(indit->first)));
        casa::Array<float> maskArray(planeIter.planeShape());

	    // Normalize
	    doNormalization(planeIter.getPlaneVector(diag),tol(),psfArray,dirtyArray, 
	        boost::shared_ptr<casa::Array<float> >(&maskArray, utility::NullDeleter()));
    
	    // Precondition the PSF and DIRTY images before solving.
        if(doPreconditioning(psfArray,dirtyArray)) {
	       // Save the new PSFs to disk
	       Axes axes(itsParams->axes(indit->first));
	       string psfName="psf."+(indit->first);
	       casa::Array<double> anothertemp(planeIter.planeShape());
	       casa::convertArray<double,float>(anothertemp,psfArray);
	       const casa::Array<double> & APSF(anothertemp);
	       if (!itsParams->has(psfName)) {
	           // create an empty parameter with the full shape
	           itsParams->add(psfName, planeIter.shape(), axes);
	       } 
	       itsParams->update(psfName, APSF, planeIter.position());	       
	    } // if there was preconditioning
	    ASKAPLOG_INFO_STR(logger, "Peak data vector flux (derivative) "<<max(dirtyArray));
		
        // We need lattice equivalents. We can use ArrayLattice which involves
        // no copying
        casa::ArrayLattice<float> dirty(dirtyArray);
        casa::ArrayLattice<float> psf(psfArray);
        casa::ArrayLattice<float> clean(cleanArray);
        casa::ArrayLattice<float> mask(maskArray);
        
        // uncomment the code below to save the residual image
        // This takes up some memory and we have to ship the residual image out inside
        // the parameter class. Therefore, we may not need this functionality in the 
        // production version (or may need to implement it in a different way).
        {
           Axes axes(itsParams->axes(indit->first));
           ASKAPDEBUGASSERT(indit->first.find("image")==0);
           ASKAPCHECK(indit->first.size()>5, 
                   "Image parameter name should have something appended to word image")           
	       const string residName="residual"+indit->first.substr(5);
	       casa::Array<double> anothertemp(planeIter.planeShape());
	       casa::convertArray<double,float>(anothertemp,dirtyArray);
	       const casa::Array<double> & AResidual(anothertemp);
	       if (!itsParams->has(residName)) {
	           // create an empty parameter with the full shape
	           itsParams->add(residName, planeIter.shape(), axes);
	       }
	       itsParams->update(residName, AResidual, planeIter.position());	               
        }
        
        
        /*
        // uncomment the code below to save the mask
        {
           Axes axes(itsParams->axes(indit->first));
	       string maskName="mask."+(indit->first);
	       casa::Array<double> anothertemp(planeIter.planeShape());
	       casa::convertArray<double,float>(anothertemp,maskArray);
	       const casa::Array<double> & AMask(anothertemp);
	       if (!itsParams->has(maskName)) {
	           // create an empty parameter with the full shape
	           itsParams->add(maskName, planeIter.shape(), axes);
	       }
	       itsParams->update(maskName, AMask, planeIter.position());	               
        }
        */
        // Create a lattice cleaner to do the dirty work :)
        /// @todo More checks on reuse of LatticeCleaner
        boost::shared_ptr<casa::LatticeCleaner<float> > lc;
        // every plane should have its own LatticeCleaner, therefore we should ammend the 
        // key somehow to make it individual for each plane. Adding tag seems to be a good idea
        const std::string cleanerKey = indit->first + planeIter.tag();
        std::map<string, boost::shared_ptr<casa::LatticeCleaner<float> > >::const_iterator it =
                         itsCleaners.find(cleanerKey);
        
        
        if(it!=itsCleaners.end()) {
          lc=it->second; 
          ASKAPDEBUGASSERT(lc);
          lc->update(dirty);
        } else {
          lc.reset(new casa::LatticeCleaner<float>(psf, dirty));
          itsCleaners[cleanerKey]=lc;          
          lc->setMask(mask,maskingThreshold());
	  
	      ASKAPDEBUGASSERT(lc);
	      if(algorithm()=="Hogbom") {
	         casa::Vector<float> scales(1);
	         scales(0)=0.0;
	         lc->setscales(scales);
	         lc->setcontrol(casa::CleanEnums::HOGBOM, niter(), gain(), threshold(),
	                   fractionalThreshold(), false);
	      } else {
	           lc->setscales(itsScales);
	           lc->setcontrol(casa::CleanEnums::MULTISCALE, niter(), gain(), threshold(), 
	                   fractionalThreshold(),false);
	      } // if algorithm == Hogbom, else case (other algorithm)
	      lc->ignoreCenterBox(true);
	    } // if cleaner found in the cache, else case - new cleaner needed
	    lc->clean(clean);
	    ASKAPLOG_INFO_STR(logger, "Peak flux of the clean image "<<max(cleanArray));

	    ASKAPDEBUGASSERT(itsParams);
	
	    const std::string peakResParam = std::string("peak_residual.") + cleanerKey;
	    if (itsParams->has(peakResParam)) {
	        itsParams->update(peakResParam, lc->strengthOptimum());
        } else {
	        itsParams->add(peakResParam, lc->strengthOptimum());
        }
        itsParams->fix(peakResParam);	    
	
	    casa::Array<double> outputPlane = planeIter.getPlane(itsParams->value(indit->first));
        casa::convertArray<double, float>(outputPlane,cleanArray);
      } // loop over map of indices
      
      quality.setDOF(nParameters);
      quality.setRank(0);
      quality.setCond(0.0);
      quality.setInfo("Multiscale Clean");
      
      /// Save the PSF and Weight
      saveWeights();      
      savePSF();
      
      return true;
    };
    
    Solver::ShPtr ImageMultiScaleSolver::clone() const
    {
      return Solver::ShPtr(new ImageMultiScaleSolver(*this));
    }
    
  }
}


