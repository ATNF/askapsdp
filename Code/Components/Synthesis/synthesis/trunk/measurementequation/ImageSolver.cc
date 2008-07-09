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

#include <measurementequation/ImageSolver.h>

#include <askap_synthesis.h>
#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, ".measurementequation");

#include <askap/AskapError.h>

#include <casa/aips.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>

using namespace askap;
using namespace askap::scimath;

#include <iostream>

#include <cmath>
using std::abs;

#include <map>
#include <vector>
#include <string>
#include <stdexcept>

using std::map;
using std::vector;
using std::string;

namespace askap
{
  namespace synthesis
  {

    ImageSolver::ImageSolver(const askap::scimath::Params& ip) :
      askap::scimath::Solver(ip)
    {
    }

    void ImageSolver::init()
    {
      resetNormalEquations();
    }
    
    // Add a new element to a list of preconditioners
    void ImageSolver::addPreconditioner(IImagePreconditioner::ShPtr pc)
    {
	    // Add a new element to the map of preconditioners
	    int n = itsPreconditioners.size();
	    itsPreconditioners[n+1] = pc;
    }

    /// @brief perform normalization of the dirty image and psf
	/// @details This method divides the PSF and dirty image by the diagonal of the Hessian.
	/// If a non-void shared pointer is specified for the mask parameter, this method assigns
	/// 0. for those elements where truncation of the weights has been performed and 1. 
	/// otherwise. 
	/// @param[in] diag diagonal of the Hessian (i.e. weights), dirty image will be
	///            divided by an appropriate element of the diagonal or by a cutoff
	///            value
	/// @param[in] tolerance cutoff value given as a fraction of the largest diagonal element
	/// @param[in] psf  point spread function, which is normalized to unity
	/// @param[in] dirty dirty image which is normalized by truncated weights (diagonal)
	/// @param[out] mask shared pointer to the output mask showing where the truncation has 
	///             been performed.
	/// @note although mask is filled in inside this method it should already have a correct 
	/// size before this method is called. Pass a void shared pointer (default) to skip 
	/// mask-related functionality. Hint: use utility::NullDeleter to wrap a shared pointer
	/// over an existing array reference.
    void ImageSolver::doNormalization(const casa::Vector<double>& diag, const float& tolerance, casa::Array<float>& psf, 
				      casa::Array<float>& dirty, 
				      const boost::shared_ptr<casa::Array<float> > &mask)
    {
        const double maxDiag(casa::max(diag));
        const double cutoff=tolerance*maxDiag;

	    /// The PSF is just an approximation calculated from a subset of the data. So we
	    /// we allowed to normalize the peak to unity.
	
	    ASKAPLOG_INFO_STR(logger, "Normalizing PSF by maximum of diagonal equal to "<<maxDiag<<
	                ", cutoff weight is "<<tolerance*100<<"\% of the largest diagonal element");
	    ASKAPLOG_INFO_STR(logger, "Peak of PSF before normalization = " << casa::max(psf));                      
        psf /= float(maxDiag);
        ASKAPLOG_INFO_STR(logger, "Peak of PSF = " << casa::max(psf));
	
	    uint nAbove = 0;
        casa::IPosition vecShape(1,diag.nelements());
        casa::Vector<float> dirtyVector(dirty.reform(vecShape));
        casa::Vector<float> maskVector(mask ? mask->reform(vecShape) : casa::Vector<float>());
        for (casa::uInt elem=0; elem<diag.nelements(); ++elem) {
             if(diag(elem)>cutoff) {
                dirtyVector(elem)/=diag(elem);
                if (mask) {
                    maskVector(elem)=1.0;
                }
                nAbove++;
             } else {
                dirtyVector(elem)/=maxDiag;
                if (mask) {
                    maskVector(elem)=0.0;
                } // if mask required
             }  // if element > cutoff
        } // loop over elements (image pixels)
        ASKAPLOG_INFO_STR(logger, "Normalized dirty image by truncated weights image");
        if (mask) {
            ASKAPLOG_INFO_STR(logger, "Converted truncated weights image to clean mask");
        } // if mask required
        ASKAPLOG_INFO_STR(logger, 100.0*float(nAbove)/float(diag.nelements()) << "% of the pixels were above the cutoff " << cutoff);
    }

    // Apply all the preconditioners in the order in which they were created.
    bool ImageSolver::doPreconditioning(casa::Array<float>& psf, casa::Array<float>& dirty)
    {
	    bool status=false;
	    for(std::map<int, IImagePreconditioner::ShPtr>::const_iterator pciter=itsPreconditioners.begin(); pciter!=itsPreconditioners.end(); pciter++)
	    {
	      status = status | (pciter->second)->doPreconditioning(psf,dirty);
	    }
	    return status;
    }

    // Solve for update simply by scaling the data vector by the diagonal term of the
    // normal equations i.e. the residual image
    bool ImageSolver::solveNormalEquations(askap::scimath::Quality& quality)
    {

      ASKAPLOG_INFO_STR(logger, "Calculating principal solution");

      // Solving A^T Q^-1 V = (A^T Q^-1 A) P
      uint nParameters=0;

      // Find all the free parameters beginning with image
      vector<string> names(itsParams->completions("image"));
      map<string, uint> indices;

      for (vector<string>::const_iterator it=names.begin(); it!=names.end(); it++)
      {
        string name="image"+*it;
        if (itsParams->isFree(name))
        {
          indices[name]=nParameters;
          nParameters+=itsParams->value(name).nelements();
        }
      }
      ASKAPCHECK(nParameters>0, "No free parameters in ImageSolver");

      for (map<string, uint>::const_iterator indit=indices.begin(); indit
          !=indices.end(); indit++)
      {
        // Axes are dof, dof for each parameter
        casa::IPosition arrShape(itsParams->value(indit->first).shape());
        casa::IPosition vecShape(1, itsParams->value(indit->first).nelements());
        ASKAPCHECK(normalEquations().normalMatrixDiagonal().count(indit->first)>0, "Diagonal not present for solution");
        const casa::Vector<double> & diag(normalEquations().normalMatrixDiagonal().find(indit->first)->second);
        ASKAPCHECK(normalEquations().dataVector(indit->first).size()>0, "Data vector not present for solution");
        const casa::Vector<double> &dv = normalEquations().dataVector(indit->first);
	ASKAPCHECK(normalEquations().normalMatrixSlice().count(indit->first)>0, "PSF Slice not present");
        const casa::Vector<double>& slice(normalEquations().normalMatrixSlice().find(indit->first)->second);

	casa::Array<float> dirtyArray(arrShape);
        casa::convertArray<float, double>(dirtyArray, dv.reform(arrShape));
        casa::Array<float> psfArray(arrShape);
        casa::convertArray<float, double>(psfArray, slice.reform(arrShape));
	
	// Normalize by the diagonal
	doNormalization(diag,tol(),psfArray,dirtyArray);

	// Do the preconditioning
	if(doPreconditioning(psfArray,dirtyArray))
	{
	
	 // Save the new PSFs to disk
         Axes axes(itsParams->axes(indit->first));
	 string psfName="psf."+(indit->first);
	 casa::Array<double> anothertemp(arrShape);
	 casa::convertArray<double,float>(anothertemp,psfArray);
	 const casa::Array<double> & APSF(anothertemp);
	 if (!itsParams->has(psfName)) {
		 itsParams->add(psfName, APSF, axes);
	 }
	 else {
		 itsParams->update(psfName, APSF);
	 }
	}

	casa::Vector<double> value(itsParams->value(indit->first).reform(vecShape));
	casa::Vector<float> dirtyVector(dirtyArray.reform(vecShape));
	for (uint elem=0; elem<dv.nelements(); elem++)
	{
		value(elem) += dirtyVector(elem);
	}
      }
      quality.setDOF(nParameters);
      quality.setRank(0);
      quality.setCond(0.0);
      quality.setInfo("Scaled residual calculated");
      
      /// Save the PSF and Weight
      saveWeights();
      savePSF();
      return true;
    }
    
    void ImageSolver::saveWeights()
    {
      // Save weights image
      vector<string> names(itsParams->completions("image"));
      for (vector<string>::const_iterator it=names.begin(); it!=names.end(); it++)
      {
        string name="image"+*it;
        if (normalEquations().normalMatrixDiagonal().count(name))
        {
          const casa::IPosition arrShape(normalEquations().shape().find(name)->second);
          Axes axes(itsParams->axes(name));
          string weightsName="weights"+*it;
          const casa::Array<double> & ADiag(normalEquations().normalMatrixDiagonal().find(name)->second.reform(arrShape));
          if (!itsParams->has(weightsName))
          {
            itsParams->add(weightsName, ADiag, axes);
          }
          else
          {
            itsParams->update(weightsName, ADiag);
          }
        }
      }
    }

    void ImageSolver::savePSF()
    {
      // Save PSF image
      vector<string> names(itsParams->completions("image"));
      for (vector<string>::const_iterator it=names.begin(); it!=names.end(); it++)
      {
        string name="image"+*it;
        if (normalEquations().normalMatrixSlice().count(name))
        {
          const casa::IPosition arrShape(normalEquations().shape().find(name)->second);
          Axes axes(itsParams->axes(name));
          string psfName="psf"+*it;
          const casa::Array<double> & APSF(normalEquations().normalMatrixSlice().find(name)->second.reform(arrShape));
          if (!itsParams->has(psfName))
          {
            itsParams->add(psfName, APSF, axes);
          }
          else
          {
            itsParams->update(psfName, APSF);
          }
        }
      }
    }

    /// @note itsPreconditioner is not cloned, only the ShPtr is.
    Solver::ShPtr ImageSolver::clone() const
    {
      return Solver::ShPtr(new ImageSolver(*this));
    }

    /// @return a reference to normal equations object
    /// @note In this class and derived classes the type returned
    /// by this method is narrowed to always provide image-specific 
    /// normal equations objects
    const scimath::ImagingNormalEquations& ImageSolver::normalEquations() const
    {
      try
      {
        return dynamic_cast<const scimath::ImagingNormalEquations&>(
            Solver::normalEquations());
      }
      catch (const std::bad_cast &bc)
      {
        ASKAPTHROW(AskapError, "An attempt to use incompatible normal "
            "equations class with image solver");
      }
    }

  }
}
