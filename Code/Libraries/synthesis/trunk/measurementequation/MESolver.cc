#include <measurementequation/MESolver.h>

namespace conrad
{

MESolver::MESolver(const MEParams& ip) : itsParams(ip) {
};

void MESolver::setParameters(const MEParams& ip) {
	itsParams=ip;
}
/// Return current values of params
const MEParams& MESolver::getParameters() const {
	return itsParams;
};
	

}
