#include "IEqParams.h"

#include <ostream>

namespace conrad
{

IEqParams::IEqParams(const string& parmtable) {
}

// Store as a table
void IEqParams::saveAsTable(const string& parmtable) const {
}

void IEqParams::add(const string& key) {
	IEqParam param;
	insert(make_pair(key, param));
}

void IEqParams::add(const string& key, const IEqParam& param) {
	insert(make_pair(key, param));
}

void IEqParams::initDerivatives() {
  for (IEqParams::iterator iter=begin();iter!=end();iter++) {
  	(*iter).second.setDeriv(0.0);
  	(*iter).second.setDeriv2(0.0);
  }
}

void IEqParams::addDerivatives(const IEqParams& ip) {
  // This will throw an exception if the keys are inconsistent
	for (IEqParams::const_iterator iter=ip.begin();iter!=ip.end();iter++) {
		(*this)[(*iter).first].setDeriv((*iter).second.deriv());
		(*this)[(*iter).first].setDeriv2((*iter).second.deriv2());
	}
}
}
