#ifndef SYNIMAGEFFTEQUATION_H_
#define SYNIMAGEFFTEQUATION_H_

#include <fitting/Params.h>
#include <fitting/Equation.h>

#include <dataaccess/SharedIter.h>
#include <dataaccess/IDataIterator.h>

#include <casa/aips.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>

namespace conrad
{
namespace synthesis
{

class ImageFFTEquation : public conrad::scimath::Equation
{
public:

	ImageFFTEquation(const conrad::scimath::Params& ip, 
        IDataSharedIter& idi) :  
        conrad::scimath::Equation(ip), itsIdi(idi) {init();};
	
    ImageFFTEquation(const ImageFFTEquation& other);
    
    ImageFFTEquation& operator=(const ImageFFTEquation& other);
    
    virtual ~ImageFFTEquation() {};
    
	/// Predict model visibility
	virtual void predict();
	
    /// Calculate the normal equations
    /// @param ne Normal equations
    virtual void calcEquations(conrad::scimath::NormalEquations& ne);
	
private:
    IDataSharedIter itsIdi;
    
	void init();
};

}

}

#endif
