#include <measurementequation/MEDesignMatrixRep.h>
#include <measurementequation/MEParamsRep.h>
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

MEDesignMatrixRep<double>::MEDesignMatrixRep(const MEParamsRep<double>& ip)
{
	vector<string> names=ip.freeNames();
	vector<string>::iterator iter;
	for (iter=names.begin();iter!=names.end();++iter) {
		itsDesignMatrix[*iter]=casa::Vector<double>(0);
	}
	itsDataLength=0;
}

MEDesignMatrixRep<double>::MEDesignMatrixRep(const MEDesignMatrixRep& other) 
{
	operator=(other);
}

MEDesignMatrixRep<double>& MEDesignMatrixRep<double>::operator=(const MEDesignMatrixRep& other)
{
	if(this!=&other) {
		itsDataLength=other.itsDataLength;
		itsDesignMatrix=other.itsDesignMatrix;
	}
}

MEDesignMatrixRep<double>::~MEDesignMatrixRep()
{
	reset();
}

void MEDesignMatrixRep<double>::merge(const MEDesignMatrixRep& other) 
{
}

void MEDesignMatrixRep<double>::addDerivative(const string& name, const casa::Vector<double>& deriv)
{
	if(itsDataLength==0) {
		itsDataLength=deriv.nelements();
	}
	if(itsDataLength!=deriv.nelements()) {
		throw(casa::IndexError("Data size incompatible with shape of design matrix"));
	}
	itsDesignMatrix[name]=deriv;
}


void MEDesignMatrixRep<double>::reset()
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