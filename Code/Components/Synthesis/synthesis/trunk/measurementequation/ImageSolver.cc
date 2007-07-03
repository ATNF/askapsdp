#include <measurementequation/ImageSolver.h>

#include <conrad/ConradError.h>

#include <casa/aips.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>

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
    
    ImageSolver::ImageSolver(const conrad::scimath::Params& ip) : 
          conrad::scimath::Solver(ip) 
    {
    }

    void ImageSolver::init()
    {
      itsNormalEquations.reset();
    }

// Solve for update simply by scaling the data vector by the diagonal term of the
// normal equations i.e. the residual image
    bool ImageSolver::solveNormalEquations(conrad::scimath::Quality& quality)
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
      CONRADCHECK(nParameters>0, "No free parameters in ImageSolver");
      
      for (map<string, uint>::const_iterator indit=indices.begin();indit!=indices.end();indit++)
      {
// Axes are dof, dof for each parameter
        casa::IPosition vecShape(1, itsParams->value(indit->first).nelements());
        
        CONRADCHECK(itsNormalEquations->normalMatrixDiagonal().count(indit->first)>0, "Diagonal not present");
        const casa::Vector<double>& diag(itsNormalEquations->normalMatrixDiagonal().find(indit->first)->second);
        CONRADCHECK(itsNormalEquations->dataVector().count(indit->first)>0, "Data vector not present");
        const casa::Vector<double>& dv(itsNormalEquations->dataVector().find(indit->first)->second);
        
        {
          casa::Vector<double> value(itsParams->value(indit->first).reform(vecShape));
          for (uint elem=0;elem<dv.nelements();elem++)
          {
            if(diag(elem)>0.0)
            {
              value(elem)+=dv(elem)/diag(elem);
            }
          }
        }

        /// Now write add some debug information but fix it to ensure that these 
        /// are not fit later on.
        if(verbose()) {              
        	Axes axes(itsParams->axes(indit->first));
          casa::IPosition valShape(itsParams->value(indit->first).shape());
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
      quality.setInfo("Scaled residual calculated");

      return true;
    };
    
    Solver::ShPtr ImageSolver::clone() const
    {
      return Solver::ShPtr(new ImageSolver(*this));
    }

  }
}
