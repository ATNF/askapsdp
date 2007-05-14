#ifndef SYNCOMPONENTEQUATION_H_
#define SYNCOMPONENTEQUATION_H_

#include <fitting/Equation.h>
#include <fitting/Params.h>
#include <dataaccess/IDataAccessor.h>

#include <boost/shared_ptr.hpp>

#include <casa/aips.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>

namespace conrad
{
namespace synthesis
{

class ComponentEquation : public conrad::scimath::Equation
{
public:

    /// @param ida data accessor
	ComponentEquation(const conrad::scimath::Params& ip,
        boost::shared_ptr<IDataAccessor>& ida) :  conrad::scimath::Equation(ip),
        itsIda(ida) {init();};
	
    ComponentEquation(const ComponentEquation& other);
    
    ComponentEquation& operator=(const ComponentEquation& other);
    
    virtual ~ComponentEquation() {};
    
	/// Predict model visibility
	virtual void predict();
	
	/// Calculate the normal equations
	/// @param ne Normal equations
	virtual void calcEquations(conrad::scimath::NormalEquations& ne);
	
private:
    boost::shared_ptr<IDataAccessor> itsIda;
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
