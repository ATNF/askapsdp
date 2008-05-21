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

        double maxDiag(casa::max(diag));
        ASKAPLOG_INFO_STR(logger, "Maximum of weights = " << maxDiag);
        const double cutoff=tol()*maxDiag;
	
	casa::Array<float> dirtyArray(arrShape);
        casa::convertArray<float, double>(dirtyArray, dv.reform(arrShape));
        casa::Array<float> psfArray(arrShape);
        casa::convertArray<float, double>(psfArray, slice.reform(arrShape));
	
	casa::Vector<float> dirtyVector(dirtyArray.reform(vecShape));
	casa::Vector<float> psfVector(psfArray.reform(vecShape));
	for (uint elem=0;elem<dv.nelements();elem++)
	{
          psfVector(elem)=slice(elem)/maxDiag;
	  if(diag(elem)>cutoff)
	  {
		  dirtyVector(elem)=dv(elem)/diag(elem);
	  }
	  else {
		  dirtyVector(elem)=dv(elem)/cutoff;
	  }
	}
	
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
	for (uint elem=0; elem<dv.nelements(); elem++)
	{
		value(elem) += dirtyVector(elem);
	}
	
	/* 
	   {
	   casa::Vector<double> value(itsParams->value(indit->first).reform(vecShape));
	   for (uint elem=0; elem<dv.nelements(); elem++)
	   {
	   if (diag(elem)>cutoff)
	   {
	   value(elem)+=dv(elem)/diag(elem);
	   }
	   else
	   {
	   value(elem)+=dv(elem)/cutoff;
	   }
	   }
	   }
	 */
	
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
      // Save weights image
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
