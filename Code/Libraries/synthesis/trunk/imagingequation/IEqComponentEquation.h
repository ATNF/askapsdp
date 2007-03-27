#ifndef COMPONENTIEQUATION_H_
#define COMPONENTIEQUATION_H_

#include "IEquation.h"

namespace conrad
{

class IEqComponentEquation : public conrad::IEquation
{
public:
	IEqComponentEquation() {};
	virtual ~IEqComponentEquation();
	/// Predict model visibility
	/// @param ip Regular parameters
	/// @param iip Image parameters
	/// @param ida data accessor
	virtual void predict(const IEqParams& ip, const IEqImageParams& iip, IEqDataAccessor& ida);
	
	/// Transpose back to parameter space
	/// @param ip Regular parameters
	/// @param iip Image parameters
	/// @param ida data accessor
	virtual void transpose(IEqParams& ip, const IEqImageParams& iip, IEqDataAccessor& ida);
	virtual void transposeImage(const IEqParams& ip, IEqImageParams& iip, IEqDataAccessor& ida);
		
	/// Predict and then transpose back to parameter space
	/// @param ip Regular parameters
	/// @param iip Image parameters
	/// @param ida data accessor
	virtual void prediffer(IEqParams& ip, const IEqImageParams& iip, IEqDataAccessor& ida);
	virtual void predifferImage(const IEqParams& ip, IEqImageParams& iip, IEqDataAccessor& ida);
	
private:
	casa::Vector<casa::Double> calcDelay(double ra, double dec, 
		casa::Vector<casa::RigidVector<casa::Double, 3> > uvw);

};

}

#endif /*COMPONENTIEQUATION_H_*/
