#include <measurementequation/MESVDSolver.h>

#include <stdexcept>

#include <casa/aips.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_linalg.h>

#include <iostream>


namespace conrad
{
namespace synthesis
{


void MESVDSolver::init() {
	itsNormalEquations.reset();
	itsDesignMatrix.reset();
}

bool MESVDSolver::solveNormalEquations(MEQuality& quality) {
	return false;
};

// Solve for scalar parameters from the designmatrix
bool MESVDSolver::solveDesignMatrix(MEQuality& quality) {

	uint nParameters=0;
	uint nData=0;
	nData=itsDesignMatrix.residual().nelements();
	
	// Find all the scalar, free parameters
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

    // Convert the design matrix to gsl format
	gsl_matrix * A = gsl_matrix_alloc (nData, nParameters);
	map<string, uint>::iterator indit;
	for (indit=indices.begin();indit!=indices.end();indit++) {
		// Axes are data, dof for each parameter
		const casa::Matrix<double>& deriv(itsDesignMatrix.derivative(indit->first));
		for (uint row=0;row<deriv.nrow();row++) {
			for (uint col=0;col<deriv.ncolumn();col++) {
				gsl_matrix_set(A, row, col+indit->second, deriv(row,col));
			}
		}
	}
	
	// Call the gsl SVD function, A is overwritten by U
	gsl_matrix * V = gsl_matrix_alloc (nParameters, nParameters);
	gsl_vector * S = gsl_vector_alloc (nParameters);
	gsl_vector * work = gsl_vector_alloc (nParameters);
	gsl_linalg_SV_decomp (A, V, S, work);

	// Now find the solution for the residual vector
	gsl_vector * res = gsl_vector_alloc(nData);
	for (uint i=0;i<nData;i++) {
		gsl_vector_set(res, i, itsDesignMatrix.residual()[i]);
	}
	gsl_vector * x = gsl_vector_alloc(nParameters);
	gsl_linalg_SV_solve (A, V, S, res, x); 
	
	// Update the parameters for the calculated changes
	for (indit=indices.begin();indit!=indices.end();indit++) {
		casa::Vector<double>& value(itsParams.value(indit->first));
		for (uint i=0;i<value.nelements();i++) {
			value(i)+=gsl_vector_get(x, indit->second+i);
		}
		itsParams.update(indit->first, value);
	}
	// Now find the statistics for the decomposition
	uint rank=0;
	double smin=1e50;
	double smax=0.0;
	for (uint i=0;i<nParameters;i++) {
		double sValue=abs(gsl_vector_get(S, i));
		if(sValue>0.0) 
		{
			rank++;
			if(sValue>smax) smax=sValue;
			if(sValue<smin) smin=sValue;
		}
	}
	quality.setRank(rank);
	quality.setCond(smax/smin);
	if(rank==nParameters) {
		quality.setInfo("SVD decomposition rank complete");
	}
	else {
		quality.setInfo("SVD decomposition rank deficient");
	}
	// Free up gsl storage
	gsl_vector_free(S);
	gsl_vector_free(work);
	gsl_matrix_free(V);
	gsl_matrix_free(A);

	return true;
};

}
}