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

MEDesignMatrix::MEDesignMatrix(const MEParams& ip)
{
	vector<string> names=ip.freeNames();
	vector<string>::iterator iter;
	for (iter=names.begin();iter!=names.end();++iter) {
		itsDesignMatrix[*iter]=casa::Vector<double>(0);
	}
	itsDataLength=0;
}

MEDesignMatrix::MEDesignMatrix(const MEDesignMatrix& other) 
{
	operator=(other);
}

MEDesignMatrix& MEDesignMatrix::operator=(const MEDesignMatrix& other)
{
	if(this!=&other) {
		itsDataLength=other.itsDataLength;
		itsDesignMatrix=other.itsDesignMatrix;
	}
}

MEDesignMatrix::~MEDesignMatrix()
{
	reset();
}

void MEDesignMatrix::merge(const MEDesignMatrix& other) 
{
}

void MEDesignMatrix::addDerivative(const string& name, const casa::Vector<double>& deriv)
{
	if(itsDataLength==0) {
		itsDataLength=deriv.nelements();
	}
	if(itsDataLength!=deriv.nelements()) {
		throw(casa::IndexError("Data size incompatible with shape of design matrix"));
	}
	itsDesignMatrix[name]=deriv;
}


void MEDesignMatrix::reset()
{
	std::map<std::string, casa::Vector<double> >::iterator iter;
	for (iter=itsDesignMatrix.begin();iter!=itsDesignMatrix.end();++iter) {
		iter->second.resize(0);
	}
	itsDesignMatrix.clear();
	itsDataLength=0;
}

}
}