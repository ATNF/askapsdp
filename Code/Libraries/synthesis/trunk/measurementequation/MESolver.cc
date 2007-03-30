#include "MESolver.h"

namespace conrad
{

MESolver::MESolver(const MEParams& ip, const MEImageParams& iip) {
	itsParams=ip;
	itsImageParams=iip;
};
void MESolver::addDerivatives(MEParams& ip) {
	itsParams.addDerivatives(ip);
}
void MESolver::addDerivatives(MEImageParams& iip) {
	itsImageParams.addDerivatives(iip);
}
void MESolver::init() {
	itsParams.initDerivatives();
	itsImageParams.initDerivatives();
}
bool MESolver::solve() {
	for (MEParams::iterator iter=itsParams.begin();iter!=itsParams.end();iter++) {
		if((*iter).second.isFree()) {
			double delta, value;
			value=(*iter).second.value();
			delta=(*iter).second.deriv()/(*iter).second.deriv2();
			(*iter).second.setValue(value+gain()*delta);
		}
	}
	return true;
};
bool MESolver::solveImage() {
//		for (MEImageParams::iterator iter=itsImageParams.begin();iter!=itsImageParams.end();iter++) {
//			if((*iter).second.isFree()) {
//				casa::LatticeExprNode delta, value;
//				value=(*iter).second.value();
//				delta=(*iter).second.deriv()/(*iter).second.deriv2();
//				casa::LatticeExprNode newvalue(value+gain()*iif((*iter).second.deriv2() > 0.0, 
//					(*iter).second.deriv()/(*iter).second.deriv2(), 0.0));     
//				(*iter).second.setValue(newvalue);
//			}
//		}
	return true;
};

}
