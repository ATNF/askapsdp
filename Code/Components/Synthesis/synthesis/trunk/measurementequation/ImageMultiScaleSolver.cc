#include <measurementequation/ImageMultiScaleSolver.h>

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
          ImageSolver(ip) 
    {
      itsScales.resize(3);
      itsScales(0)=0;
      itsScales(1)=10;
      itsScales(2)=30;
      itsRobustness=0.0;
    }

    ImageMultiScaleSolver::ImageMultiScaleSolver(const askap::scimath::Params& ip,
      const casa::Vector<float>& scales) : 
          ImageSolver(ip)
    {
      itsScales.resize(scales.size());
      itsScales=scales;
      itsRobustness=0.0;
    }
    
    ImageMultiScaleSolver::ImageMultiScaleSolver(const askap::scimath::Params& ip,
      const casa::Vector<float>& scales, const float& robust) : 
          ImageSolver(ip),itsRobustness(robust)
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
      
      for (map<string, uint>::const_iterator indit=indices.begin();indit!=indices.end();indit++)
      {
// Axes are dof, dof for each parameter
        const casa::IPosition vecShape(1, itsParams->value(indit->first).nelements());
        const casa::IPosition valShape(itsParams->value(indit->first).shape());
        
        ASKAPCHECK(normalEquations().normalMatrixDiagonal().count(indit->first)>0, "Diagonal not present");
        const casa::Vector<double>& diag(normalEquations().normalMatrixDiagonal().find(indit->first)->second);
        ASKAPCHECK(normalEquations().dataVector(indit->first).size()>0, "Data vector not present");
        const casa::Vector<double>& dv = normalEquations().dataVector(indit->first);
        ASKAPCHECK(normalEquations().normalMatrixSlice().count(indit->first)>0, "PSF Slice not present");
        const casa::Vector<double>& slice(normalEquations().normalMatrixSlice().find(indit->first)->second);
        
        casa::Array<float> dirtyArray(valShape);
        casa::convertArray<float, double>(dirtyArray, diag.reform(valShape));
        casa::Array<float> psfArray(valShape);
        casa::convertArray<float, double>(psfArray, slice.reform(valShape));
        casa::Array<float> cleanArray(valShape);
        casa::convertArray<float, double>(cleanArray, itsParams->value(indit->first));
				double maxDiag(casa::max(diag));
				ASKAPLOG_INFO_STR(logger, "Maximum of weights = " << maxDiag );
				double cutoff=tol()*maxDiag;
        {
          casa::Vector<float> dirtyVector(dirtyArray.reform(vecShape));
          casa::Vector<float> psfVector(psfArray.reform(vecShape));
          for (uint elem=0;elem<dv.nelements();elem++)
          {
	    // For the PSF, we really need a separate version for each pixel. However, 
	    // a reasonable approximation is to just divide by the maximum weight.
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
        
        // We need lattice equivalents. We can use ArrayLattice which involves
        // no copying
        casa::ArrayLattice<float> dirty(dirtyArray);
        casa::ArrayLattice<float> psf(psfArray);
        casa::ArrayLattice<float> clean(cleanArray);
      
	// Precondition the PSF and DIRTY images before solving.
	if(itsRobustness > 1e-06)
	{
	  preconditionNE(psf,dirty);
	  
	  // Renormalize the PSF and dirty image
	  float maxpsf = max(psfArray);
	  psfArray/=maxpsf;
	  dirtyArray/=maxpsf;
	  
	  // Save the new PSFs to disk
          Axes axes(itsParams->axes(indit->first));
          string psfName="psf."+(indit->first);
          casa::Array<double> anothertemp(valShape);
          casa::convertArray<double,float>(anothertemp,psfArray);
          const casa::Array<double> & APSF(anothertemp);
          if (!itsParams->has(psfName)) {
            itsParams->add(psfName, APSF, axes);
          }
          else {
            itsParams->update(psfName, APSF);
          }
	}
	// preconditioning done.
	
        // Create a lattice cleaner to do the dirty work :)
        /// @todo More checks on reuse of LatticeCleaner
        boost::shared_ptr<casa::LatticeCleaner<float> > lc;
        std::map<string, boost::shared_ptr<casa::LatticeCleaner<float> > >::const_iterator it =
                                       itsCleaners.find(indit->first);
        
        
        // temporary commented out to see whether local variables cause the bug
        /*
        if(it!=itsCleaners.end()) {
          lc=it->second; 
          ASKAPDEBUGASSERT(lc);
          lc->update(dirty);
        }
        else {
          lc.reset(new casa::LatticeCleaner<float>(psf, dirty));
          itsCleaners[indit->first]=lc;          
        }
        */
        lc.reset(new casa::LatticeCleaner<float>(psf, dirty));          
        // end of the temporary altered section, the previous line to be removed
        // when uncommented
        
        ASKAPDEBUGASSERT(lc);
        if(algorithm()=="Hogbom") {
          casa::Vector<float> scales(1);
          scales(0)=0.0;
          lc->setscales(scales);
          lc->setcontrol(casa::CleanEnums::HOGBOM, niter(), gain(), threshold(), false);
        }
        else {
          lc->setscales(itsScales);
          lc->setcontrol(casa::CleanEnums::MULTISCALE, niter(), gain(), threshold(), false);
        }
        lc->ignoreCenterBox(true);
        lc->clean(clean);

        casa::convertArray<double, float>(itsParams->value(indit->first), cleanArray);
      }

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

    
    void ImageMultiScaleSolver::preconditionNE(casa::ArrayLattice<float>& psf, 
		                               casa::ArrayLattice<float>& dirty)
    {
       // Setup work arrays.
       IPosition valShape = psf.shape();
       casa::ArrayLattice<casa::Complex> weinerfilter(valShape);
       casa::ArrayLattice<casa::Complex> scratch(valShape);
       
       // Construct a Weiner filter
       scratch.copyData(casa::LatticeExpr<casa::Complex>(toComplex(psf)));
       LatticeFFT::cfft2d(scratch, True);
       casa::LatticeExpr<casa::Complex> wf(conj(scratch)/(scratch*conj(scratch) + itsRobustness));
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
    
  }
}


