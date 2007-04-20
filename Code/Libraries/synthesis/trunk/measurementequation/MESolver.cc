#include <measurementequation/MESolver.h>

namespace conrad
{
namespace synthesis
{

MESolver::MESolver(const MEParams& ip) : itsParams(ip), itsNormalEquations(ip),
	itsDesignMatrix(ip)
{
};

void MESolver::setParameters(const MEParams& ip) {
	itsParams=ip;
}
/// Return current values of params
const MEParams& MESolver::parameters() const {
	return itsParams;
};

void MESolver::addNormalEquations(const MENormalEquations& normeq) {
//	itsNormalEquations.merge(normeq);
	itsNormalEquations=normeq;
}

void MESolver::addDesignMatrix(const MEDesignMatrix& designmatrix) 
{
	itsDesignMatrix=designmatrix;
}

}
}