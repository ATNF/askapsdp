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

#include <measurementequation/ImageRestoreSolver.h>
#include <measurementequation/SynthesisParamsHelper.h>
#include <measurementequation/ImageParamsHelper.h>
#include <measurementequation/ImageSolverFactory.h>
#include <measurementequation/IImagePreconditioner.h>
#include <measurementequation/WienerPreconditioner.h>
#include <measurementequation/GaussianTaperPreconditioner.h>

#include <utils/PaddingUtils.h>

#include <askap_synthesis.h>
#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, ".measurementequation");

#include <askap/AskapError.h>

#include <casa/aips.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/Logging/LogIO.h>
#include <scimath/Mathematics/VectorKernel.h>
#include <images/Images/TempImage.h>
#include <images/Images/Image2DConvolver.h>

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
    ImageRestoreSolver::ImageRestoreSolver(const casa::Vector<casa::Quantum<double> >& beam) :
	    itsBeam(beam), itsEqualiseNoise(false)
    {
    }
    
    void ImageRestoreSolver::init()
    {
	    resetNormalEquations();
    }

    /// @brief Solve for parameters, updating the values kept internally
    /// The solution is constructed from the normal equations. The parameters named 
    /// image* are interpreted as images and solved for.
    /// @param[in] ip current model (to be updated)
    /// @param[in] quality Solution quality information
    bool ImageRestoreSolver::solveNormalEquations(askap::scimath::Params& ip, askap::scimath::Quality& quality)
    {
	// Solving A^T Q^-1 V = (A^T Q^-1 A) P
	
	// Find all the free parameters beginning with image
	vector<string> names(ip.completions("image"));
	uint nParameters=0;
	for (vector<string>::iterator it=names.begin(); it!=names.end(); ++it)
	{
		const string name="image"+*it;
		// completions should return only free parameters according to its code in Code/Base
		ASKAPDEBUGASSERT(ip.isFree(name));
		*it = name; // append the common part to the front of the parameter name
		nParameters+=ip.value(name).nelements();
	}

	ASKAPCHECK(nParameters>0, "No free parameters in ImageRestoreSolver");
	
	// determine which images are faceted and setup parameters representing the 
	// result of a merge.
	map<string,int> facetmap;
	SynthesisParamsHelper::listFacets(names, facetmap);
	for (map<string,int>::const_iterator ci=facetmap.begin();ci!=facetmap.end();++ci) {
	     if (ci->second != 1) {
	         // this is a multi-facet image, add a fixed parameter representing the whole image
	         ASKAPLOG_INFO_STR(logger, "Adding a fixed parameter " << ci->first<<
                           " representing faceted image with "<<ci->second<<" facets");                 
	         SynthesisParamsHelper::add(ip,ci->first,ci->second);
	         ip.fix(ci->first);
	     }
	}
	//
		
	// iterate over all free parameters (i.e. parts of the image for faceted case)
	for (vector<string>::const_iterator ci=names.begin(); ci !=names.end(); ++ci) {
      ImageParamsHelper iph(*ci);
      // obtain name with just the taylor suffix, if present
      const std::string name = iph.taylorName();

	  if (facetmap[name] == 1) {
	      // this is not a faceting case, restore the image in situ and add residuals 
	      ASKAPLOG_INFO_STR(logger, "Restoring " << *ci );

	      // Create a temporary image
	      boost::shared_ptr<casa::TempImage<float> > image(SynthesisParamsHelper::tempImage(ip, *ci));	      
	      casa::Image2DConvolver<float> convolver;	
	      const casa::IPosition pixelAxes(2, 0, 1);	
	      casa::LogIO logio;
	      convolver.convolve(logio, *image, *image, casa::VectorKernel::GAUSSIAN,
			     pixelAxes, itsBeam, true, 1.0, false);
          SynthesisParamsHelper::update(ip, *ci, *image);
	      // for some reason update makes the parameter free as well
	      ip.fix(*ci);
	  
	      addResiduals(*ci,ip.value(*ci).shape(),ip.value(*ci));
	      SynthesisParamsHelper::setBeam(ip, *ci, itsBeam);
      } else {
          // this is a single facet of a larger image, just fill in the bigger image with the model
          ASKAPLOG_INFO_STR(logger, "Inserting facet " << iph.paramName()<<" into merged image "<<name);
          casa::Array<double> patch = SynthesisParamsHelper::getFacet(ip,iph.paramName());
          const casa::Array<double> model = scimath::PaddingUtils::centeredSubArray(ip.value(iph.paramName()),
                                          patch.shape());
          patch = model;
      }
	}
	
	// restore faceted images
    for (map<string,int>::const_iterator ci=facetmap.begin();ci!=facetmap.end();++ci) {
	     if (ci->second != 1) {
	         // this is a multi-facet image
	         ASKAPLOG_INFO_STR(logger, "Restoring faceted image " << ci->first );
            
             boost::shared_ptr<casa::TempImage<float> > image(SynthesisParamsHelper::tempImage(ip, ci->first));
	         casa::Image2DConvolver<float> convolver;	
	         const casa::IPosition pixelAxes(2, 0, 1);	
	         casa::LogIO logio;
	         convolver.convolve(logio, *image, *image, casa::VectorKernel::GAUSSIAN,
			       pixelAxes, itsBeam, true, 1.0, false);
	         SynthesisParamsHelper::update(ip, ci->first, *image);
	         // for some reason update makes the parameter free as well
	         ip.fix(ci->first);
	        
	         // add residuals
	         for (int xFacet = 0; xFacet<ci->second; ++xFacet) {
	              for (int yFacet = 0; yFacet<ci->second; ++yFacet) {
	                   ASKAPLOG_INFO_STR(logger, "Adding residuals for facet ("<<xFacet<<","
	                        <<yFacet<<")");
	                   // ci->first may have taylor suffix defined, load it first and then add facet indices
	                   ImageParamsHelper iph(ci->first);	                   
	                   iph.makeFacet(xFacet,yFacet);
	                   addResiduals(iph.paramName(),ip.value(iph.paramName()).shape(),
	                                SynthesisParamsHelper::getFacet(ip,iph.paramName()));
	                   
	              }
	         }
	         
	         SynthesisParamsHelper::setBeam(ip, ci->first, itsBeam);
	         
	     }
	}

    // remove parts of each faceted image
	for (vector<string>::const_iterator ci=names.begin(); ci !=names.end(); ++ci) {
	     ImageParamsHelper iph(*ci);
         if (iph.isFacet()) {
             ASKAPLOG_INFO_STR(logger, "Remove facet patch "<<*ci<<" from the parameters");
             ip.remove(*ci);
         }
	}
	
	quality.setDOF(nParameters);
	quality.setRank(0);
	quality.setCond(0.0);
	quality.setInfo("Restored image calculated");
	
	return true;
    };
    
    /// @brief solves for and adds residuals
    /// @details Restore solver convolves the current model with the beam and adds the
    /// residual image. The latter has to be "solved for" with a proper preconditioning and
    /// normalisation using the normal equations stored in the base class. All operations
    /// required to extract residuals from normal equations and fill an array with them
    /// are encapsulated in this method. Faceting needs a subimage only, hence the array
    /// to fill may not have exactly the same shape as the dirty (residual) image corresponding
    /// to the given parameter. This method assumes that the centres of both images are the same
    /// and extracts only data required.
    /// @param[in] name name of the parameter to work with
    /// @param[in] shape shape of the parameter (we wouldn't need it if the shape of the
    ///                   output was always the same as the shape of the paramter. It is not
    ///                   the case for faceting).
    /// @param[in] out output array
    void ImageRestoreSolver::addResiduals(const std::string &name, const casa::IPosition &shape,
                         casa::Array<double> out) const
    {
	   // Axes are dof, dof for each parameter
	   casa::IPosition vecShape(1, out.shape().product());
	   
	   ASKAPCHECK(normalEquations().normalMatrixDiagonal().count(name)>0, "Diagonal not present");
	   const casa::Vector<double>& diag(normalEquations().normalMatrixDiagonal().find(name)->second);
	   ASKAPCHECK(normalEquations().dataVector(name).size()>0, "Data vector not present");
	   const casa::Vector<double> &dv = normalEquations().dataVector(name);
	   ASKAPCHECK(normalEquations().normalMatrixSlice().count(name)>0, "PSF Slice not present");
       const casa::Vector<double>& slice(normalEquations().normalMatrixSlice().find(name)->second);

	   ASKAPLOG_INFO_STR(logger, "Maximum of data vector corresponding to "<<name<<" is "<<casa::max(dv));

	   casa::Array<float> dirtyArray(shape);
       casa::convertArray<float, double>(dirtyArray, dv.reform(shape));
       casa::Array<float> psfArray(shape);
       casa::convertArray<float, double>(psfArray, slice.reform(shape));

       // uninitialised mask shared pointer means that we don't need it (i.e. no weight equalising)
       boost::shared_ptr<casa::Array<float> > mask;
       if (itsEqualiseNoise) {
           ASKAPLOG_INFO_STR(logger, "Residual will be multiplied by sqrt(normalised weight) during restoration");
           // mask will have a noramised sqrt(weight) pattern after doNormalization
           mask.reset(new casa::Array<float>(dirtyArray.shape()));
       } else {
           ASKAPLOG_INFO_STR(logger, "Restored image will have primary beam corrected noise (no equalisation)");
       }
       
	   // Normalize by the diagonal
	   doNormalization(diag,tol(),psfArray,dirtyArray,mask);
	  
	   // Do the preconditioning
	   doPreconditioning(psfArray,dirtyArray);
	   
	   // we have to do noise equalisation for final residuals after preconditioning
	   if (itsEqualiseNoise) {
	       const casa::IPosition vecShape(1,dirtyArray.nelements());
           casa::Vector<float> dirtyVector(dirtyArray.reform(vecShape));
	       const casa::Vector<float> maskVector(mask->reform(vecShape));
	       for (int i = 0; i<vecShape[0]; ++i) {
	            dirtyVector[i] *= maskVector[i];
	       }
	   }
	  
	   // Add the residual image        
	   // the code below involves an extra copying. We can replace it later with a copyless version
	   // doing element by element adding explicitly.
	   casa::Array<double> convertedResidual(out.shape());
	   convertArray(convertedResidual, scimath::PaddingUtils::centeredSubArray(dirtyArray,out.shape()));
	   out += convertedResidual;
    }
    
    /// @brief obtain an estimate of the restoring beam
    /// @details This method fits a 2D Gaussian into the central area of the PSF
    /// (a support is searched assuming 50% cutoff) if the appropriate option
    /// is set. Otherwise, it just returns the beam parameters passed in the constructor
    /// (i.e. user override).
    /// @param[in] name name of the parameter to work with
    /// @param[in] shape shape of the parameter
    casa::Vector<casa::Quantum<double> > ImageRestoreSolver::getBeam(const std::string &name, 
                                             const casa::IPosition &shape) const
    {
        return itsBeam;
    }
	
    Solver::ShPtr ImageRestoreSolver::clone() const
    {
	    return Solver::ShPtr(new ImageRestoreSolver(*this));
    }
    
    /// @brief static method to create solver
    /// @details Each solver should have a static factory method, which is
    /// able to create a particular type of the solver and initialise it with
    /// the parameters taken from the given parset. It is assumed that the method
    /// receives a subset of parameters where the solver name, if it was present in
    /// the parset, is already taken out
    /// @param[in] parset input parset file
    /// @param[in] ip model parameters
    /// @return a shared pointer to the solver instance
    boost::shared_ptr<ImageRestoreSolver> ImageRestoreSolver::createSolver(const LOFAR::ParameterSet &parset,
                  const askap::scimath::Params &ip)
    {
       casa::Vector<casa::Quantum<double> > qBeam(3);
       const vector<string> beam = parset.getStringVector("beam");
       if (beam.size() == 1) {
           ASKAPCHECK(beam[0] == "fit", 
               "beam parameter should be either equal to 'fit' or contain 3 elements defining the beam size. You have "<<beam[0]);
           // we use the feature here that the restoring solver is created when the imaging is completed, so
           // there is a PSF image in the parameters. Fitting of the beam has to be moved to restore solver to
           // be more flexible
           qBeam = SynthesisParamsHelper::fitBeam(ip);           
       } else {
          ASKAPCHECK(beam.size() == 3, "Need three elements for beam or a single word 'fit'. You have "<<beam);
          for (int i=0; i<3; ++i) {
               casa::Quantity::read(qBeam(i), beam[i]);
          }
       }
       ASKAPDEBUGASSERT(qBeam.size() == 3);
       ASKAPLOG_INFO_STR(logger, "Restore solver will convolve with the 2D gaussian: "<<qBeam[0].getValue("arcsec")<<
                 " x "<<qBeam[1].getValue("arcsec")<<" arcsec at position angle "<<qBeam[2].getValue("deg")<<" deg");
       //
       boost::shared_ptr<ImageRestoreSolver> result(new ImageRestoreSolver(qBeam));
       const bool equalise = parset.getBool("equalise",false);
       result->equaliseNoise(equalise);
       return result;
    }

    /// @brief configure basic parameters of the restore solver
    /// @details This method configures basic parameters of this restore solver the same way as
    /// they are configured for normal imaging solver. We want to share the same parameters between
    /// these two types of solvers (e.g. weight cutoff tolerance, preconditioning, etc), but the 
    /// appropriate parameters are given in a number of places of the parset, sometimes with 
    /// solver-specific prefies, so parsing a parset in createSolver is not a good idea. This method
    /// does the job and encapsulates all related code.
    /// @param[in] ts template solver (to take parameters from)
    void ImageRestoreSolver::configureSolver(const ImageSolver &ts) 
    {
      setThreshold(ts.threshold());
      setVerbose(ts.verbose());
      setTol(ts.tol());    
      
      // behavior in the weight cutoff area
      zeroWeightCutoffMask(ts.zeroWeightCutoffMask());
      zeroWeightCutoffArea(ts.zeroWeightCutoffArea());
    }
    

  } // namespace synthesis
} // namespace askap



