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
	virtual void calcEquations(IDataAccessor& ida,
		MENormalEquations& normeq);
		
	/// Calculate the design matrix and normal equations
	/// @param ida data accessor
	/// @param design matrix
	/// @param normeq Normal equations
	virtual void calcEquations(IDataAccessor& ida,
		MEDesignMatrix& designmatrix, MENormalEquations& normeq);

	/// Calculate the design matrix
	/// @param ida data accessor
	/// @param design matrix
	virtual void calcEquations(IDataAccessor& ida,
		MEDesignMatrix& designmatrix);
	
private:
	void init();
	/// Templated function to do the calculation of value and derivatives.
	template<class T>
	void calcVis(const T& ra, const T& dec, const T& flux, 
		const casa::Vector<double>& freq, const double u, const double v, 
		casa::Vector<T>& vreal, casa::Vector<T>& vimag);

};

}

}

#endif
