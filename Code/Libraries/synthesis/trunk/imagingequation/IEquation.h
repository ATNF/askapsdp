#ifndef IEQUATION_H
#define IEQUATION_H

#include <casa/aips.h>

namespace conrad { 

class IEqDataAccessor;
class IEqParams;

class IEquation {
public:

	enum Policy {
		CAUTIOUS=0
	};
	
	enum Optimization {
		MEMORY=0,
		IO=1,
		CPU=2
	};
	
	enum Context {
		HIGHDYNAMICRANGE=0,
		LOWDYNAMICRANGE=1,		
	};

	/// Constructor
	/// @param name Name of equation
	/// @param ip Equation parameters
    IEquation(const casa::String& name, IEqParams& ip) : itsName(name), itsParams(ip) {};
    
    /// Overwrite the parameters
    /// @param ip New parameters
    void setParameters(IEqParams& ip) {itsParams=ip;};
    
    /// Return the parameters
    IEqParams& parameters() {return itsParams;}; 

	/// Predict model visibility
	/// @param ida data accessor
	virtual void predict(IEqDataAccessor& ida) {
		cout << "predict in " << itsName << endl;
	}
	/// Transpose back to parameter space
	/// @param ida data accessor
	virtual IEqParams& transpose(IEqDataAccessor& ida) {
		cout << "transpose in " << itsName << endl;
		return itsParams;
	}
	/// Predict and then transpose back to parameter space
	/// @param ida data accessor
	/// @param ip imaging params
	virtual IEqParams& prediffer(IEqDataAccessor& ida) {
		cout << "prediffer in " << itsName << endl;
		return itsParams;
	}
	virtual ~IEquation() {};
		
 protected:
 private:
 	casa::String itsName;
 	IEqParams itsParams;
};

}

#endif





