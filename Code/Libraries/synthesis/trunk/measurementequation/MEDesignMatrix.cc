#include <measurementequation/MEDesignMatrix.h>
#include <measurementequation/MEParams.h>
#include <casa/aips.h>
#include <casa/Utilities/Regex.h>
#include <casa/Exceptions/Error.h>

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
		itsDesignMatrix[*iter]=casa::Array<double>(casa::IPosition(0));
	}
	itsResiduals.resize(casa::IPosition(0));
	itsWeights.resize(casa::IPosition(0));
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
		itsResiduals=other.itsResiduals;
		itsWeights=other.itsWeights;
	}
}

MEDesignMatrix::~MEDesignMatrix()
{
	reset();
}

void MEDesignMatrix::merge(const MEDesignMatrix& other) 
{
}

void MEDesignMatrix::addDerivative(const string& name, const casa::Array<double>& deriv)
{
	// This should be append!
	itsDesignMatrix[name]=deriv;
}

void MEDesignMatrix::addResidual(const casa::Vector<double>& residual, const casa::Vector<double>& weights)
{
	// These should be appends!
	itsResiduals=residual;
	itsWeights=weights;
}

vector<string> MEDesignMatrix::names() const
{
	return itsParams.names();
}

void MEDesignMatrix::reset()
{
	std::map<std::string, casa::Array<double> >::iterator iter;
	for (iter=itsDesignMatrix.begin();iter!=itsDesignMatrix.end();++iter) {
		iter->second.resize(casa::IPosition(0));
	}
	itsDesignMatrix.clear();
	itsResiduals.resize(0);
	itsWeights.resize(0);
}

}
}