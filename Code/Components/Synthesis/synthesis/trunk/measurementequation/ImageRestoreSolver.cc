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

	  casa::Array<float> dirtyArray(valShape);
          casa::convertArray<float, double>(dirtyArray, dv.reform(valShape));
          casa::Array<float> psfArray(valShape);
          casa::convertArray<float, double>(psfArray, slice.reform(valShape));

	  // Normalize by the diagonal
	  doNormalization(diag,tol(),psfArray,dirtyArray);
	  
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

  }
}



