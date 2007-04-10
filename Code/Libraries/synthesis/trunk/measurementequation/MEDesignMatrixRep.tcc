#include <measurementequation/MEDesignMatrixRep.h>
#include <measurementequation/MEParamsRep.h>
#include <casa/aips.h>
#include <casa/Utilities/Regex.h>
#include <casa/Exceptions/Error.h>

namespace conrad
{
namespace synthesis
{

template<class T>
MEDesignMatrixRep<T>::MEDesignMatrixRep(const MEParamsRep<T>& ip)
{
	vector<string> names=ip.freeNames();
	vector<string>::iterator iter;
	uint index=0;
	itsIndices.clear();
	for (iter=names.begin();iter!=names.end();++iter) {
		itsIndices[*iter]=index++;
	}
	itsDataLength=0;
}

template<class T>
MEDesignMatrixRep<T>::MEDesignMatrixRep(const MEDesignMatrixRep& other) 
{
	operator=(other);
}

template<class T>
MEDesignMatrixRep<T>& MEDesignMatrixRep<T>::operator=(const MEDesignMatrixRep& other)
{
	if(this!=&other) {
		itsIndices=other.itsIndices;
		itsDataLength=other.itsDataLength;
		itsDesignMatrix=other.itsDesignMatrix;
	}
}

template<class T>
MEDesignMatrixRep<T>::~MEDesignMatrixRep()
{
	itsIndices.clear();
	itsDesignMatrix.resize(0,0);
	itsDataLength=0;
}

template<class T>
void MEDesignMatrixRep<T>::merge(const MEDesignMatrixRep& other) 
{
}

template<class T>
void MEDesignMatrixRep<T>::addDerivative(const string& name, const casa::Vector<T>& deriv)
{
	uint index;
	index=itsIndices[name];
	if(itsDataLength==0) {
		itsDataLength=deriv.nelements();
		itsDesignMatrix.resize(itsIndices.size(), itsDataLength);
	}
	if(itsDataLength!=deriv.nelements()) {
		throw(casa::IndexError("Data size incompatible with shape of design matrix"));
	}
	itsDesignMatrix.column(index)=deriv;
}


template<class T>
void MEDesignMatrixRep<T>::reset()
{
	itsIndices.clear();
	itsDataLength=0;
	itsDesignMatrix.resize(0,0);
}

}
}