#include <measurementequation/MESimpleSolver.h>

#include <iostream>

namespace conrad
{
namespace synthesis
{


void MESimpleSolver::init() {
	itsNormalEquations.reset();
	itsDesignMatrix.reset();
}

bool MESimpleSolver::solveNormalEquations(MEQuality& quality) {
	uint rank;
	rank = 0;
	return true;
};

bool MESimpleSolver::solveDesignMatrix(MEQuality& quality) {
	uint rank;
	rank = 0;
	return true;
};

}
}