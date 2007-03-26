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

#include "IEqParams.h"
#include "IEqImageParams.h"
#include "IEqDataAccessor.h"

//class IEqDataAccessor;

namespace conrad { 

class IEquation {
public:

	/// Constructor
	/// @param name Name of equation
	/// @param ip Equation parameters
    /// IEquation(IEqParams& ip);
    
    /// Overwrite the parameters
    /// @param ip New parameters
    void setParameters(const IEqParams& ip, const IEqImageParams& iip) {
    	itsParams=ip;
    	itsImageParams=iip;
	};
    
    void setParameters(const IEqParams& ip) {
    	itsParams=ip;
	};
    
    void setParameters(const IEqImageParams& iip) {
    	itsImageParams=iip;
	};
    
    /// Return the parameters
    const IEqParams& parameters() const {return itsParams;}; 
    const IEqImageParams& imageParameters() const {return itsImageParams;}; 

	/// Predict model visibility
	/// @param ida data accessor
	virtual void predict(IEqDataAccessor& ida) =0;
	
	/// Transpose back to parameter space
	/// @param ida data accessor
	virtual IEqParams& transpose(IEqDataAccessor& ida) = 0;
	virtual IEqImageParams& transposeImage(IEqDataAccessor& ida) =0;
		
	/// Predict and then transpose back to parameter space
	/// @param ida data accessor
	/// @param ip imaging params
	virtual IEqParams& prediffer(IEqDataAccessor& ida) =0;
	virtual IEqImageParams& predifferImage(IEqDataAccessor& ida) = 0;
	
			
 protected:
 	IEqParams itsParams;
	IEqImageParams itsImageParams;
};

}

#endif





