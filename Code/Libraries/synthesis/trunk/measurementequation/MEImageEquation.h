#ifndef MEIMAGEEQUATION_H_
#define MEIMAGEEQUATION_H_

#include <measurementequation/MEquation.h>
#include <dataaccess/IDataAccessor.h>

#include <casa/aips.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>

namespace conrad
{
namespace synthesis
{

class MEImageEquation : public MEquation
{
public:

	MEImageEquation() : MEquation() {};
	MEImageEquation(const MEParams& ip) : MEquation(ip) {};
	
	virtual ~MEImageEquation();
	
	/// Predict model visibility
	/// @param ida data accessor
	virtual void predict(IDataAccessor& ida);
	
	/// Calculate the normal equations
	/// @param ida data accessor
	/// @param normeq Normal equations
	virtual void calcEquations(IDataAccessor& ida, MENormalEquations& normeq);
	
	/// Calculate the regular design matrix
	/// @param ida data accessor
	/// @param design matrix
	virtual void calcEquations(IDataAccessor& ida, MEDesignMatrix& designmatrix);
	
private:
	void init();
    void calcVis(const casa::Vector<double>& imagePixels, 
		const double raStart, const double raEnd, const int raCells, 
		const double decStart, const double decEnd, const int decCells, 
		const casa::Vector<double>& freq, 
		const casa::Vector<casa::RigidVector<double, 3> >& uvw, 
		casa::Matrix<casa::Complex>& vis, bool doderiv, casa::Matrix<casa::Complex>& imageDeriv); 
};

}

}

#endif
