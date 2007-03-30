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
	
	/// Predict and then transpose back to parameter space
	/// @param ip Regular parameters
	/// @param iip Image parameters
	/// @param ida data accessor
	virtual void calcDerivatives(MEParams& ip, MEImageParams& iip, MEDataAccessor& ida,
		MESolver& is);
	
	
private:
	casa::Vector<casa::Double> calcDelay(double ra, double dec, 
		casa::Vector<casa::RigidVector<casa::Double, 3> > uvw);

};

}

#endif /*COMPONENTMEUATION_H_*/
