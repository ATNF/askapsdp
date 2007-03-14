#ifndef IEQPARAMS_H_
#define IEQPARAMS_H_

#include <casa/aips.h>
#include <casa/BasicSL/String.h>
#include <casa/Arrays/Vector.h>
#include <tables/Tables/Table.h>

namespace conrad {

class IEqParam;

class IEqParams {
public:
	// Create from a table
	IEqParams(casa::Table& parmtable);
	
	// Empty constructor
	IEqParams();
	
	// Return names of parameters
	casa::Vector<casa::String> names();
	
	// Add an ImagingParam
	void add(IEqParam& ip);
	
	// Does this contain the named param?
	bool contains(const casa::String& name) const;
	
	// Get the named parameter
	IEqParam operator()(const casa::String& name);
	
	// Store as a table
	void storeAsTable(casa::Table& parmtable);
	
	virtual ~IEqParams();
};

}

#endif /*IMAGINGPARAMS_H_*/
