#ifndef MECOMPONENTEQUATION_H_
#define MECOMPONENTEQUATION_H_

#include <measurementequation/MEquation.h>

namespace conrad
{
namespace synthesis
{

class MEComponentEquation : public MEquation
{
public:

	MEComponentEquation() : MEquation() {};
	MEComponentEquation(const MEParams& ip) : MEquation(ip) {};
	
	virtual ~MEComponentEquation();
	/// Predict model visibility
	/// @param ida data accessor
	virtual void predict(IDataAccessor& ida);
	
	/// Calculate the normal equations
	/// @param ida data accessor
	/// @param normeq Normal equations
	virtual void calcNormalEquations(IDataAccessor& ida,
		MENormalEquations& normeq);
		
	/// Calculate the design matrix
	/// @param ip Regular parameters
	/// @param ida data accessor
	/// @param design matrix
	virtual void calcDesignMatrix(IDataAccessor& ida,
		MEDesignMatrix& designmatrix);
	
private:
	/// Templated function to do the calculation of value and derivatives.
	template<class T>
	void calc(const IDataAccessor& ida, casa::Vector<T>& vreal, casa::Vector<T>& vimag);

};

}

}

#endif
