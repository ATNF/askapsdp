#include <measurementequation/MEImageSolver.h>

namespace conrad
{
namespace synthesis
{

MEImageSolver::MEImageSolver(const MEImageParams& ip) : itsParams(ip), itsNormalEquations(ip) {
};

void MEImageSolver::setParameters(const MEImageParams& ip) {
	itsParams=ip;
}
/// Return current values of params
const MEImageParams& MEImageSolver::getParameters() const {
	return itsParams;
};

void MEImageSolver::addNormalEquations(const MEImageNormalEquations& normeq) {
	itsNormalEquations.merge(normeq);
}

}
}