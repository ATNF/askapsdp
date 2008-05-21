#include <measurementequation/ImageRestoreSolver.h>
#include <measurementequation/SynthesisParamsHelper.h>

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
	ASKAPCHECK(nParameters>0, "No free parameters in ImageRestoreSolver");

	for (map<string, uint>::const_iterator indit=indices.begin(); indit !=indices.end(); indit++)
	{
	  ASKAPLOG_INFO_STR(logger, "Restoring " << indit->first );
	  // Axes are dof, dof for each parameter
	  casa::IPosition vecShape(1, itsParams->value(indit->first).nelements());
	  const casa::IPosition valShape(itsParams->value(indit->first).shape());
 
	  ASKAPCHECK(normalEquations().normalMatrixDiagonal().count(indit->first)>0, "Diagonal not present");
	  const casa::Vector<double>& diag(normalEquations().normalMatrixDiagonal().find(indit->first)->second);
	  ASKAPCHECK(normalEquations().dataVector(indit->first).size()>0, "Data vector not present");
	  const casa::Vector<double> &dv = normalEquations().dataVector(indit->first);
	  ASKAPCHECK(normalEquations().normalMatrixSlice().count(indit->first)>0, "PSF Slice not present");
          const casa::Vector<double>& slice(normalEquations().normalMatrixSlice().find(indit->first)->second);

	  double maxDiag(casa::max(diag));
	  ASKAPLOG_INFO_STR(logger, "Maximum of weights = " << maxDiag );
	  double cutoff=tol()*maxDiag;
	  
	  // Make the residual image, and apply the Weiner filter to it
	  casa::Array<float> dirtyArray(valShape);
          casa::convertArray<float, double>(dirtyArray, diag.reform(valShape));
          casa::Array<float> psfArray(valShape);
          casa::convertArray<float, double>(psfArray, slice.reform(valShape));
	  {
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
		       dirtyVector(elem)=0.0;
	       }
	     }
	  }
	  
	  // Do the preconditioning
	  doPreconditioning(psfArray,dirtyArray);
	  
	  // We need lattice equivalents. We can use ArrayLattice which involves
	  // no copying
	  casa::ArrayLattice<float> dirty(dirtyArray);
	  casa::ArrayLattice<float> psf(psfArray);

	  // Create a temporary image
	  boost::shared_ptr<casa::TempImage<float> > image(SynthesisParamsHelper::tempImage(*itsParams, indit->first));
	  casa::Image2DConvolver<float> convolver;	
	  const casa::IPosition pixelAxes(2, 0, 1);	
	  casa::LogIO logio;
	  convolver.convolve(logio, *image, *image, casa::VectorKernel::GAUSSIAN,
			  pixelAxes, itsBeam, true, 1.0, false);
	  SynthesisParamsHelper::update(*itsParams, indit->first, *image);

	  // Add the residual image        
	  {
	    casa::Vector<double> value(itsParams->value(indit->first).reform(vecShape));
	    casa::Vector<float> dirtyVector(dirtyArray.reform(vecShape));
	    for (uint elem=0; elem<dv.nelements(); elem++)
	    {
	      value(elem) += dirtyVector(elem);
	    }
	  
	  }  
	}
	
	quality.setDOF(nParameters);
	quality.setRank(0);
	quality.setCond(0.0);
	quality.setInfo("Restored image calculated");
	
	return true;
    };
	
    Solver::ShPtr ImageRestoreSolver::clone() const
    {
	    return Solver::ShPtr(new ImageRestoreSolver(*this));
    }
        
    /*
    void ImageRestoreSolver::applyWeinerFilter(casa::ArrayLattice<float>& psf, 
		                               casa::ArrayLattice<float>& dirty,
					       float& noisepower)
    {
       // Setup work arrays.
       casa::IPosition valShape(psf.shape());
       casa::ArrayLattice<casa::Complex> weinerfilter(valShape);
       casa::ArrayLattice<casa::Complex> scratch(valShape);
       
       // Construct a Weiner filter from the PSF
       scratch.copyData(casa::LatticeExpr<casa::Complex>(toComplex(psf)));
       LatticeFFT::cfft2d(scratch, True);
       casa::LatticeExpr<casa::Complex> wf(conj(scratch)/(scratch*conj(scratch) + noisepower));
       weinerfilter.copyData(wf);

       // Apply the filter to the psf
       // (reuse the ft(psf) currently held in 'scratch')
       scratch.copyData(casa::LatticeExpr<casa::Complex> (weinerfilter * scratch));
       LatticeFFT::cfft2d(scratch, False);
       psf.copyData(casa::LatticeExpr<float> ( real(scratch) ));
       
       // Apply the filter to the dirty image
       scratch.copyData(casa::LatticeExpr<casa::Complex>(toComplex(dirty)));
       LatticeFFT::cfft2d(scratch, True);
       scratch.copyData(casa::LatticeExpr<casa::Complex> (weinerfilter * scratch));
       LatticeFFT::cfft2d(scratch, False);
       dirty.copyData(casa::LatticeExpr<float> ( real(scratch) ));
    };
    */
    

  }
}



