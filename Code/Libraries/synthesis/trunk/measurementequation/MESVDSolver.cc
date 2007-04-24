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

    // Convert the normal equations to gsl format
    // Note that A is complex but hermitean so it has the
    // right number of independent terms (nParameter*nParameter)
    // although the matrix is bigger. It might be worth
    // packing to a purely real format.
	gsl_matrix * A = gsl_matrix_alloc (nParameters, nParameters);
	gsl_vector * B = gsl_vector_alloc (nParameters);
	gsl_vector * X = gsl_vector_alloc (nParameters);

	map<string, uint>::iterator indit1;
	map<string, uint>::iterator indit2;
	for (indit2=indices.begin();indit2!=indices.end();indit2++) {
		for (indit1=indices.begin();indit1!=indices.end();indit1++) {
		// Axes are dof, dof for each parameter
			const casa::Matrix<double>& nm(itsNormalEquations.normalMatrix()[indit1->first][indit2->first]);
			for (uint row=0;row<nm.nrow();row++) {
				for (uint col=0;col<nm.ncolumn();col++) {
					gsl_matrix_set(A, row+(indit1->second), col+(indit2->second), nm(row,col));
				}
			}
		}
	}
	for (indit1=indices.begin();indit1!=indices.end();indit1++) {
		const casa::Vector<double>& dv(itsNormalEquations.dataVector()[indit1->first]);
		for (uint row=0;row<dv.nelements();row++) {
			gsl_vector_set(B, row+(indit1->second), dv(row));
		}
	}
	
	gsl_linalg_cholesky_decomp(A);
	gsl_linalg_cholesky_solve(A, B, X);
	
	// Update the parameters for the calculated changes
	map<string, uint>::iterator indit;
	for (indit=indices.begin();indit!=indices.end();indit++) {
		casa::Vector<double>& value(itsParams.value(indit->first));
		for (uint i=0;i<value.nelements();i++) {
			value(i)+=gsl_vector_get(X, indit->second+i);
		}
		itsParams.update(indit->first, value);
	}
	quality.setInfo("LU decomposition rank complete");

	// Free up gsl storage
	gsl_vector_free(B);
	gsl_matrix_free(A);
	gsl_vector_free(X);

	return true;
};

// Solve for parameters from the designmatrix
bool MESVDSolver::solveDesignMatrix(MEQuality& quality) {

	uint nParameters=0;
	uint nData=0;
	nData=itsDesignMatrix.residual().nelements();
	
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

    // Convert the design matrix to gsl format
	gsl_matrix * A = gsl_matrix_alloc (2*nData, nParameters);
	map<string, uint>::iterator indit;
	for (indit=indices.begin();indit!=indices.end();indit++) {
		// Axes are data, dof for each parameter
		const casa::Matrix<casa::DComplex>& deriv(itsDesignMatrix.derivative(indit->first));
		for (uint i=0;i<nData;i++) {
			for (uint col=0;col<deriv.ncolumn();col++) {
				gsl_matrix_set(A, 2*i, col+indit->second, real(deriv(i,col)));
				gsl_matrix_set(A, 2*i+1, col+indit->second, imag(deriv(i,col)));
			}
		}
	}
	
	// Call the gsl SVD function, A is overwritten by U
	gsl_matrix * V = gsl_matrix_alloc (nParameters, nParameters);
	gsl_vector * S = gsl_vector_alloc (nParameters);
	gsl_vector * work = gsl_vector_alloc (nParameters);
	gsl_linalg_SV_decomp (A, V, S, work);

	// Now find the solution for the residual vector
	gsl_vector * res = gsl_vector_alloc(2*nData);
	for (uint i=0;i<nData;i++) {
		gsl_vector_set(res, 2*i, real(itsDesignMatrix.residual()[i]));
		gsl_vector_set(res, 2*i+1, imag(itsDesignMatrix.residual()[i]));
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