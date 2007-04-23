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
		itsDesignMatrix[*iter]=casa::Matrix<double>(0,0);
	}
	itsResidual.resize(0);
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
		itsDesignMatrix=other.itsDesignMatrix;
		itsResidual=other.itsResidual;
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

void MEDesignMatrix::addDerivative(const string& name, const casa::Matrix<double>& deriv)
{
	// This should be append!
	if(!itsParams.has(name)) {
		throw(std::invalid_argument("Parameter "+name+" does not exist in the declared parameters"));
	}
	itsDesignMatrix[name]=deriv.copy();
}

void MEDesignMatrix::addResidual(const casa::Vector<double>& residual, const casa::Vector<double>& weight)
{
	// These should be appends!
	itsResidual=residual.copy();
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

const std::map<string, casa::Matrix<double> >& MEDesignMatrix::designMatrix() const
{
	return itsDesignMatrix;
}

const casa::Matrix<double>& MEDesignMatrix::derivative(const string& name) const
{
	if(!itsParams.has(name)) {
		throw(std::invalid_argument("Parameter "+name+" does not exist in the declared parameters"));
	}
	if(itsDesignMatrix.count(name)==0) {
		throw(std::invalid_argument("Parameter "+name+" does not exist in the assigned values"));
	}
	return itsDesignMatrix[name];
}

const casa::Vector<double>& MEDesignMatrix::residual() const
{
	return itsResidual;
}

const casa::Vector<double>& MEDesignMatrix::weight() const
{
	return itsWeight;
}

void MEDesignMatrix::reset()
{
	std::map<std::string, casa::Matrix<double> >::iterator iter;
	for (iter=itsDesignMatrix.begin();iter!=itsDesignMatrix.end();++iter) {
		iter->second.resize(0,0);
	}
	itsDesignMatrix.clear();
	itsResidual.resize(0);
	itsWeight.resize(0);
}

double MEDesignMatrix::fit() const
{
	double sumwt=casa::sum(itsWeight);
	if(sumwt>0.0) {
		return sqrt(casa::sum(itsResidual*itsResidual*itsWeight)/sumwt);
	}
	else {
		throw(std::invalid_argument("Sum of weights is zero"));
	}
}

}
}