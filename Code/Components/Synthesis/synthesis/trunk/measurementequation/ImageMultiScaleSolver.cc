#include <measurementequation/ImageMultiScaleSolver.h>

#include <conrad/ConradError.h>

#include <casa/aips.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/Arrays/Vector.h>

#include <lattices/Lattices/LatticeCleaner.h>
#include <lattices/Lattices/ArrayLattice.h>

using namespace casa;
using namespace conrad;
using namespace conrad::scimath;

#include <iostream>

#include <cmath>
using std::abs;

#include <map>
#include <vector>
#include <string>

using std::map;
using std::vector;
using std::string;

namespace conrad
{
  namespace synthesis
  {
    
    ImageMultiScaleSolver::ImageMultiScaleSolver(const conrad::scimath::Params& ip) : 
          conrad::scimath::Solver(ip) 
    {
      itsScales.resize(4);
      itsScales(0)=0;
      itsScales(1)=3;
      itsScales(2)=10;
      itsScales(3)=30;
    }

    void ImageMultiScaleSolver::init()
    {
      itsNormalEquations.reset();
    }

// Solve for update simply by scaling the data vector by the diagonal term of the
// normal equations i.e. the residual image
    bool ImageMultiScaleSolver::solveNormalEquations(conrad::scimath::Quality& quality)
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
      CONRADCHECK(nParameters>0, "No free parameters in ImageMultiScaleSolver");
      
      for (map<string, uint>::const_iterator indit=indices.begin();indit!=indices.end();indit++)
      {
// Axes are dof, dof for each parameter
        const casa::IPosition vecShape(1, itsParams->value(indit->first).nelements());
        const casa::IPosition valShape(itsParams->value(indit->first).shape());
        
        CONRADCHECK(itsNormalEquations->normalMatrixDiagonal().count(indit->first)>0, "Diagonal not present");
        const casa::Vector<double>& diag(itsNormalEquations->normalMatrixDiagonal().find(indit->first)->second);
        CONRADCHECK(itsNormalEquations->dataVector().count(indit->first)>0, "Data vector not present");
        const casa::Vector<double>& dv(itsNormalEquations->dataVector().find(indit->first)->second);
        CONRADCHECK(itsNormalEquations->normalMatrixSlice().count(indit->first)>0, "Data vector not present");
        const casa::Vector<double>& slice(itsNormalEquations->normalMatrixSlice().find(indit->first)->second);
        
        casa::Array<float> dirtyArray(valShape);
        casa::convertArray<float, double>(dirtyArray, diag.reform(valShape));
        casa::Array<float> psfArray(valShape);
        casa::convertArray<float, double>(psfArray, slice.reform(valShape));
        casa::Array<float> cleanArray(valShape);
        casa::convertArray<float, double>(cleanArray, itsParams->value(indit->first));
        {
          casa::Vector<float> dirtyVector(dirtyArray.reform(vecShape));
          casa::Vector<float> psfVector(psfArray.reform(vecShape));
          for (uint elem=0;elem<dv.nelements();elem++)
          {
            if(diag(elem)>0.0)
            {
              dirtyVector(elem)=dv(elem)/diag(elem);
              psfVector(elem)=slice(elem)/diag(elem);
            }
            else {
              dirtyVector(elem)=0.0;
              psfVector(elem)=0.0;
            }
          }
        }
        
        // We need lattice equivalents. We can use ArrayLattice which involves
        // no copying
        casa::ArrayLattice<float> dirty(dirtyArray);
        casa::ArrayLattice<float> psf(psfArray);
        casa::ArrayLattice<float> clean(cleanArray);
                
        // Create a lattice cleaner to do the dirty work :)
        casa::LatticeCleaner<float> lc(psf, dirty);
        if(algorithm()=="Hogbom") {
          casa::Vector<float> scales(1);
          scales(0)=0.0;
          lc.setscales(scales);
          lc.setcontrol(casa::CleanEnums::HOGBOM, niter(), gain(), threshold(), false);
        }
        else {
          lc.setscales(itsScales);
          lc.setcontrol(casa::CleanEnums::MULTISCALE, niter(), gain(), threshold(), false);
        }
        lc.clean(clean);

        casa::convertArray<double, float>(itsParams->value(indit->first), cleanArray);

        /// Now write add some debug information but fix it to ensure that these 
        /// are not fit later on.
        if(verbose()) {              
        	Axes axes(itsParams->axes(indit->first));
        	{
        	  casa::Array<double> value(itsNormalEquations->normalMatrixDiagonal().find(indit->first)->second.reform(valShape));
            itsParams->add("debug."+indit->first+".diagonal", value, axes);
            itsParams->fix("debug."+indit->first+".diagonal");
        	}
        	{
        	  casa::Array<double> value(itsNormalEquations->dataVector().find(indit->first)->second.reform(valShape));
        	  itsParams->add("debug."+indit->first+".dataVector", value, axes);
            itsParams->fix("debug."+indit->first+".dataVector");
        	}
        	{
        	  casa::Array<double> value(itsNormalEquations->normalMatrixSlice().find(indit->first)->second.reform(valShape));
            itsParams->add("debug."+indit->first+".slice", value, axes);
            itsParams->fix("debug."+indit->first+".slice");
        	}
        }
      }

      quality.setDOF(nParameters);
      quality.setRank(0);
      quality.setCond(0.0);
      quality.setInfo("Multiscale Clean");

      return true;
    };
    
    Solver::ShPtr ImageMultiScaleSolver::clone() const
    {
      return Solver::ShPtr(new ImageMultiScaleSolver(*this));
    }

    void ImageMultiScaleSolver::setScales(const casa::Vector<float>& scales) 
    {
      itsScales.resize(scales.size());
      itsScales=scales;
    }
  }
}
