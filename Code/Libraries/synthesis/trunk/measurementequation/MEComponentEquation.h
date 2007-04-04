#ifndef COMPONENTMEQUATION_H_
#define COMPONENTMEQUATION_H_

#include <measurementequation/MEquation.h>

namespace conrad
{

class MEComponentEquation : public MEquation
{
public:

	MEComponentEquation() {};
	virtual ~MEComponentEquation();
	/// Predict model visibility
	/// @param ip Regular parameters
	/// @param ida data accessor
	virtual void predict(const MEParams& ip, MEDataAccessor& ida);
	
	/// Calculate the normal equations
	/// @param ip Regular parameters
	/// @param ida data accessor
	/// @param normeq Normal equations
	virtual void calcNormalEquations(MEParams& ip, MEDataAccessor& ida,
		MENormalEquations& normeq);
	
private:
	/// Templated function to do the calculation of value and derivatives.
	template<class T>
	void calc(const MEDataAccessor& ida, const MEParams& ip, 
		casa::Vector<T>& vreal, casa::Vector<T>& vimag);

};

}

#endif
