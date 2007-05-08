#ifndef SYNCOMPONENTEQUATION_H_
#define SYNCOMPONENTEQUATION_H_

#include <measurementequation/SynEquation.h>

namespace conrad
{
namespace synthesis
{

class ComponentEquation : public SynEquation
{
public:

	ComponentEquation(const conrad::scimath::Params& ip) : SynEquation(ip) {};
	
	/// Predict model visibility
	/// @param ida data accessor
	virtual void predict(IDataAccessor& ida);
	
	/// Calculate the normal equations
	/// @param ida data accessor
	/// @param normeq Normal equations
	virtual void calcEquations(IDataAccessor& ida, conrad::scimath::NormalEquations& normeq);
	
	/// Calculate the regular design matrix
	/// @param ida data accessor
	/// @param design matrix
	virtual void calcEquations(IDataAccessor& ida, conrad::scimath::DesignMatrix& designmatrix);
	
private:
	void init();
	/// Templated function to do the calculation of value and derivatives.
	template<class T>
	void calcRegularVis(const T& ra, const T& dec, const T& flux, 
		const casa::Vector<double>& freq, const double u, const double v, 
		casa::Vector<T>& vis);
};

}

}

#endif
