#ifndef COMPONENTIEQUATION_H_
#define COMPONENTIEQUATION_H_

#include "IEquation.h"

namespace conrad
{

class IEqComponentEquation : public conrad::IEquation
{
public:
	IEqComponentEquation(IEqParams& ip);
	virtual ~IEqComponentEquation();
	virtual void predict(IEqDataAccessor& ida);

	/// Transpose back to parameter space
	/// @param ida data accessor
	virtual IEqParams& transpose(IEqDataAccessor& ida);
	virtual IEqImageParams& transposeImage(IEqDataAccessor& ida);
	
	/// Predict and then transpose back to parameter space
	/// @param ida data accessor
	/// @param ip imaging params
	virtual IEqParams& prediffer(IEqDataAccessor& ida);
	virtual IEqImageParams& predifferImage(IEqDataAccessor& ida);
	
	
private:
	casa::Vector<casa::Double> calcDelay(double ra, double dec, 
		casa::Vector<casa::RigidVector<casa::Double, 3> > uvw);

};

}

#endif /*COMPONENTIEQUATION_H_*/
