#include <fitting/LinearSolver.h>

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
namespace scimath
{


void LinearSolver::init() {
	itsNormalEquations.reset();
	itsDesignMatrix.reset();
}

// Fully general solver for the normal equations for any shape 
// parameters.
bool LinearSolver::solveNormalEquations(Quality& quality, const bool useSVD) {
	
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
	
	if(useSVD) {
		gsl_matrix * V = gsl_matrix_alloc (nParameters, nParameters);
		gsl_vector * S = gsl_vector_alloc (nParameters);
		gsl_vector * work = gsl_vector_alloc (nParameters);
		gsl_linalg_SV_decomp (A, V, S, work);

		gsl_vector * X = gsl_vector_alloc(nParameters);
		gsl_linalg_SV_solve (A, V, S, B, X);
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
		quality.setDOF(nParameters);
		quality.setRank(rank);
		quality.setCond(smax/smin);
		if(rank==nParameters) {
			quality.setInfo("SVD decomposition rank complete");
		}
		else {
			quality.setInfo("SVD decomposition rank deficient");
		}
		gsl_vector_free(S);
		gsl_vector_free(work);
		gsl_matrix_free(V);
	}
	else { 
		quality.setInfo("Cholesky decomposition");
		gsl_linalg_cholesky_decomp(A);
		gsl_linalg_cholesky_solve(A, B, X);
	}
	
	// Update the parameters for the calculated changes
	map<string, uint>::iterator indit;
	for (indit=indices.begin();indit!=indices.end();indit++) {
		casa::Vector<double>& value(itsParams.value(indit->first));
		for (uint i=0;i<value.nelements();i++) {
			value(i)+=gsl_vector_get(X, indit->second+i);
		}
		itsParams.update(indit->first, value);
	}

	// Free up gsl storage
	gsl_vector_free(B);
	gsl_matrix_free(A);
	gsl_vector_free(X);

	return true;
};

// Fully general solver from the design matrix
bool LinearSolver::solveDesignMatrix(Quality& quality) {

	uint nParameters=0;
	uint nData=0;
	DMBVector::iterator bIt;
	for (bIt=itsDesignMatrix.residual().begin();bIt!=itsDesignMatrix.residual().end();bIt++) {
		nData+=bIt->nelements();
	}
	
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
	DMAMatrix::iterator AIt;
	// Outer loop is over the names of parameters. Each parameter
	// should have the same number of data points
	for (indit=indices.begin();indit!=indices.end();indit++) {
		// Axes are data, dof for each parameter
		// First find  the number of lists
		uint nA=itsDesignMatrix.derivative(indit->first).size();
		uint iA;
		uint outerRow=0;
		for (AIt=itsDesignMatrix.derivative(indit->first).begin(),iA=0;
			iA<nA;iA++,AIt++) {
			for (uint row=0;row<AIt->nrow();row++) {
				for (uint col=0;col<AIt->ncolumn();col++) {
					gsl_matrix_set(A, outerRow+2*row, col+indit->second, real((*AIt)(row,col)));
					gsl_matrix_set(A, outerRow+2*row+1, col+indit->second, imag((*AIt)(row,col)));
				}
			}
			outerRow+=2*AIt->nrow();
		}
	}
	
	// Call the gsl SVD function, A is overwritten by U
	gsl_matrix * V = gsl_matrix_alloc (nParameters, nParameters);
	gsl_vector * S = gsl_vector_alloc (nParameters);
	gsl_vector * work = gsl_vector_alloc (nParameters);
	gsl_linalg_SV_decomp (A, V, S, work);

	// Now find the solution for the residual vector
	gsl_vector * res = gsl_vector_alloc(2*nData);
	
	DMBVector::iterator BIt;
	for (BIt=itsDesignMatrix.residual().begin();
			BIt!=itsDesignMatrix.residual().end();BIt++) {
		uint outerRow=0;
		for (uint row=0;row<BIt->nelements();row++) {
			gsl_vector_set(res, outerRow++, real((*BIt)[row]));
			gsl_vector_set(res, outerRow++, imag((*BIt)[row]));
		}
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
	quality.setDOF(nParameters);
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
