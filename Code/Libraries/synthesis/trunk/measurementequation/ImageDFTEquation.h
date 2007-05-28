#ifndef SYNIMAGEDFTEQUATION_H_
#define SYNIMAGEDFTEQUATION_H_

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

class ImageDFTEquation : public conrad::scimath::Equation
{
public:

	ImageDFTEquation(const conrad::scimath::Params& ip, 
        IDataSharedIter& idi) :  
        conrad::scimath::Equation(ip), itsIdi(idi) {init();};
	
    ImageDFTEquation(const ImageDFTEquation& other);
    
    ImageDFTEquation& operator=(const ImageDFTEquation& other);
    
    virtual ~ImageDFTEquation() {};
    
	/// Predict model visibility
	virtual void predict();
	
    /// Calculate the normal equations
    /// @param ne Normal equations
    virtual void calcEquations(conrad::scimath::NormalEquations& ne);
	
private:
    IDataSharedIter itsIdi;
    
	void init();
    void calcVisDFT(const casa::Array<double>& imagePixels, 
        const double raStart, const double raEnd, const int raCells, 
        const double decStart, const double decEnd, const int decCells, 
        const casa::Vector<double>& freq, 
        const casa::Vector<casa::RigidVector<double, 3> >& uvw, 
        casa::Matrix<double>& vis, bool doderiv, casa::Matrix<double>& imageDeriv); 
};

}

}

#endif
