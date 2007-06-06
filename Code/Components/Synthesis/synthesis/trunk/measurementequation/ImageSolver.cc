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

void ImageSolver::init() {
	itsNormalEquations.reset();
}

// Solve for update simply by scaling the data vector by the diagonal term of the
// normal equations i.e. the residual image
bool ImageSolver::solveNormalEquations(Quality& quality) {
	
	// Solving A^T Q^-1 V = (A^T Q^-1 A) P
	uint nParameters=0;
		
	// Find all the free parameters
	const vector<string> names(itsParams.freeNames());
	if(names.size()<1) {
		throw(std::domain_error("No free parameters"));
	}
	vector<string>::const_iterator it;
	map<string, uint> indices;
	for (it=names.begin();it!=names.end();it++) {
		indices[*it]=nParameters;
		nParameters+=itsParams.value(*it).nelements();
	}
	if(nParameters<1) {
		throw(std::domain_error("No free parameters"));
	}

	map<string, uint>::iterator indit;
    for (indit=indices.begin();indit!=indices.end();indit++) {
	// Axes are dof, dof for each parameter
        casa::IPosition vecShape(1, itsParams.value(indit->first).nelements());
        const casa::Vector<double>& diag(itsNormalEquations.normalMatrixDiagonal()[indit->first]);
        const casa::Vector<double>& dv(itsNormalEquations.dataVector()[indit->first]);
        casa::Vector<double> value(itsParams.value(indit->first).reform(vecShape));
    	for (uint elem=0;elem<dv.nelements();elem++) {
            if(diag(elem)>0.0) {
                value(elem)+=dv(elem)/diag(elem);
            }
		}
	}

	quality.setDOF(nParameters);
	quality.setRank(0);
	quality.setCond(0.0);
	quality.setInfo("Scaled residual calculated");

	return true;
};

// Fully general solver from the design matrix
bool ImageSolver::solveDesignMatrix(Quality& quality) {
	return false;
};

}
}
