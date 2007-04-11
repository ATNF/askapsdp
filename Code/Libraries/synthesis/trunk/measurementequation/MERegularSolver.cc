#include <measurementequation/MERegularSolver.h>

namespace conrad
{
namespace synthesis
{

MERegularSolver::MERegularSolver(const MERegularParams& ip) : itsParams(ip), itsNormalEquations(ip),
	itsDesignMatrix(ip)
{
};

void MERegularSolver::setParameters(const MERegularParams& ip) {
	itsParams=ip;
}
/// Return current values of params
const MERegularParams& MERegularSolver::getParameters() const {
	return itsParams;
};

void MERegularSolver::addNormalEquations(const MERegularNormalEquations& normeq) {
	itsNormalEquations.merge(normeq);
}

void MERegularSolver::addDesignMatrix(const MERegularDesignMatrix& designmatrix) 
{
	itsDesignMatrix.merge(designmatrix);
}

}
}