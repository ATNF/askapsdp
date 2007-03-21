/// @file
///
/// IEquation: Represent a parametrized imaging equation. An IEquation 
/// is constructed with a name and a set of parameters. The parameters
/// can be updated subsequently. The IEquation can do two principal things
///    - calculate data (passed via a data accessor)
///    - transpose residual data back to the parameter space
/// These two things can be combined in a prediffer step to allow calculation
/// of gradients for parameters. The parameters may then be solved for by
/// an IEqSolver class.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef IEQUATION_H
#define IEQUATION_H

#include <casa/aips.h>

namespace conrad { 

class IEqDataAccessor;
class IEqParams;

class IEquation {
public:

	/// Not sure that these enum's are useful
	/// List of mutually exclusive policies
	enum Policy {
		CAUTIOUS=0
	};

	/// List of mutually exclusive optimizations	
	enum Optimization {
		MEMORY=0,
		IO=1,
		CPU=2
	};
	
	/// List of mutually exclusive contexts
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
    void setParameters(const IEqParams& ip) {itsParams=ip;};
    
    /// Return the parameters
    const IEqParams& parameters() const {return itsParams;}; 

	/// Predict model visibility
	/// @param ida data accessor
	virtual void predict(IEqDataAccessor& ida) {
		cout << "Stubbed predict in " << itsName << endl;
	}
	
	/// Transpose back to parameter space
	/// @param ida data accessor
	virtual IEqParams& transpose(IEqDataAccessor& ida) {
		cout << "Stubbed transpose in " << itsName << endl;
		return itsParams;
	}
	
	/// Predict and then transpose back to parameter space
	/// @param ida data accessor
	/// @param ip imaging params
	virtual IEqParams& prediffer(IEqDataAccessor& ida) {
		cout << "Stubbed prediffer in " << itsName << endl;
		return itsParams;
	}
	virtual ~IEquation() {};
		
 protected:
 	casa::String itsName;
 	IEqParams itsParams;
};

}

#endif





