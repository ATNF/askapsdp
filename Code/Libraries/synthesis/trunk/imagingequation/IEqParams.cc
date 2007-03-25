#include "IEqParams.h"

namespace conrad
{

IEqParams::IEqParams(const string& parmtable) {
}

// Store as a table
void IEqParams::saveAsTable(const string& parmtable) const {
}

void IEqParams::add(const string& key, const IEqParam& param) {
	insert(make_pair(key, param));
}

void IEqParams::initDerivatives() {
  for (IEqParams::iterator iter=begin();iter!=end();iter++) {
  	(*iter).second.deriv()=0.0;
  	(*iter).second.deriv2()=0.0;
  }

}

void IEqParams::addDerivatives(IEqParams& ip) {
  // This will throw an exception if the keys are inconsistent
  for (IEqParams::iterator iter=begin();iter!=end();iter++) {
  	(*iter).second.deriv()=ip[(*iter).first].deriv();
  	(*iter).second.deriv()=ip[(*iter).first].deriv2();
  }
}

}