#include <measurementequation/MEDesignMatrix.h>
#include <measurementequation/MEParams.h>
#include <casa/aips.h>
#include <casa/Utilities/Regex.h>

#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/Arrays/ArrayMath.h>

#include <stdexcept>
#include <iostream>
#include <string>
#include <vector>
#include <map>
using std::string;
using std::vector;
using std::map;

namespace conrad
{
namespace synthesis
{

MEDesignMatrix::MEDesignMatrix(const MEParams& ip) : itsParams(ip)
{
	vector<string> names=itsParams.freeNames();
	vector<string>::iterator iter;
	for (iter=names.begin();iter!=names.end();++iter) {
		itsAMatrix[*iter]=casa::Matrix<casa::Complex>(0,0);
	}
	itsBVector.resize(0);
	itsWeight.resize(0);
}

MEDesignMatrix::MEDesignMatrix(const MEDesignMatrix& other) 
{
	operator=(other);
}

MEDesignMatrix& MEDesignMatrix::operator=(const MEDesignMatrix& other)
{
	if(this!=&other) {
		itsParams=other.itsParams;
		itsAMatrix=other.itsAMatrix;
		itsBVector=other.itsBVector;
		itsWeight=other.itsWeight;
	}
}

MEDesignMatrix::~MEDesignMatrix()
{
	reset();
}

void MEDesignMatrix::merge(const MEDesignMatrix& other) 
{
}

void MEDesignMatrix::addDerivative(const string& name, const casa::Matrix<casa::Complex>& deriv)
{
	// This should be append!
	if(!itsParams.has(name)) {
		throw(std::invalid_argument("Parameter "+name+" does not exist in the declared parameters"));
	}
	itsAMatrix[name]=deriv.copy();
}

void MEDesignMatrix::addResidual(const casa::Vector<casa::Complex>& residual, const casa::Vector<double>& weight)
{
	// These should be appends!
	itsBVector=residual.copy();
	itsWeight=weight.copy();
}

vector<string> MEDesignMatrix::names() const
{
	return itsParams.names();
}

const MEParams& MEDesignMatrix::parameters() const
{
	return itsParams;
}

MEParams& MEDesignMatrix::parameters() 
{
	return itsParams;
}

const std::map<string, casa::Matrix<casa::Complex> >& MEDesignMatrix::designMatrix() const
{
	return itsAMatrix;
}

const casa::Matrix<casa::Complex>& MEDesignMatrix::derivative(const string& name) const
{
	if(!itsParams.has(name)) {
		throw(std::invalid_argument("Parameter "+name+" does not exist in the declared parameters"));
	}
	if(itsAMatrix.count(name)==0) {
		throw(std::invalid_argument("Parameter "+name+" does not exist in the assigned values"));
	}
	return itsAMatrix[name];
}

const casa::Vector<casa::Complex>& MEDesignMatrix::residual() const
{
	return itsBVector;
}

const casa::Vector<double>& MEDesignMatrix::weight() const
{
	return itsWeight;
}

void MEDesignMatrix::reset()
{
	std::map<std::string, casa::Matrix<casa::Complex> >::iterator iter;
	for (iter=itsAMatrix.begin();iter!=itsAMatrix.end();++iter) {
		iter->second.resize(0,0);
	}
	itsAMatrix.clear();
	itsBVector.resize(0);
	itsWeight.resize(0);
}

double MEDesignMatrix::fit() const
{
	double sumwt=casa::sum(itsWeight);
	if(sumwt>0.0) {
		return sqrt(casa::sum(real(itsBVector*conj(itsBVector)))/sumwt);
	}
	else {
		throw(std::invalid_argument("Sum of weights is zero"));
	}
}

}
}