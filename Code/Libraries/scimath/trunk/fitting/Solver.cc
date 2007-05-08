#include <fitting/Solver.h>

namespace conrad
{
namespace synthesis
{

Solver::Solver(const Params& ip) : itsParams(ip), itsNormalEquations(ip),
	itsDesignMatrix(ip)
{
};

void Solver::setParameters(const Params& ip) {
	itsParams=ip;
}
/// Return current values of params
const Params& Solver::parameters() const {
	return itsParams;
};

/// Return current values of params
Params& Solver::parameters() {
	return itsParams;
};

void Solver::addNormalEquations(const NormalEquations& normeq) {
	itsNormalEquations.merge(normeq);
}

void Solver::addDesignMatrix(const DesignMatrix& designmatrix) 
{
	itsDesignMatrix.merge(designmatrix);
}

}
}
