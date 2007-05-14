#ifndef SYNIMAGEEQUATION_H_
#define SYNIMAGEEQUATION_H_

#include <fitting/Params.h>
#include <fitting/Equation.h>
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

class ImageEquation : public conrad::scimath::Equation
{
public:

	ImageEquation(const conrad::scimath::Params& ip, 
        boost::shared_ptr<IDataAccessor>& ida) :  conrad::scimath::Equation(ip),
        itsIda(ida) {init();};
	
    ImageEquation(const ImageEquation& other);
    
    ImageEquation& operator=(const ImageEquation& other);
    
    virtual ~ImageEquation() {};
    
	/// Predict model visibility
	virtual void predict();
	
    /// Calculate the normal equations
    /// @param ne Normal equations
    virtual void calcEquations(conrad::scimath::NormalEquations& ne);
	
private:
    boost::shared_ptr<IDataAccessor> itsIda;
	void init();
    void calcVis(const casa::Vector<double>& imagePixels, 
		const double raStart, const double raEnd, const int raCells, 
		const double decStart, const double decEnd, const int decCells, 
		const casa::Vector<double>& freq, 
		const casa::Vector<casa::RigidVector<double, 3> >& uvw, 
		casa::Matrix<double>& vis, bool doderiv, casa::Matrix<double>& imageDeriv); 
};

}

}

#endif
