#ifndef COMPONENTMEUATION_H_
#define COMPONENTMEUATION_H_

#include "MEquation.h"

namespace conrad
{

class MEComponentEquation : public MEquation
{
public:
	MEComponentEquation() {};
	virtual ~MEComponentEquation();
	/// Predict model visibility
	/// @param ip Regular parameters
	/// @param iip Image parameters
	/// @param ida data accessor
	virtual void predict(const MEParams& ip, const MEImageParams& iip, MEDataAccessor& ida);
	
	/// Transpose back to parameter space
	/// @param ip Regular parameters
	/// @param iip Image parameters
	/// @param ida Data accessor
	/// @param is Solver
	/// @param iis Image solver
	virtual void transpose(MEParams& ip, const MEImageParams& iip, MEDataAccessor& ida,
		MESolver& is);
	virtual void transpose(const MEParams& ip, MEImageParams& iip, MEDataAccessor& ida, 
		MEImageSolver& iis) {};
	virtual void transpose(MEParams& ip, MEImageParams& iip, MEDataAccessor& ida, 
		MESolver& is, MEImageSolver& iis) {};
				
	/// Predict and then transpose back to parameter space
	/// @param ip Regular parameters
	/// @param iip Image parameters
	/// @param ida data accessor
	virtual void prediffer(MEParams& ip, const MEImageParams& iip, MEDataAccessor& ida,
		MESolver& is);
	virtual void prediffer(const MEParams& ip, MEImageParams& iip, MEDataAccessor& ida,
		MEImageSolver& iis) {};
	virtual void prediffer(MEParams& ip, MEImageParams& iip, MEDataAccessor& ida,
		MESolver& is, MEImageSolver& iis) {};
	
	
private:
	casa::Vector<casa::Double> calcDelay(double ra, double dec, 
		casa::Vector<casa::RigidVector<casa::Double, 3> > uvw);

};

}

#endif /*COMPONENTMEUATION_H_*/
