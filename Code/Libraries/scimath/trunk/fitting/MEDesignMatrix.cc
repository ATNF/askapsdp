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
#include <algorithm>
using std::string;
using std::vector;
using std::map;

namespace conrad
{
namespace synthesis
{

MEDesignMatrix::MEDesignMatrix(const MEParams& ip) : itsParams(ip)
{
	itsAMatrix.clear();
	itsBVector.clear();
	itsWeight.clear();
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
	if(itsAMatrix.size()==0) {
		itsParams=other.itsParams;
		itsAMatrix=other.itsAMatrix;
		itsBVector=other.itsBVector;
		itsWeight=other.itsWeight;
	}
	else {
		std::map<string, DMAMatrix>::iterator AIt, OtherAIt;
		for (AIt=itsAMatrix.begin();AIt!=itsAMatrix.end();AIt++) {
			std::copy(other.itsAMatrix[AIt->first].begin(), other.itsAMatrix[AIt->first].end(), 
				itsAMatrix[AIt->first].end());
		}
	
		std::copy(other.itsBVector.begin(), other.itsBVector.end(), itsBVector.end());
		std::copy(other.itsWeight.begin(), other.itsWeight.end(), itsWeight.end());
	}
}

void MEDesignMatrix::addDerivative(const string& name, const casa::Matrix<casa::DComplex>& deriv)
{
	if(!itsParams.has(name)) {
		throw(std::invalid_argument("Parameter "+name+" does not exist in the declared parameters"));
	}
	itsAMatrix[name].push_back(deriv.copy());
}

void MEDesignMatrix::addResidual(const casa::Vector<casa::DComplex>& residual, const casa::Vector<double>& weight)
{
	itsBVector.push_back(residual.copy());
	itsWeight.push_back(weight.copy());
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

DMAMatrix MEDesignMatrix::derivative(const string& name) const
{
	if(!itsParams.has(name)) {
		throw(std::invalid_argument("Parameter "+name+" does not exist in the declared parameters"));
	}
	if(itsAMatrix.count(name)==0) {
		throw(std::invalid_argument("Parameter "+name+" does not exist in the assigned values"));
	}
	return itsAMatrix[name];
}

DMBVector MEDesignMatrix::residual() const
{
	return itsBVector;
}

DMWeight MEDesignMatrix::weight() const
{
	return itsWeight;
}

void MEDesignMatrix::reset()
{
	itsAMatrix.clear();
	itsBVector.clear();
	itsWeight.clear();
}

double MEDesignMatrix::fit() const
{
	double sumwt=0.0;
	double sum=0.0;
	DMBVector::iterator bIt;
	DMWeight::iterator wIt;
	for (bIt=itsBVector.begin(),wIt=itsWeight.begin();
		(bIt!=itsBVector.end())&&(wIt!=itsWeight.end());bIt++, wIt++) {
		sumwt+=casa::sum(*wIt);
		sum+=casa::sum((*wIt)*real((*bIt)*conj(*bIt)));
	}
	if(sumwt>0.0) {
		return sqrt(sum/sumwt);
	}
	else {
		throw(std::invalid_argument("Sum of weights is zero"));
	}
}

uint MEDesignMatrix::nData() const
{
	uint nData=0;
	std::map<string, DMAMatrix>::iterator AIt;
	for (AIt=itsAMatrix.begin();AIt!=itsAMatrix.end();AIt++) {
		DMAMatrix::iterator it;
		for (it=AIt->second.begin();it!=AIt->second.end();it++) {
			nData+=it->nrow();
		}
	}
	return nData;
}

uint MEDesignMatrix::nParameters() const
{
	uint nParameters=0;
	std::map<string, DMAMatrix>::iterator AIt;
	for (AIt=itsAMatrix.begin();AIt!=itsAMatrix.end();AIt++) {
		DMAMatrix::iterator it;
		for (it=AIt->second.begin();it!=AIt->second.end();it++) {
			nParameters+=it->ncolumn();
		}
	}
	return nParameters;
}

}
}