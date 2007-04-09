#include <measurementequation/MEDesignMatrixRep.h>
#include <measurementequation/MEParamsRep.h>

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
		itsDesignMatrix=other.itsDesignMatrix;
	}
}

template<class T>
MEDesignMatrixRep<T>::~MEDesignMatrixRep()
{
	itsIndices.clear();
	itsDesignMatrix.resize(0,0);
}

template<class T>
void MEDesignMatrixRep<T>::merge(const MEDesignMatrixRep& other) 
{
}

template<class T>
void MEDesignMatrixRep<T>::reset()
{
	itsIndices.clear();
	itsDesignMatrix.resize(0,0);
}

}
}