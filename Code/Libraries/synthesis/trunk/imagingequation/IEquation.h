#ifndef IEQUATION_H
#define IEQUATION_H

#include <casa/aips.h>

// forwards

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

  IEquation();
  
  // Destructor
  virtual ~IEquation()=0;

  // Predict for a given set of ImagingParams
  virtual void predict(IEqDataAccessor& ida, const IEqParams& ip)=0;
  
  // Transpose
  virtual IEqParams& transpose(IEqDataAccessor& ida, const IEqParams& ip)=0;

  // Predict and differentiate
  virtual IEqParams& prediffer(IEqDataAccessor& ida, const IEqParams& ip)=0;

 protected:
};

class StubbedIEquation : public IEquation {
	public:
		virtual ~StubbedIEquation() {};
		StubbedIEquation(casa::String name) {itsName=name;};
		virtual void predict(IEqDataAccessor& ida, IEqParams& ip) {
			cout << "predict in " << itsName << endl;
		}
		virtual IEqParams& transpose(IEqDataAccessor& ida, IEqParams& ip) {
			cout << "transpose in " << itsName << endl;
			return ip;
		}
		virtual IEqParams& prediffer(IEqDataAccessor& ida, IEqParams& ip) {
			cout << "prediffer in " << itsName << endl;
			return ip;
		}
	protected:
		casa::String itsName;
};


}

#endif





