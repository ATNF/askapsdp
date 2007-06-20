#include <measurementequation/ImageSolver.h>

#include <stdexcept>

#include <casa/aips.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>

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
      vector<string>::const_iterator it;
      map<string, uint> indices;
      
      for (it=names.begin();it!=names.end();it++)
      {
        string name="image"+*it;
        if(itsParams->isFree(name)) {
          indices[name]=nParameters;
          nParameters+=itsParams->value(name).nelements();
        }
      }
      if(nParameters<1)
      {
        throw(std::domain_error("No free parameters"));
      }

      map<string, uint>::iterator indit;
      for (indit=indices.begin();indit!=indices.end();indit++)
      {
// Axes are dof, dof for each parameter
        casa::IPosition vecShape(1, itsParams->value(indit->first).nelements());
        const casa::Vector<double>& diag(itsNormalEquations->normalMatrixDiagonal()[indit->first]);
        const casa::Vector<double>& dv(itsNormalEquations->dataVector()[indit->first]);
        casa::Vector<double> value(itsParams->value(indit->first).reform(vecShape));
        for (uint elem=0;elem<dv.nelements();elem++)
        {
          if(diag(elem)>0.0)
          {
            value(elem)+=dv(elem)/diag(elem);
          }
        }
        itsParams->add("debug."+indit->first+".diagonal", itsNormalEquations->normalMatrixDiagonal()[indit->first]);
        itsParams->add("debug."+indit->first+".dataVector", itsNormalEquations->dataVector()[indit->first]);
        itsParams->add("debug."+indit->first+".slice", itsNormalEquations->normalMatrixSlice()[indit->first]);
      }

      quality.setDOF(nParameters);
      quality.setRank(0);
      quality.setCond(0.0);
      quality.setInfo("Scaled residual calculated");

      return true;
    };

  }
}
