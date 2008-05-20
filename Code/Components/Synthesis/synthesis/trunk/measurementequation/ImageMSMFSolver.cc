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
          ImageSolver(ip),itsNTaylor(2)
    {
      itsScales.resize(3);
      itsScales(0)=0;
      itsScales(1)=10;
      itsScales(2)=30;
      itsRobustness=0.0;
      itsNPsfTaylor = 2*itsNTaylor-1;
      dbg=True;
    }

    ImageMSMFSolver::ImageMSMFSolver(const askap::scimath::Params& ip,
      const casa::Vector<float>& scales, const int& nterms, const float& robust) : 
          ImageSolver(ip),itsNTaylor(nterms),itsRobustness(robust)
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

// Solve for update simply by scaling the data vector by the diagonal term of the
// normal equations i.e. the residual image
    bool ImageMSMFSolver::solveNormalEquations(askap::scimath::Quality& quality)
    {

// Solving A^T Q^-1 V = (A^T Q^-1 A) P
      uint nParameters=0;

// Find all the free parameters beginning with image
      vector<string> names(itsParams->completions("image"));
      map<string, uint> indices;
      std::string stokes = "n";
      map<uint, string> stokeslist;
      int nstokes=0;

      for (vector<string>::const_iterator  it=names.begin();it!=names.end();it++)
      {
        string name="image"+*it;
        if(itsParams->isFree(name)) {
          indices[name]=nParameters;
          nParameters+=itsParams->value(name).nelements();
	  
	  // Pick out the Stokes parameter
	  if (stokes != it->substr(1,1)) 
	  {
	    stokes = it->substr(1,1);
	    std::cout << "Read input for stokes " << stokes << endl;
	    stokeslist[nstokes] = stokes;
	    nstokes++;
	  }
	}
      }
      ASKAPCHECK(nParameters>0, "No free parameters in ImageMSMFSolver");

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
      //    for ( order in [0:ntaylor-1] )
      //        latticecleaner[stokes]->setresidual(order,residual[order]);
      //        latticecleaner[stokes]->setmodel(order,model[order]);
      //    latticecleaner[stokes]->mtclean();
      //    for ( order in [0:ntaylor-1] )
      //        latticecleaner[stokes]->getmodel(order,model[order]);
      //

      
      try
      {
      
        if(dbg)ASKAPLOG_INFO_STR(logger, "There are " << nstokes << " stokes parameters to solve for." );
	
	static bool firsttime=True;
	
	// Iterate through Stokes parameters.
	for (int sindex=0;sindex<nstokes;sindex++)
	{
	  if(dbg)ASKAPLOG_INFO_STR(logger, "In Image MSMFSSolver::solveN..E.. : About to iterate for Stokes " << stokes );
	  
	  stokes = stokeslist[sindex];
	  std::string samplename(indices.begin()->first);

	  // Setup the psf and residual normalization vector
	  std::string imagename(makeImageString(samplename,stokes,0));
	  std::cout << "Reading the normalization vector from : " << imagename << std::endl;
	  ASKAPCHECK(normalEquations().normalMatrixDiagonal().count(imagename)>0, "Diagonal not present");
	  const casa::Vector<double>& normdiag(normalEquations().normalMatrixDiagonal().find(imagename)->second);
	  const casa::IPosition vecShape(1, itsParams->value(imagename).nelements());
	  const casa::IPosition valShape(itsParams->value(imagename).shape());
	  double maxDiag(casa::max(normdiag));
	  ASKAPLOG_INFO_STR(logger, "Maximum of weights = " << maxDiag );
	  double cutoff=tol()*maxDiag;

	  // Array to store the Weiner filter
	  //casa::ArrayLattice<casa::Complex> weinerfilter(valShape);
	  casa::ArrayLattice<casa::Complex> scratch(valShape);
	  float maxpsf = 1.0;

	  if(firsttime) // Initialize everything only once.
	  {
	    // Initialize the latticecleaners
	    if(dbg)ASKAPLOG_INFO_STR(logger, "Initialising the solver for Stokes " << stokes);
	    
	    (itsCleaners[stokes]).reset(new casa::MultiTermLatticeCleaner<float>());
	    ASKAPDEBUGASSERT((itsCleaners[stokes]));
	    
	    (itsCleaners[stokes])->setcontrol(casa::CleanEnums::MULTISCALE, niter(), gain(), threshold(), false);
	    (itsCleaners[stokes])->ignoreCenterBox(true);
	    (itsCleaners[stokes])->setscales(itsScales);
	    (itsCleaners[stokes])->setntaylorterms(itsNTaylor);
	    (itsCleaners[stokes])->initialise(); // allocates memory once....

            // Setup the PSFs - all ( 2 x ntaylor - 1 ) of them
	    for( int order=0; order < 2*itsNTaylor-1; order++)
	    {
	      imagename = makeImageString(samplename,stokes,order);
	      ASKAPCHECK(normalEquations().normalMatrixSlice().count(imagename)>0, "PSF Slice not present");
	      const casa::Vector<double>& slice(normalEquations().normalMatrixSlice().find(imagename)->second);
	      casa::Array<float> psfArray(valShape);
	      casa::convertArray<float, double>(psfArray, slice.reform(valShape));
	      {
	       casa::Vector<float> psfVector(psfArray.reform(vecShape));
	       for (uint elem=0;elem<normdiag.nelements();elem++) psfVector(elem)=slice(elem)/maxDiag;
	      }
	      casa::ArrayLattice<float> psf(psfArray);
	      
	      ASKAPLOG_INFO_STR(logger, "Preconditioning PSF for stokes " << stokes << " and order " << order );
	      if( itsRobustness > 1e-06 )
	      {
	        if( order==0 )
		{	  
	          // For PSF0, construct the filter.
	  	  itsWeinerFilter = casa::ArrayLattice<casa::Complex>(valShape);
		  scratch.copyData(casa::LatticeExpr<casa::Complex>(toComplex(psf)));
		  LatticeFFT::cfft2d(scratch, True);
		  casa::LatticeExpr<casa::Complex> wf(conj(scratch)/(scratch*conj(scratch) + itsRobustness));
		  itsWeinerFilter.copyData(wf);
		}
	        // Apply the filter
		scratch.copyData(casa::LatticeExpr<casa::Complex>(toComplex(psf)));
		LatticeFFT::cfft2d(scratch, True);
		scratch.copyData(casa::LatticeExpr<casa::Complex> (itsWeinerFilter * scratch));
		LatticeFFT::cfft2d(scratch, False);	    
		psf.copyData(casa::LatticeExpr<float> ( real(scratch) ));
		// Re-normalize
	        if( order==0) 
		{
		  maxpsf = max(psfArray);
		}
		psfArray/=maxpsf;
	      }
	      itsCleaners[stokes]->setpsf(order,psf);
	      
	      // Write PSFs to disk.
	      ASKAPLOG_INFO_STR(logger, "Writing psfs to disk");
              Axes axes(itsParams->axes(imagename));
	      string psfName="psf."+(imagename);
	      casa::Array<double> aargh(valShape);
	      casa::convertArray<double,float>(aargh,psfArray);
	      const casa::Array<double> & APSF(aargh);
	      //const casa::Array<double> & APSF(normalEquations().normalMatrixSlice().find(indit->first)->second.reform(arrShape));
	      if (!itsParams->has(psfName))
	      {
		      itsParams->add(psfName, APSF, axes);
	      }
	      else
	      {
		      itsParams->update(psfName, APSF);
	      }
	    }
	  }// end of firsttime 
	    
	  ASKAPLOG_INFO_STR(logger, "Setting up Residual images");
	  // Setup the Residual Images and Model Images  - ( ntaylor ) of them
	  for( int order=0; order < itsNTaylor; order++)
	  {
	    imagename = makeImageString(samplename,stokes,order);
	    
	    ASKAPCHECK(normalEquations().dataVector(imagename).size()>0, "Data vector not present");
	    const casa::Vector<double>& dv = normalEquations().dataVector(imagename);
	    
	    casa::Array<float> dirtyArray(valShape);
	    casa::convertArray<float, double>(dirtyArray, normdiag.reform(valShape));
	    casa::Array<float> cleanArray(valShape);
	    casa::convertArray<float, double>(cleanArray, itsParams->value(imagename));
	   
	    {
	     casa::Vector<float> dirtyVector(dirtyArray.reform(vecShape));
	     for (uint elem=0;elem<dv.nelements();elem++)
	     {
		    if(normdiag(elem)>cutoff)
			    dirtyVector(elem)=dv(elem)/normdiag(elem);
		    else 
			    dirtyVector(elem)=0.0;
	     }
	    }
	    
	    // We need lattice equivalents. We can use ArrayLattice which involves
	    // no copying
	    casa::ArrayLattice<float> dirty(dirtyArray);
	    casa::ArrayLattice<float> clean(cleanArray);

	    if( itsRobustness > 1e-06 )
	    {
	       ASKAPLOG_INFO_STR(logger, "Applying the Weiner filter to the resid images");
	       // Apply the Weiner filter to the dirty images.
	       scratch.copyData(casa::LatticeExpr<casa::Complex>(toComplex(dirty)));
	       LatticeFFT::cfft2d(scratch, True);
	       scratch.copyData(casa::LatticeExpr<casa::Complex> (itsWeinerFilter * scratch));
	       LatticeFFT::cfft2d(scratch, False);
	       dirty.copyData(casa::LatticeExpr<float> ( real(scratch) ));
	       // Re-normalize
	       dirtyArray/=maxpsf;
	    }
	    
	    // Send in Dirty images only for ntaylor terms
	    itsCleaners[stokes]->setresidual(order,dirty);
	    itsCleaners[stokes]->setmodel(order,clean);
	    
	  }// end of order iteration
	  
	  
	  ASKAPLOG_INFO_STR(logger, "Finished Setup. Starting Minor Cycles" );
	  itsCleaners[stokes]->mtclean();
	  ASKAPLOG_INFO_STR(logger, "Finished Minor Cycles." );
	  
	  // Write the final vector of clean model images into casa:images with the correct names.
	  for( int order=0; order < itsNTaylor; order++)
	  {
		  imagename = makeImageString(samplename,stokes,order);
		  const casa::IPosition valShape(itsParams->value(imagename).shape());
		  casa::Array<float> cleanArray(valShape);
		  casa::ArrayLattice<float> clean(cleanArray);
		  ASKAPLOG_INFO_STR(logger, "About to get model" );
		  itsCleaners[stokes]->getmodel(order,clean);
		  casa::convertArray<double, float>(itsParams->value(imagename), cleanArray);
	  }
	}// end of stokes loop 
	
	// Make sure that the next set of minor cycles does not redo unnecessary things.
	// Also "fix" parameters for order >= itsNTaylor. so that the gridding doesn't get done
	// for these extra terms.

	if(firsttime)
	{
	  // Fix the params corresponding to extra Taylor terms.
	  
	  for (vector<string>::const_iterator  it=names.begin();it!=names.end();it++)
	  {
		  string name="image"+*it;
		  int torder = getOrder(name);
		  if(torder >= itsNTaylor && itsParams->isFree(name)) itsParams->fix(name);
	  }
	  
	  firsttime = False;
	}
      }
      catch( AipsError &x )
      {
	      throw AskapError("Failed in the MSMFS Minor Cycle : " + x.getMesg() );
      }
      
      quality.setDOF(nParameters);
      quality.setRank(0);
      quality.setCond(0.0);
      quality.setInfo("Multi-Scale Multi-Frequency Clean");
      
      /// Save the PSF and Weight
      saveWeights();
      savePSF();

      return true;
    };
    
  }
}
