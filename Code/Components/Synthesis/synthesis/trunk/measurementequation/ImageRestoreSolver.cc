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
    ImageRestoreSolver::ImageRestoreSolver(const askap::scimath::Params& ip,
		    const casa::Vector<casa::Quantum<double> >& beam) :
	    ImageSolver(ip), itsBeam(beam)
    {
    }
    
    void ImageRestoreSolver::init()
    {
	    resetNormalEquations();
    }
    
    // Solve for update simply by scaling the data vector by the diagonal term of the
    // normal equations i.e. the residual image
    bool ImageRestoreSolver::solveNormalEquations(
		    askap::scimath::Quality& quality)
    {
	// Solving A^T Q^-1 V = (A^T Q^-1 A) P
	
	// Find all the free parameters beginning with image
	vector<string> names(itsParams->completions("image"));
	uint nParameters=0;
	for (vector<string>::iterator it=names.begin(); it!=names.end(); ++it)
	{
		const string name="image"+*it;
		// completions should return only free parameters according to its code in Code/Base
		ASKAPDEBUGASSERT(itsParams->isFree(name));
		*it = name; // append the common part to the front of the parameter name
		nParameters+=itsParams->value(name).nelements();
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
	         SynthesisParamsHelper::add(*itsParams,ci->first,ci->second);
	         itsParams->fix(ci->first);
	     }
	}
	//
		
	// iterate over all free parameters (i.e. parts of the image for faceted case)
	for (vector<string>::const_iterator ci=names.begin(); ci !=names.end(); ++ci) {
      ImageParamsHelper iph(*ci);

	  if (facetmap[iph.name()] == 1) {
	      // this is not a faceting case, restore the image in situ and add residuals 
	      ASKAPLOG_INFO_STR(logger, "Restoring " << *ci );

	      // Create a temporary image
	      boost::shared_ptr<casa::TempImage<float> > image(SynthesisParamsHelper::tempImage(*itsParams, *ci));
	      casa::Image2DConvolver<float> convolver;	
	      const casa::IPosition pixelAxes(2, 0, 1);	
	      casa::LogIO logio;
	      convolver.convolve(logio, *image, *image, casa::VectorKernel::GAUSSIAN,
			     pixelAxes, itsBeam, true, 1.0, false);
	      SynthesisParamsHelper::update(*itsParams, *ci, *image);
	      // for some reason update makes the parameter free as well
	      itsParams->fix(*ci);
	  
	      addResiduals(*ci,itsParams->value(*ci).shape(),itsParams->value(*ci));
	      SynthesisParamsHelper::setBeam(*itsParams, *ci, itsBeam);
      } else {
          // this is a single facet of a larger image, just fill in the bigger image with the model
          ASKAPLOG_INFO_STR(logger, "Inserting facet " << *ci<<" into merged image "<<iph.name());
          casa::Array<double> patch = SynthesisParamsHelper::getFacet(*itsParams,*ci);
          const casa::Array<double> model = scimath::PaddingUtils::centeredSubArray(itsParams->value(*ci),
                                          patch.shape());
          patch = model;
      }
	}
	
	// restore faceted images
    for (map<string,int>::const_iterator ci=facetmap.begin();ci!=facetmap.end();++ci) {
	     if (ci->second != 1) {
	         // this is a multi-facet image
	         ASKAPLOG_INFO_STR(logger, "Restoring faceted image " << ci->first );
            
             boost::shared_ptr<casa::TempImage<float> > image(SynthesisParamsHelper::tempImage(*itsParams, ci->first));
	         casa::Image2DConvolver<float> convolver;	
	         const casa::IPosition pixelAxes(2, 0, 1);	
	         casa::LogIO logio;
	         convolver.convolve(logio, *image, *image, casa::VectorKernel::GAUSSIAN,
			       pixelAxes, itsBeam, true, 1.0, false);
	         SynthesisParamsHelper::update(*itsParams, ci->first, *image);
	         // for some reason update makes the parameter free as well
	         itsParams->fix(ci->first);
	        
	         // add residuals
	         for (int xFacet = 0; xFacet<ci->second; ++xFacet) {
	              for (int yFacet = 0; yFacet<ci->second; ++yFacet) {
	                   ASKAPLOG_INFO_STR(logger, "Adding residuals for facet ("<<xFacet<<","
	                        <<yFacet<<")");
	                   ImageParamsHelper iph(ci->first,xFacet,yFacet);
	                   addResiduals(iph.paramName(),itsParams->value(iph.paramName()).shape(),
	                                SynthesisParamsHelper::getFacet(*itsParams,iph.paramName()));
	                   
	              }
	         }
	         
	         SynthesisParamsHelper::setBeam(*itsParams, ci->first, itsBeam);
	         
	     }
	}

    // remove parts of each faceted image
	for (vector<string>::const_iterator ci=names.begin(); ci !=names.end(); ++ci) {
	     ImageParamsHelper iph(*ci);
         if (iph.isFacet()) {
             ASKAPLOG_INFO_STR(logger, "Remove facet patch "<<*ci<<" from the parameters");
             itsParams->remove(*ci);
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
                         casa::Array<double> out)
    {
	   // Axes are dof, dof for each parameter
	   casa::IPosition vecShape(1, out.shape().product());
	   
	   ASKAPCHECK(normalEquations().normalMatrixDiagonal().count(name)>0, "Diagonal not present");
	   const casa::Vector<double>& diag(normalEquations().normalMatrixDiagonal().find(name)->second);
	   ASKAPCHECK(normalEquations().dataVector(name).size()>0, "Data vector not present");
	   const casa::Vector<double> &dv = normalEquations().dataVector(name);
	   ASKAPCHECK(normalEquations().normalMatrixSlice().count(name)>0, "PSF Slice not present");
       const casa::Vector<double>& slice(normalEquations().normalMatrixSlice().find(name)->second);

	   casa::Array<float> dirtyArray(shape);
       casa::convertArray<float, double>(dirtyArray, dv.reform(shape));
       casa::Array<float> psfArray(shape);
       casa::convertArray<float, double>(psfArray, slice.reform(shape));

	   // Normalize by the diagonal
	   doNormalization(diag,tol(),psfArray,dirtyArray);
	  
	   // Do the preconditioning
	   doPreconditioning(psfArray,dirtyArray);
	  
	   // Add the residual image        
	   // the code below involves an extra copying. We can replace it later with a copyless version
	   // doing element by element adding explicitly.
	   casa::Array<double> convertedResidual(out.shape());
	   convertArray(convertedResidual, scimath::PaddingUtils::centeredSubArray(dirtyArray,out.shape()));
	   out += convertedResidual;
    }
	
    Solver::ShPtr ImageRestoreSolver::clone() const
    {
	    return Solver::ShPtr(new ImageRestoreSolver(*this));
    }

  }
}



