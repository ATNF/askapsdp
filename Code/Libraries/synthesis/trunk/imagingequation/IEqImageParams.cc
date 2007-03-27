#include "IEqImageParams.h"

#include <ostream>

namespace conrad
{

void IEqImageParams::add(const string& key, const IEqImageParam& param) {
	insert(make_pair(key, param));
}

void IEqImageParams::initDerivatives() {
  for (IEqImageParams::iterator iter=begin();iter!=end();iter++) {
  	(*iter).second.setDeriv(0.0);
  	(*iter).second.setDeriv2(0.0);
  }
}

void IEqImageParams::addDerivatives(const IEqImageParams& ip) {
  // This will throw an exception if the keys are inconsistent
	for (IEqImageParams::const_iterator iter=ip.begin();iter!=ip.end();iter++) {
		(*this)[(*iter).first].setDeriv((*iter).second.deriv());
		(*this)[(*iter).first].setDeriv2((*iter).second.deriv2());
	}
}
}
